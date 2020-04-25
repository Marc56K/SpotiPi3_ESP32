#include "StringUtils.h"
#include <sstream>

namespace StringUtils
{
    bool IsIp(String str)
    {
        for (int i = 0; i < str.length(); i++)
        {
            int c = str.charAt(i);
            if (c != '.' && (c < '0' || c > '9'))
            {
                return false;
            }
        }
        return true;
    }

    String ToStringIp(IPAddress ip)
    {
        String res = "";
        for (int i = 0; i < 3; i++)
        {
            res += String((ip >> (8 * i)) & 0xFF) + ".";
        }
        res += String(((ip >> 8 * 3)) & 0xFF);
        return res;
    }

    String HtmlEncode(const String& html)
    {
        std::ostringstream result;
        for (int i = 0; i < html.length(); i++)
        {
            unsigned char c = html[i];
            if (c == '<')
                result << "&lt;";
            else if (c == '>')
                result << "&gt;";
            else if (c == '&')
                result << "&amp;";
            else if (c == '"')
                result << "&quot;";
            else
                result << c;
        }
        return result.str().c_str();
    }
}