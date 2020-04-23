#include "PowerManager.h"
#include "PinMapping.h"
#include "Log.h"
#include <limits>

namespace PowerManager
{    
    bool IsOnUsb()
    {
        return digitalRead(USB_POWER_PIN);
    }

    bool BatteryIsFull()
    {
        return digitalRead(USB_POWER_PIN) && (analogRead(BAT_FULL_PIN) < 1000);
    }

    bool IsCharging()
    {
        return IsOnUsb() && !BatteryIsFull();
    }

    int GetRawBatteryLevel()
    {
        if (IsCharging())
            return 0;
            
        digitalWrite(BAT_LEVEL_ENABLE_PIN, HIGH);
        delay(1);
        int x = analogRead(BAT_LEVEL_VALUE_PIN);
        digitalWrite(BAT_LEVEL_ENABLE_PIN, LOW);
        return x;
    }

    float voltage = -1;
    float GetBatteryVoltage()
    {
        int x = GetRawBatteryLevel();
        if (x == 0)
        {
            voltage = -1;
            return std::numeric_limits<float>::quiet_NaN();
        }
        
        float v = (4.1f / 2333.0f) * x;
        voltage = voltage < 0 ? v : (voltage * 0.9f + v * 0.1f);
        voltage = max(0.0f, voltage);
        return voltage;
    }

    void Init()
    {
        pinMode(USB_POWER_PIN, INPUT);
        pinMode(BAT_FULL_PIN, INPUT);
        pinMode(BAT_LEVEL_VALUE_PIN, INPUT);
        
        pinMode(BAT_LEVEL_ENABLE_PIN, OUTPUT);
        digitalWrite(BAT_LEVEL_ENABLE_PIN, LOW);
        
        pinMode(POWER_OFF_PIN, OUTPUT);
        digitalWrite(POWER_OFF_PIN, LOW);

        pinMode(PI_POWER_PIN, OUTPUT);
        digitalWrite(PI_POWER_PIN, LOW);
    }

    PowerState GetPowerState()
    {
        PowerState result = {};
        result.isOnUsb = IsOnUsb();
        result.isCharging = IsCharging();
        result.batteryIsFull = BatteryIsFull();
        result.batteryVoltage = GetBatteryVoltage();
        return result;
    }
}