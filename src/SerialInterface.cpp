#include "SerialInterface.h"
#include <Base64.h>
#include <Arduino.h>
#include <Log.h>

const uint8_t STX = 2; // ASCII: Start of Text
const uint8_t ETX = 3; // ASCII: End of Text

SerialInterface::SerialInterface()
{
    Serial2.begin(115200, SERIAL_8E1);
}

void SerialInterface::Write(const std::string& msg)
{
    Write(msg.c_str(), msg.length());
}

void SerialInterface::Write(const void* data, const uint16_t size)
{
    uint32_t headerSize = sizeof(uint32_t);
    uint32_t encodedHeaderLength = Base64.encodedLength(headerSize);
    uint32_t encodedDataLength = Base64.encodedLength(size);
    uint32_t bufferSize = encodedHeaderLength + encodedDataLength;

    if (_buffer.size() < bufferSize)
    {
        _buffer.resize(bufferSize);
    }

    Base64.encode((char*)_buffer.data(), (char*)&encodedDataLength, headerSize);
    Base64.encode((char*)&_buffer[encodedHeaderLength], (char*)data, size);

    Serial2.write(STX);
    Serial2.write(_buffer.data(), bufferSize);
    Serial2.write(ETX);

    Log().Info("SERIAL-OUT") << "Sent message: " << encodedDataLength << " bytes " << std::endl;
}
