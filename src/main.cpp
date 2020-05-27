#include <SPI.h>
#include "Display.h"

SetupManager* setupMgr = nullptr;
SettingsManager settings;
Display display(settings);
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

        display.Clear();
        display.RenderSetupScreen(setupMgr->GetWifiSsid());
        display.Present();

        if (is.buttons[0] > 0 || setupMgr->SetupCompleted())
        {
            delete setupMgr;
            ESP.restart();
        }
    }
    else
    {
        is.potiValue = std::min(is.potiValue, (uint32_t)settings.GetIntValue(Setting::MAX_VOLUME));

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
