#pragma once
#include <string>
#include <vector>

class SerialInterface
{
public:
    SerialInterface();
    void Write(const std::string& msg);
    void Write(const void* data, const uint16_t size);
    int32_t Read(uint8_t*& data);

private:
    std::vector<uint8_t> _buffer;
    bool _readStarted;              
    std::vector<uint8_t> _readEncodedBuffer;
    std::vector<uint8_t> _readDecodedBuffer;
    int32_t _readEncodedDataSize;
};