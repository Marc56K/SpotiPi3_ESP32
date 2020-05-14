#include "PinMapping.h"
#include <SPI.h>
#include <epd1in54_V2.h>
#include <epdpaint.h>
#include "SetupManager.h"
#include "Raspi.h"

SettingsManager settings;
SetupManager* setupMgr = nullptr;
Raspi raspi(settings);

unsigned char image[200*200];
Paint paint(image, 200, 200);
Epd epd;

std::string artist = "";

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
    Serial.begin(115200);
    Log().Info("MAIN") << "setup" << std::endl;

    Log().Info("MAIN") << "e-Paper init" << std::endl;
    if (epd.Init() != 0)
      Log().Error("MAIN") << "e-Paper init failed" << std::endl;

    paint.Clear(0);
    epd.DisplayPart(paint.GetImage(), true);
    paint.Clear(1);
    epd.DisplayPart(paint.GetImage(), false);

    PowerManager::Init();
    InputManager::Init();
    InitSettings();

    Log().Info("MAIN") << "ready" << std::endl;
}

void loop()
{
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
        auto raspiState = raspi.Update(ps, is);

        paint.Clear(1);
        if (raspiState == RaspiState::ShuttingDown)
        {
            paint.DrawStringAt(0, 0, "Shutting down ...", &Font20, 0);
            if (!epd.IsBusy())
                epd.DisplayPart(paint.GetImage(), false);
        }
        else if (raspiState == RaspiState::Shutdown || raspiState == RaspiState::StartTimeout)
        {
            epd.WaitUntilIdle();            
            paint.DrawStringAt(0, 0, raspiState == RaspiState::Shutdown ? "POWER OFF" : "PI TIMEOUT", &Font20, 0);
            epd.DisplayPart(paint.GetImage(), true);
            digitalWrite(POWER_OFF_PIN, HIGH);
        }
        else
        {
            float now = (float)millis() / 1000;
            paint.DrawStringAt(0, 0, (String(now)).c_str(), &Font24, 0);        
            paint.DrawStringAt(0, 20, (String("STATE: ") + String(raspiState) + String(" ") + String(raspi.IsBusy())).c_str(), &Font24, 0);
            paint.DrawStringAt(0, 40, (String("BAT: ") + String(ps.batteryVoltage)).c_str(), &Font24, 0);
            paint.DrawStringAt(0, 60, (String("FULL: ") + String(ps.batteryIsFull)).c_str(), &Font24, 0);
            paint.DrawStringAt(0, 80, (String("USB: ") + String(ps.isOnUsb)).c_str(), &Font24, 0);
            paint.DrawStringAt(0, 100, (String("RFID: ") + is.rfId.c_str()).c_str(), &Font20, 0);
            paint.DrawStringAt(0, 120, (String("POTI: ") + is.potiValue).c_str(), &Font24, 0);
            paint.DrawStringAt(0, 140, artist.c_str(), &Consolas24, 0);

            if (!epd.IsBusy())
                epd.DisplayPart(paint.GetImage(), false);
        }
    }
}
