#pragma once
#include <string>
#include <vector>
#include <ArduinoJson.h>

class SerialInterface
{
public:
    SerialInterface();
    void WriteKeyValue(const std::string& key, const std::string& value);
    void WriteKeyValue(const std::string& key, const int32_t value);
    void Write(const std::string& msg);
    void Write(const void* data, const uint16_t size);
    int32_t Read(uint8_t*& data);
    StaticJsonDocument<1024>& Read();

private:
    std::vector<uint8_t> _buffer;
    bool _readStarted;              
    std::vector<uint8_t> _readEncodedBuffer;
    std::vector<uint8_t> _readDecodedBuffer;
    int32_t _readEncodedDataSize;
    StaticJsonDocument<128> _jsonOutDoc;
    StaticJsonDocument<1024> _jsonInDoc;
};