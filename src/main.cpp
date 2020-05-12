#include "PinMapping.h"
#include <SPI.h>
#include <epd1in54_V2.h>
#include <epdpaint.h>
#include "SetupManager.h"
#include "Log.h"
#include "InputManager.h"
#include "PowerManager.h"
#include "SettingsManager.h"
#include "SerialInterface.h"

SettingsManager settings;
SetupManager* setupMgr = nullptr;
SerialInterface serialIf;

unsigned char image[200*200];
Paint paint(image, 200, 200);
Epd epd;

void InitSettings()
{
    bool setupMode = digitalRead(BT2_PIN) == HIGH;
    bool factoryReset = setupMode && digitalRead(BT3_PIN) == HIGH;
    if (factoryReset)
    {
        settings.ClearEEPROM();
    }
    settings.LoadFromEEPROM();

    if (setupMode)
    {
        setupMgr = new SetupManager(settings);
    }
}

void setup()
{
    //Serial2.begin(115200, SERIAL_8E1);

    Serial.begin(115200);
    Log().Info("MAIN") << "setup" << std::endl;

    Log().Info("MAIN") << "e-Paper init" << std::endl;
    if (epd.Init() != 0)
      Log().Error("MAIN") << "e-Paper init failed" << std::endl;

    paint.Clear(1);
    epd.Display(paint.GetImage());

    PowerManager::Init();
    InputManager::Init();
    InitSettings();

    Log().Info("MAIN") << "ready" << std::endl;
}

void loop()
{
    //Log().Info("MAIN") << "update" << std::endl;

    auto is = InputManager::GetInputState();
    auto ps = PowerManager::GetPowerState();

    if (setupMgr != nullptr)
    {
        setupMgr->Update();

        paint.Clear(1);
        paint.DrawStringAt(0, 0, "  Setup Mode", &Font20, 0);
        paint.DrawStringAt(0, 40,  "WiFi-SSID: ", &Font16, 0);
        paint.DrawStringAt(0, 60,  setupMgr->GetWifiSsid(), &Font20, 0);
        paint.DrawStringAt(0, 100, "WiFi-KEY:", &Font16, 0);
        paint.DrawStringAt(0, 120, settings.GetStringValue(Setting::SETUP_KEY).c_str(), &Font24, 0);
        if (!epd.IsBusy())
            epd.DisplayPart(paint.GetImage(), false);

        if (is.buttons[0] > 0 || setupMgr->SetupCompleted())
        {
            delete setupMgr;
            ESP.restart();
        }
    }
    else
    {
        auto json = serialIf.Read();

        if (json.size() > 0)
        {
            serialIf.WriteKeyValue("wifiSsid", settings.GetStringValue(Setting::WIFI_SSID));
            serialIf.WriteKeyValue("wifiKey", settings.GetStringValue(Setting::WIFI_KEY));
            serialIf.WriteKeyValue("spotifyUser", settings.GetStringValue(Setting::SPOTIFY_USER));
            serialIf.WriteKeyValue("spotifyPassword", settings.GetStringValue(Setting::SPOTIFY_PASSWORD));
            serialIf.WriteKeyValue("spotifyClientId", settings.GetStringValue(Setting::SPOTIFY_CLIENT_ID));
            serialIf.WriteKeyValue("spotifyClientSecret", settings.GetStringValue(Setting::SPOTIFY_CLIENT_SECRET));
            serialIf.WriteKeyValue("playlist",  is.rfId);
        }

        if (json.size() > 0)
        {
            bool online = json["online"].as<bool>();
            Log().Info("PI-STATE") << "Online: " << online << std::endl;
            int tracks = json["tracks"].as<int>();
            Log().Info("PI-STATE") << "Tracks: " << tracks << std::endl;         
        }

        if (json.size() > 0 || abs(is.potiDelta) > 1)
        {
            serialIf.WriteKeyValue("volume",  is.potiValue);
        }

        if (is.buttons[1] > 0)
        {
            if (is.buttons[1] < 3)
                serialIf.WriteKeyValue("skipPrevious", is.buttons[1]);
            else
                serialIf.WriteKeyValue("skipToStart", 1);
        }

        if (is.buttons[3] > 0)
        {
            if (is.buttons[3] < 3)
                serialIf.WriteKeyValue("skipNext", is.buttons[3]);
            else
                serialIf.WriteKeyValue("skipNext", 10);
        }

        if (is.buttons[2] % 2 == 1)
        {
            serialIf.WriteKeyValue("togglePlayPause", 1);
        }

        if (is.buttons[0] > 0)
        {
            serialIf.WriteKeyValue("shutdown", 1);
        }

        float now = (float)millis() / 1000;

        paint.Clear(1);
        paint.DrawStringAt(0, 0, (String(now)).c_str(), &Font24, 0);        
        paint.DrawStringAt(0, 20, (String("BTN: ") + String(is.buttons[0]) + String(is.buttons[1]) + String(is.buttons[2]) + String(is.buttons[3])).c_str(), &Font24, 0);
        paint.DrawStringAt(0, 40, (String("BAT: ") + String(ps.batteryVoltage)).c_str(), &Font24, 0);
        paint.DrawStringAt(0, 60, (String("FULL: ") + String(ps.batteryIsFull)).c_str(), &Font24, 0);
        paint.DrawStringAt(0, 80, (String("USB: ") + String(ps.isOnUsb)).c_str(), &Font24, 0);
        paint.DrawStringAt(0, 100, (String("RFID: ") + is.rfId.c_str()).c_str(), &Font20, 0);
        paint.DrawStringAt(0, 120, (String("POTI: ") + is.potiValue).c_str(), &Font24, 0);

        //if (is.buttons[0] + is.buttons[1] + is.buttons[2] + is.buttons[3] > 0)
            //epd.WaitUntilIdle();

        if (!epd.IsBusy())
            epd.DisplayPart(paint.GetImage(), false);
/*
        if (is.buttons[0] > 0)
        {
            digitalWrite(POWER_OFF_PIN, HIGH);
        }

        if (is.buttons[1] > 0)
        {
            digitalWrite(PI_POWER_PIN, HIGH);
        }

        if (is.buttons[2] > 0)
        {
            digitalWrite(PI_POWER_PIN, LOW);
        }

        if (is.buttons[3] > 0)
        {
            ESP.restart();
        }
        */
    }
}
