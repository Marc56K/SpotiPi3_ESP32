#include "SettingsManager.h"
#include "PinMapping.h"
#include "Log.h"
#include <EEPROM.h>

#define EEPROM_SIZE 512

SettingsManager::SettingsManager()
{
}

SettingsManager::~SettingsManager()
{    
}

void SettingsManager::ClearEEPROM()
{
    Log().Info("Settings") << "ClearEEPROM" << std::endl;
    EEPROM.begin(EEPROM_SIZE);
    uint8_t* ptr = EEPROM.getDataPtr();
    memset(ptr, 0, EEPROM_SIZE);
    EEPROM.commit();
    EEPROM.end();
    Log().Info("Settings") << "ClearEEPROM done" << std::endl;
}

void SettingsManager::LoadFromEEPROM()
{
    Log().Info("Settings") << "LoadFromEEPROM" << std::endl;
    _settings.clear();
    EEPROM.begin(EEPROM_SIZE);
    uint8_t* ptr = EEPROM.getDataPtr();

    uint32_t crc = *((uint32_t*)ptr);
    ptr += sizeof(uint32_t);
    uint32_t actualCrc = ComputeCrc(ptr, EEPROM_SIZE - sizeof(uint32_t));
    Log().Info("Settings") << "CRC " << crc << " vs " << actualCrc << std::endl;
    if (crc == actualCrc)
    {
        uint8_t numEntries = *ptr;
        ptr += sizeof(uint8_t);
        Log().Info("Settings") << "numEntries " << (int)numEntries << std::endl;
        for (uint8_t i = 0; i < numEntries; i++)
        {
            Setting key = *((Setting*)ptr);
            ptr += sizeof(Setting);

            uint16_t size = *((uint16_t*)ptr);
            ptr += sizeof(uint16_t);

            std::vector<uint8_t> value(size);
            memcpy(value.data(), ptr, size);
            ptr += size;

            _settings[key] = value;
        }
    }
    EEPROM.end();
    Log().Info("Settings") << "LoadFromEEPROM done " << std::endl;
    if (!HasValue(Setting::SETUP_KEY))
    {
        Log().Info("Settings") << "creating SETUP_KEY" << std::endl;
        char setupKey[] = "aaaaaaaaaaaa";
        srand(analogRead(POTI_PIN));
        for (int i = 0; i < strlen(setupKey); i++)
        {
            setupKey[i] += (rand() ^ analogRead(POTI_PIN)) % 26;
        }
        SetValue(Setting::SETUP_KEY, setupKey);
        Log().Info("Settings") << "SETUP_KEY: " << setupKey << std::endl;
        SaveToEEPROM();
    }
}

void SettingsManager::SaveToEEPROM()
{
    Log().Info("Settings") << "SaveToEEPROM" << std::endl;
    EEPROM.begin(EEPROM_SIZE);
    uint8_t numEntries = (uint8_t)_settings.size();
    uint8_t* ptr = EEPROM.getDataPtr();

    uint32_t* crcPtr = (uint32_t*)ptr;
    ptr += sizeof(uint32_t);

    *ptr = numEntries;
    ptr += sizeof(uint8_t);
    for (auto& it : _settings)
    {
        *((Setting*)ptr) = it.first;
        ptr += sizeof(Setting);

        *((uint16_t*)ptr) = (uint16_t)it.second.size();
        ptr += sizeof(uint16_t);

        memcpy(ptr, it.second.data(), it.second.size());
        ptr += it.second.size();
    }

    *crcPtr = ComputeCrc(EEPROM.getDataPtr() + sizeof(uint32_t), EEPROM_SIZE - sizeof(uint32_t));
    Log().Info("Settings") << "CRC " << (*crcPtr) << std::endl;
    Log().Info("Settings") << "commit" << std::endl;
    EEPROM.commit();
    EEPROM.end();
    Log().Info("Settings") << "SaveToEEPROM done" << std::endl;
}

bool SettingsManager::HasValue(Setting key)
{
    return _settings.find(key) != _settings.end();
}

float SettingsManager::GetFloatValue(Setting key)
{
    auto it = _settings.find(key);
    if (it != _settings.end())
    {
        if (sizeof(float) == it->second.size())
        {
            return *((float*)it->second.data());
        }
    }
    return 0;
}

std::string SettingsManager::GetStringValue(Setting key)
{
    auto it = _settings.find(key);
    if (it != _settings.end())
    {
        return std::string((const char*)it->second.data());
    }
    return "";
}

void SettingsManager::SetValue(Setting key, float value)
{
    std::vector<uint8_t> val(sizeof(float));
    memcpy(val.data(), &value, sizeof(float));
    _settings[key] = val;
}

void SettingsManager::SetValue(Setting key, const std::string& value)
{
    std::vector<uint8_t> val(value.length() + 1);
    memcpy(val.data(), value.c_str(), value.length());
    _settings[key] = val;
}

uint32_t SettingsManager::ComputeCrc(void* ptr, uint32_t size)
{
    uint32_t result = 0;
    for (uint32_t i = 0; i < size / sizeof(uint32_t); i++)
    {
        result += ((uint32_t*)ptr)[i];
    }
    return result;
}