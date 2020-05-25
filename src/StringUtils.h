#pragma once
#include <Arduino.h>
#include <string>
#include <functional>

namespace StringUtils
{
    bool IsIp(const std::string& str);

    std::string HtmlEncode(const std::string& html);
    std::string HtmlTextInput(const std::string& id, const std::string& label, const std::string& value, const bool password);
    std::string HtmlSelectBox(const std::string& id, const std::string& label, const std::string& value, const uint32_t numOptions, std::function<std::string(uint32_t)> options);
    std::string Utf8ToLatin1String(const std::string& utf8String);
    std::string Trim(const std::string& s);
    std::string Replace(const std::string& s, const std::string& replace,  const std::string& with);
    std::string SecondsToTime(const float seconds);
    std::vector<std::string> Split(const std::string& s, char delimiter);
    std::vector<std::string> LineWrap(const uint32_t maxCharsPerLine, const uint32_t maxLines, const std::string& txt);
}