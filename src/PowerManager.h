#pragma once
#include <Arduino.h>

namespace PowerManager
{
    struct PowerState
    {
        bool isOnUsb;
        bool isCharging;
        bool batteryIsFull;
        float batteryVoltage;
    };

    void Init();
    PowerState GetPowerState();
};