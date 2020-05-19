#include "PinMapping.h"
#include <SPI.h>
#include "SetupManager.h"
#include "Raspi.h"
#include "Display.h"

Display display;
SetupManager* setupMgr = nullptr;
SettingsManager settings;
Raspi raspi(settings);

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

    display.Init();
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

        display.Clear();
        display.RenderSetupScreen(setupMgr->GetWifiSsid(), settings.GetStringValue(Setting::SETUP_KEY));
        display.Present();

        if (is.buttons[0] > 0 || setupMgr->SetupCompleted())
        {
            delete setupMgr;
            ESP.restart();
        }
    }
    else
    {
        SpotiPiInfo& info = raspi.Info();
        auto raspiState = raspi.Update(ps, is);

        if (raspiState == RaspiState::ShuttingDown)
        {
            display.Clear();
            display.RenderBusyAnimation(72, 72);
            display.Present(false);
        }
        else if (raspiState == RaspiState::Shutdown || raspiState == RaspiState::StartTimeout)
        {
            display.Clear();
            display.RenderPowerOffScreen();
            display.RenderStandbyIcon(5, 40);
            display.Present(true);

            digitalWrite(POWER_OFF_PIN, HIGH);
        }
        else
        {
            display.Clear();

            if (info.playlistName != "")
                display.RenderCenterText(100, 2, 200, 1, info.playlist);
            else if (is.rfId != "")
                display.RenderCenterText(100, 2, 200, 1, "[" + is.rfId + "]");
            else
                display.RenderCenterText(100, 2, 200, 1, "SpotiPi");
            

            display.RenderRectangle(0, 30, 199, 31);

            display.RenderStandbyIcon(5, 40);

            if (info.tracks > 0)
            {
                if (raspiState == RaspiState::Playing)
                    display.RenderPauseIcon(174, 40);
                if (raspiState == RaspiState::Idle)
                    display.RenderPlayIcon(174, 40);
                
                if (info.track > 0)
                    display.RenderPrevTrackIcon(5, 140);

                if (info.track < info.tracks - 1)
                    display.RenderNextTrackIcon(174, 140);
            }

            if (raspi.IsBusy())
            {
                display.RenderBusyAnimation(72, 72);
            }
            else if (info.tracks > 0)
            {
                {
                    display.RenderCenterText(100, 38, 140, 1, StringUtils::SecondsToTime(info.time));
                }

                {
                    std::stringstream ss;
                    ss << info.title;
                    display.RenderCenterText(100, 72, 190, 2, ss.str());
                }

                {
                    std::stringstream ss;
                    ss << (info.track + 1) << "/" << info.tracks;
                    display.RenderCenterText(100, 138, 140, 1, ss.str());
                }
            }            

            display.RenderRectangle(0, 170, 199, 171);
            display.RenderVolumeIndicator(5, 176, is.potiValue);
            display.RenderOnlineIndicator(122, 176, info.online);
            display.RenderBatteryIndicator(156, 176, ps.isCharging, ps.isOnUsb, ps.batteryLevel);
            display.Present();
        }
    }
}
