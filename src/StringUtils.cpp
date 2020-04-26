#include "StringUtils.h"
#include <sstream>

namespace StringUtils
{
    bool IsIp(const std::string& str)
    {
        for (int i = 0; i < str.length(); i++)
        {
            int c = str[i];
            if (c != '.' && (c < '0' || c > '9'))
            {
                return false;
            }
        }
        return true;
    }

    std::string HtmlEncode(const std::string& html)
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
        return result.str();
    }

    std::string HtmlTextInput(const std::string& id, const std::string& label, const std::string& value, const bool password)
    {
        std::ostringstream result;
        result << "<div class=\"form-group\">";
        result << "<label for=\"" << id << "\">" << HtmlEncode(label) << "</label>";
        result << "<input class=\"form-control\" type=\"" << (password ? "password" : "text") << "\" id=\"" << id << "\" name=\"" << id << "\" ";
        result << "value=\"" << HtmlEncode(value) << "\">";
        result << "</div>";
        return result.str();
    }

    std::string HtmlSelectBox(const std::string& id, const std::string& label, const std::string& value, const uint32_t numOptions, std::function<std::string(uint32_t)> options)
    {
        std::ostringstream result;
        result << "<div class=\"form-group\">";
        result << "<label for=\"" << id << "\">" << HtmlEncode(label) << "</label>";
        result << "<select class=\"form-control\" type=\"text\" id=\"" << id << "\" name=\"" << id << "\">";
        result << "<option></option>";
        bool foundValue = false;
        for (uint32_t i = 0; i < numOptions; i++)
        {
            std::string option = options(i);
            if (option == "")
            {
                continue;
            }
            if (option == value)
            {
                foundValue = true;
                result << "<option selected>";
            }                
            else
            {
                result << "<option>";
            }
            result << HtmlEncode(option);
            result << "</option>";
        }
        if (value != "" && !foundValue)
        {
            result << "<option selected>";
            result << HtmlEncode(value);
            result << "</option>";
        }        
        result << "</select>";
        result << "</div>";
        return result.str();
    }
}