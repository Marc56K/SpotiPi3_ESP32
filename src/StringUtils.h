#pragma once
#include <Arduino.h>

namespace StringUtils
{
    bool IsIp(String str);
    String ToStringIp(IPAddress ip);

    String HtmlEncode(const String& html);
}