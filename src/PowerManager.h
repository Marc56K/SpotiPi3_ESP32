#pragma once
#include <Arduino.h>

struct PowerState
{
    bool isOnUsb;
    bool isCharging;
    bool batteryIsFull;
    float batteryVoltage;
};

namespace PowerManager
{
    void Init();
    PowerState GetPowerState();
};