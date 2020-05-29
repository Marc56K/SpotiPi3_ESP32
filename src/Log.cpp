#include "Log.h"
#include <esp32-hal.h>
#include <HardwareSerial.h>

//#define TIMESTAMPS

Log::~Log()
{
    if (Serial)
    {
        Serial.print(os.str().c_str());
    }
}

std::ostringstream& Log::Info(const std::string& sender)
{
    SetTimestamp();
    os << "[INFO] " << sender << ": ";
    return os;
}

std::ostringstream& Log::Warning(const std::string& sender)
{
    SetTimestamp();
    os << "[WARNING] " << sender << ": ";
    return os;
}

std::ostringstream& Log::Error(const std::string& sender)
{
    SetTimestamp();
    os << "[ERROR] " << sender << ": ";
    return os;
}

void Log::SetTimestamp()
{
    #ifdef TIMESTAMPS

    unsigned long t = millis();
    unsigned long ms = t % 1000;
    unsigned long s = (t / 1000) % 60;
    unsigned long m = (t / 1000 / 60) % 60;
    unsigned long h = (t / 1000 / 60 / 60) % 60;

    os << "[";
    if (h < 10) os << "0"; 
    os << h;
    os << ":";
    if (m < 10) os << "0"; 
    os << m;
    os << ":";
    if (s < 10) os << "0";
    os << s;
    os << ":";
    if (ms < 10) os << "0";
    if (ms < 100) os << "0";
    os << ms;    
    os << "] ";

    #endif
}