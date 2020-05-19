#pragma once
#include <Arduino.h>

struct PowerState
{
    bool isOnUsb;
    bool isCharging;
    bool batteryIsFull;
    float batteryVoltage;
    int batteryLevel;
    bool sufficientPower;
};

namespace PowerManager
{
    void Init();
    PowerState GetPowerState();
};