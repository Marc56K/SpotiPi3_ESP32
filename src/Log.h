#pragma once
#include <string>
#include <iostream>
#include <sstream>

class Log
{
public:
    Log(const Log&) = delete;
    Log(Log&&) = delete;
    Log() {};
    virtual ~Log();
    std::ostringstream& Info(const std::string& sender);
    std::ostringstream& Warning(const std::string& sender);
    std::ostringstream& Error(const std::string& sender);

protected:
    void SetTimestamp();

    std::ostringstream os;
};