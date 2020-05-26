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
        int x = 0;
        for (int i = 0; i < 2; i++)
        {
            x = std::max(x, (int)analogRead(BAT_LEVEL_VALUE_PIN));
            delay(1);
        }
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
        
        float v = std::max(3.0f, (4.1f / 2333.0f) * x);
        voltage = voltage < 0 ? v : (voltage * 0.9f + v * 0.1f);
        voltage = max(0.0f, voltage);
        return voltage;
    }

    int GetBatteryLevel(float v)
    {
        int result = -240.0673368f * v * v * v + 2658.611075f * v * v - 9650.778487f * v + 11521.76802f;
        return max(0, min(result, 100));
    }

    unsigned long lastSufficientBatteryVoltage = 0;
    bool BatteryVoltageIsSufficient(float v)
    {
        auto now = millis();
        if (v > 3.3f)
        {
            lastSufficientBatteryVoltage = now;
        }

        unsigned long delta = 0;        
        if (now >= lastSufficientBatteryVoltage)
        {
            delta = now - lastSufficientBatteryVoltage;
        }
        else
        {
            // handle time overflow
            unsigned long t = -1;
            t -= lastSufficientBatteryVoltage;
            delta = t + now;
        }

        return delta < 60000;
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
        result.batteryLevel = GetBatteryLevel(result.batteryVoltage);
        result.sufficientPower = result.isOnUsb || BatteryVoltageIsSufficient(result.batteryVoltage);
        return result;
    }

    void PowerOff()
    {
        digitalWrite(POWER_OFF_PIN, HIGH);
    }
}