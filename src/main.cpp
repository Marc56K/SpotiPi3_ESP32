#include <SPI.h>
#include "Display.h"

SetupManager* setupMgr = nullptr;
SettingsManager settings;
Display display(settings);
Raspi raspi(settings);

void InitSettings()
{
    settings.LoadFromEEPROM();

    if (digitalRead(BT2_PIN) == HIGH)
    {
        setupMgr = new SetupManager(settings);
    }
}

void setup()
{
    Serial.begin(115200);
    display.Init();
    PowerManager::Init();
    InputManager::Init();
    InitSettings();

    auto ps = PowerManager::GetPowerState();
    if (!ps.sufficientPower)
    {
        display.Clear();
        display.RenderLowBatteryScreen();
        display.Present(true);
        PowerManager::PowerOff();
    }
}

void loop()
{
    auto is = InputManager::GetInputState();
    auto ps = PowerManager::GetPowerState();

    if (setupMgr != nullptr)
    {
        setupMgr->Update();

        if (millis() < 30 * 60 * 1000) // 30 min
        {
            display.Clear();
            display.RenderSetupScreen(setupMgr->GetWifiSsid());
            display.Present();

            if (is.buttons[0] > 0 || setupMgr->SetupCompleted())
            {
                delete setupMgr;
                ESP.restart();
            }
        }
        else // timeout
        {
            delete setupMgr;
            display.Clear();
            display.RenderPowerOffScreen();
            display.Present(true);
            PowerManager::PowerOff();
        }
    }
    else
    {
        is.potiValue = (uint32_t)roundf((float)is.potiValue * settings.GetFloatValue(Setting::MAX_VOLUME) / 100.0f);

        RaspiInfo& pi = raspi.Update(ps, is);

        if (pi.state == RaspiState::ShuttingDown)
        {
            display.Clear();
            switch (pi.shutdownReason)
            {
                case ShutdownReason::Timeout:
                    display.RenderTimeoutScreen();
                    break;
                case ShutdownReason::LowPower:
                    display.RenderLowBatteryScreen();
                    break;
                default:
                    display.RenderBusyAnimation(72, 72);
            }
            display.Present(false);
        }
        else if (pi.state == RaspiState::Shutdown)
        {
            display.Clear();
            switch (pi.shutdownReason)
            {
                case ShutdownReason::Timeout:
                    display.RenderTimeoutScreen();
                    break;
                case ShutdownReason::LowPower:
                    display.RenderLowBatteryScreen();
                    break;
                default:
                    display.RenderPowerOffScreen(); 
            }
            display.Present(true);
            delay(2000);
            PowerManager::PowerOff();
        }
        else
        {
            display.Clear();
            display.RenderMediaPlayerScreen(pi, is, ps);
            display.Present(false);
        }
    }
}
