#include "SerialInterface.h"
#include <Base64.h>
#include <Arduino.h>
#include <Log.h>

const uint8_t STX = 2; // ASCII: Start of Text
const uint8_t ETX = 3; // ASCII: End of Text
const int32_t headerSize = sizeof(int32_t);

SerialInterface::SerialInterface()
    : _readStarted(false), _readEncodedDataSize(-1)
{
    Serial2.begin(115200, SERIAL_8E1);
}

void SerialInterface::Write(const std::string& msg)
{
    Write(msg.c_str(), msg.length());
}

void SerialInterface::Write(const void* data, const uint16_t size)
{
    const int32_t encodedHeaderLength = Base64.encodedLength(headerSize);
    const int32_t encodedDataLength = Base64.encodedLength(size);
    const int32_t bufferSize = encodedHeaderLength + encodedDataLength + 1; // Base64.encode needs one additional byte for null termination

    if (_buffer.size() < bufferSize)
    {
        _buffer.resize(bufferSize);
    }

    Base64.encode((char*)_buffer.data(), (char*)&encodedDataLength, headerSize);
    Base64.encode((char*)&_buffer[encodedHeaderLength], (char*)data, size);

    Serial2.write(STX);
    Serial2.write(_buffer.data(), bufferSize);
    Serial2.write(ETX);
}

int32_t SerialInterface::Read(uint8_t*& data)
{
    const int32_t encodedHeaderLength = Base64.encodedLength(headerSize);
    while (Serial2.available()) 
    {
        auto b = (uint8_t)Serial2.read();
        if (b == STX || b == ETX)
        {
            _readStarted = (b == STX);
            _readEncodedBuffer.resize(0);
            _readEncodedDataSize = -1;
        }
        else if (_readStarted)
        {
            if (_readEncodedDataSize < 0)
            {
                _readEncodedBuffer.push_back(b);
                if (_readEncodedBuffer.size() == encodedHeaderLength)
                {
                    char decodeBuffer[sizeof(int32_t) + 1]; // Base64.decode needs one additional byte for null termination
                    Base64.decode(decodeBuffer, (char*)_readEncodedBuffer.data(), encodedHeaderLength);
                    memcpy(&_readEncodedDataSize, decodeBuffer, sizeof(int32_t));
                    _readEncodedBuffer.resize(0);
                    //Log().Info("SERIAL-IN") << "Received " << _readEncodedDataSize << " bytes" << std::endl;
                }
            }
            else
            {
                if (_readEncodedBuffer.size() < _readEncodedDataSize)
                {
                    _readEncodedBuffer.push_back(b);
                }
                if (_readEncodedBuffer.size() == _readEncodedDataSize)
                {
                    int32_t decodeDataLength = Base64.decodedLength((char*)_readEncodedBuffer.data(), _readEncodedDataSize);
                    _readDecodedBuffer.resize(decodeDataLength + 1); // Base64.decode needs one additional byte for null termination
                    Base64.decode((char*)_readDecodedBuffer.data(), (char*)_readEncodedBuffer.data(), _readEncodedDataSize);
                    data =_readDecodedBuffer.data(); 
                    return decodeDataLength;
                }                
            }            
        }
    }
    return 0;
}