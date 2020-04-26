#pragma once
#include <map>
#include <vector>
#include <Arduino.h>

enum Setting : uint8_t
{
    SETUP_KEY = 0,
    WIFI_SSID,
    WIFI_KEY,
};

class SettingsManager
{
public:
    SettingsManager();
    ~SettingsManager();

    void ClearEEPROM();
    void LoadFromEEPROM();
    void SaveToEEPROM();

    bool HasValue(Setting key);
    float GetFloatValue(Setting key);
    const char* GetStringValue(Setting key);
    void SetValue(Setting key, float value);
    void SetValue(Setting key, const char* value);

private:
    uint32_t ComputeCrc(void* ptr, uint32_t size);

private:
    std::map<Setting, std::vector<uint8_t>> _settings;
};
 