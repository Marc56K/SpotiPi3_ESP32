#include <SPI.h>
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
        RaspiInfo& pi = raspi.Update(ps, is);

        if (pi.state == RaspiState::ShuttingDown)
        {
            display.Clear();
            display.RenderBusyAnimation(72, 72);
            display.Present(false);
        }
        else if (pi.state == RaspiState::Shutdown || pi.state == RaspiState::StartTimeout)
        {
            display.Clear();
            if (ps.sufficientPower)
                display.RenderPowerOffScreen();
            else
                display.RenderLowBatteryScreen();
            display.Present(true);
            PowerManager::PowerOff();
        }
        else
        {
            display.Clear();
            display.RenderMediaPlayerScreen(pi, is, ps);
            display.Present();
        }
    }
}
