#pragma once
#include <string>
#include <vector>

class SerialInterface
{
public:
    SerialInterface();
    void Write(const std::string& msg);
    void Write(const void* data, const uint16_t size);

private:
    std::vector<uint8_t> _buffer;
};