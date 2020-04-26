#include "SetupManager.h"
#include <WiFi.h>
#include <WiFiClient.h> 
#include <ESPmDNS.h>
#include <soc/sens_reg.h>
#include "Log.h"
#include "PinMapping.h"
#include "bootstrap.min.h"
#include "StringUtils.h"

#define ESP32_SSID "SpotiPi-Config"
#define ESP32_HOST "SPOTIPI_ESP32"
#define DNS_PORT 53
#define GPIO_OUT_W1TS_REG (DR_REG_GPIO_BASE + 0x0008)
#define GPIO_OUT_W1TC_REG (DR_REG_GPIO_BASE + 0x000c)

IPAddress apIP(172, 20, 0, 1);
IPAddress netMsk(255, 255, 255, 0);

SetupManager::SetupManager(SettingsManager& settings)
    : _httpServer(80), _setupCompleted(false), _settings(settings)

{
    // save some register states that wifi will screw up
    reg_a = READ_PERI_REG(SENS_SAR_START_FORCE_REG);
    reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);
    reg_c = READ_PERI_REG(SENS_SAR_MEAS_START2_REG);

    // init wifi
    WiFi.setAutoReconnect (false);
    WiFi.persistent(false);
    WiFi.disconnect(); 
    WiFi.setHostname(ESP32_HOST); // Set the DHCP hostname assigned to ESP station.
    WiFi.disconnect();
    std::string key = _settings.GetStringValue(Setting::SETUP_KEY);
    Log().Info("WIFI-AP") << "Initalize Wifi-AP: " << ESP32_SSID << " Key: " << key << std::endl;
    if (!WiFi.softAP(ESP32_SSID, key.c_str()))
    {
        Log().Error("WIFI-AP") << "Failed to start WiFi-AP. " << std::endl;
    }
    else
    {
        delay(2000);
        WiFi.softAPConfig(apIP, apIP, netMsk);
        _dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        _dnsServer.start(DNS_PORT, "*", apIP);
    }

    // init http server
    _httpServer.on("/", [this]() { HandleRootRequest(); });
    _httpServer.on("/save", [this]() { HandleSaveRequest(); });
    _httpServer.on("/bootstrap.min.css", [this]() { HandleCssRequest(); });
    _httpServer.on("/generate_204", [this]() { HandleRootRequest(); }); //Android captive portal. Maybe not needed. Might be handled by notFound handler.
    _httpServer.on("/favicon.ico", [this]() { HandleRootRequest(); });   //Another Android captive portal. Maybe not needed. Might be handled by notFound handler. Checked on Sony Handy
    _httpServer.on("/fwlink", [this]() { HandleRootRequest(); });  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    _httpServer.onNotFound ( [this]() { HandleNotFound(); } );

    _httpServer.begin(); // Web server start
}

SetupManager::~SetupManager()
{
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);

    // fix ADC2 registers to make analog pin reads work again
    WRITE_PERI_REG(SENS_SAR_START_FORCE_REG, reg_a);  
    WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
    WRITE_PERI_REG(SENS_SAR_MEAS_START2_REG, reg_c);
}

const char* SetupManager::GetWifiSsid() const
{
    return ESP32_SSID;
}

bool SetupManager::SetupCompleted() const
{
    return _setupCompleted;
}

void SetupManager::Update()
{
    _dnsServer.processNextRequest();
    _httpServer.handleClient();
}

void SetupManager::HandleRootRequest()
{   
    Log().Info("SETUP") << "HandleRootRequest" << std::endl;
    WiFi.scanDelete();
    int numWifis = WiFi.scanNetworks(false, false);

    // HTML Header
    _httpServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    _httpServer.sendHeader("Pragma", "no-cache");
    _httpServer.sendHeader("Expires", "-1");
    _httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
    // HTML Content
    _httpServer.send (200, "text/html", "");
    _httpServer.sendContent("<!DOCTYPE HTML><html lang=\"de\"><head><meta charset=\"UTF-8\"><meta name= viewport content=\"width=device-width, initial-scale=1.0,\">");
    _httpServer.sendContent("<head><title>SpotiPi</title>");
    //_httpServer.sendContent("<link rel=\"stylesheet\" href=\"bootstrap.min.css\">");
    _httpServer.sendContent("<style>");
    _httpServer.sendContent_P(bootstrap_css);
    _httpServer.sendContent("</style>");
    _httpServer.sendContent("</head><body style=\"padding: 5px;\">");
    _httpServer.sendContent("<form action=\"/save\" method=\"post\">");

    _httpServer.sendContent(StringUtils::HtmlSelectBox(SettingName[Setting::WIFI_SSID], "WiFi-SSID", _settings.GetStringValue(Setting::WIFI_SSID), numWifis, [] (uint32_t i) { return WiFi.SSID(i).c_str(); } ).c_str());
    _httpServer.sendContent(StringUtils::HtmlTextInput(SettingName[Setting::WIFI_KEY], "WiFi-Password", _settings.GetStringValue(Setting::WIFI_KEY), true).c_str());

    _httpServer.sendContent("<button type=\"submit\" class=\"btn btn-primary\">Save</button>");
    _httpServer.sendContent("</form>");
    _httpServer.sendContent("</body></html>");
    _httpServer.client().stop(); // Stop is needed because we sent no content length

    Log().Info("SETUP") << "HandleRootRequest done" << std::endl;
}

void SetupManager::HandleSaveRequest()
{
    Log().Info("SETUP") << "HandleSaveRequest " <<_httpServer.args() << std::endl;
    for (uint32_t i = 0; i < Setting::NUM_SETTINGS; i++)
    {
        auto key = SettingName[i];
        if (_httpServer.hasArg(key))
        {
            auto value = _httpServer.arg(key);
            _settings.SetValue((Setting)i, std::string(value.c_str()));
        }
    }
    _settings.SaveToEEPROM();

    _httpServer.send (200, "text/html", "configuration completed" );
    _httpServer.client().stop();

    _setupCompleted = true;

    Log().Info("SETUP") << "HandleSaveRequest done" << std::endl;
}

void SetupManager::HandleCssRequest()
{
    size_t size = strlen(bootstrap_css);
    _httpServer.setContentLength(size);
    _httpServer.send (200, "text/css", "");
    _httpServer.sendContent_P(bootstrap_css);
}

void SetupManager::HandleNotFound()
{
    Log().Info("SETUP") << "HandleNotFound " << _httpServer.hostHeader().c_str() << std::endl;
    if (!StringUtils::IsIp(_httpServer.hostHeader().c_str()) && _httpServer.hostHeader() != (String(ESP32_HOST)+".local"))
    {
        // Serial.println("Request redirected to captive portal");  
        
        String url = String("http://") + _httpServer.client().localIP().toString();
        Log().Info("SETUP") << "Request redirected to captive portal: " << url.c_str() << std::endl;
        _httpServer.sendHeader("Location", url, true);        
        _httpServer.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        _httpServer.client().stop(); // Stop is needed because we sent no content length    
    }
}