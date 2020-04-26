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
    const char* key = _settings.GetStringValue(Setting::SETUP_KEY);
    Log().Info("WIFI-AP") << "Initalize Wifi-AP: " << ESP32_SSID << " Key: " << key << std::endl;
    if (!WiFi.softAP(ESP32_SSID, key))
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
    WiFi.scanDelete();
    int n = WiFi.scanNetworks(false, false);

    // HTML Header
    _httpServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    _httpServer.sendHeader("Pragma", "no-cache");
    _httpServer.sendHeader("Expires", "-1");
    _httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
    // HTML Content
    _httpServer.send (200, "text/html", "");
    _httpServer.sendContent("<!DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'><meta name= viewport content='width=device-width, initial-scale=1.0,'>");
    _httpServer.sendContent("<head><title>SpotiPi</title>");
    _httpServer.sendContent("<link rel='stylesheet' href='bootstrap.min.css'>");
    _httpServer.sendContent("</head><body style='padding: 5px;'>");
    _httpServer.sendContent("<form action='/save' method='post'>");

    _httpServer.sendContent("<div class='form-group'><label for='wifi_ssid'>WiFi-SSID</label><select type='text' class='form-control' id='wifi_ssid' name='wifi_ssid'>");
    _httpServer.sendContent("<option = \"\"></option>");
    for (int i = 0; i < n; i++)
    {
        String ssid = StringUtils::HtmlEncode(WiFi.SSID(i));
        _httpServer.sendContent("<option value=\"" + ssid + "\">");
        _httpServer.sendContent(ssid);
        _httpServer.sendContent("</option>");
    }
    _httpServer.sendContent("</select></div>");

    _httpServer.sendContent("<div class='form-group'><label for='wifi_password'>WiFi-Password</label><input type='password' class='form-control' id='wifi_password' name='wifi_password'></div>");
    _httpServer.sendContent("<button type='submit' class='btn btn-primary'>Save</button>");
    _httpServer.sendContent("</form>");
    _httpServer.sendContent("</body></html>");
    _httpServer.client().stop(); // Stop is needed because we sent no content length
}

void SetupManager::HandleSaveRequest()
{
    Log().Info("SAVE") << _httpServer.args() <<  std::endl;

    if (_httpServer.hasArg("wifi_ssid"))
    {
        Log().Info("wifi_ssid") << _httpServer.arg("wifi_ssid").c_str() << std::endl;
    }
    if (_httpServer.hasArg("wifi_password"))
    {
        Log().Info("wifi_password") << _httpServer.arg("wifi_password").c_str() << std::endl;
    }

    _httpServer.send (200, "text/html", "" );
    _httpServer.client().stop();

    _settings.SaveToEEPROM();

    _setupCompleted = true;
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
    if (!StringUtils::IsIp(_httpServer.hostHeader()) && _httpServer.hostHeader() != (String(ESP32_HOST)+".local"))
    {
        // Serial.println("Request redirected to captive portal");  
        _httpServer.sendHeader("Location", String("http://") + StringUtils::ToStringIp(_httpServer.client().localIP()), true);
        _httpServer.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        _httpServer.client().stop(); // Stop is needed because we sent no content length    
    }
}