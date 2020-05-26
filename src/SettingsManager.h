#pragma once
#include <map>
#include <vector>
#include <string>

enum Setting : uint8_t
{
    SETUP_KEY = 0,
    WIFI_SSID,
    WIFI_KEY,
    SPOTIFY_USER,
    SPOTIFY_PASSWORD,
    SPOTIFY_CLIENT_ID,
    SPOTIFY_CLIENT_SECRET,
    DISPLAY_NAME,
    SHUTDOWN_DELAY,
    MAX_VOLUME,
    NUM_SETTINGS
};

const char SettingName[][32] =
{
    "SETUP_KEY",
    "WIFI_SSID",
    "WIFI_KEY",
    "SPOTIFY_USER",
    "SPOTIFY_PASSWORD",
    "SPOTIFY_CLIENT_ID",
    "SPOTIFY_CLIENT_SECRET",
    "DISPLAY_NAME",
    "SHUTDOWN_DELAY",
    "MAX_VOLUME"
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
    int GetIntValue(Setting key);
    std::string GetStringValue(Setting key);
    void SetValue(Setting key, float value);
    void SetValue(Setting key, const std::string& value);

private:
    void InitDefaultValues();
    uint32_t ComputeCrc(void* ptr, uint32_t size);

private:
    std::map<Setting, std::vector<uint8_t>> _settings;
};
 