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

    std::string HtmlNumberInput(const std::string& id, const std::string& label, const int value, const int minValue, const int maxValue)
    {
        std::ostringstream result;
        result << "<div class=\"form-group\">";
        result << "<label for=\"" << id << "\">" << HtmlEncode(label) << "</label>";
        result << "<input class=\"form-control\" type=\"number\" id=\"" << id << "\" name=\"" << id << "\" ";
        result << "value=\"" << value << "\" ";
        result << "min=\"" << minValue << "\" ";
        result << "max=\"" << maxValue << "\">";
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

    int GetUtf8CharacterLength(const unsigned char utf8Char)
    {
        if ( utf8Char < 0x80 ) return 1;
        else if ( ( utf8Char & 0x20 ) == 0 ) return 2;
        else if ( ( utf8Char & 0x10 ) == 0 ) return 3;
        else if ( ( utf8Char & 0x08 ) == 0 ) return 4;
        else if ( ( utf8Char & 0x04 ) == 0 ) return 5;

        return 6;
    }

    char Utf8ToLatin1Character(const char *s, int& readIndex)
    {
        int len = GetUtf8CharacterLength(static_cast<unsigned char>( s[readIndex]));
        if (len == 1)
        {
            char c = s[readIndex];
            readIndex++;
            return c;
        }

        unsigned int v = (s[readIndex] & (0xff >> (len + 1))) << ((len - 1) * 6);
        readIndex++;
        for (len--; len > 0; len--)
        {
            v |= (static_cast<unsigned char>(s[readIndex]) - 0x80) << (( len - 1 ) * 6);
            readIndex++;
        }

        return ( v > 0xff ) ? 0 : (char)v;
    }

    std::string Utf8ToLatin1String(const std::string& utf8String)
    {
        std::ostringstream result;
        const char* s = utf8String.c_str();
        int readIndex = 0;
        while(s[readIndex] != 0)
        {
            char c = Utf8ToLatin1Character(s, readIndex);
            if ( c == 0 )
            {
                c = '?';
            }

            result << c;
        }

        return result.str();
    }

    std::string Trim(const std::string &s)
    {
        auto start = s.begin();
        while (start != s.end() && std::isspace(*start))
        {
            start++;
        }

        auto end = s.end();
        do 
        {
            end--;
        } while (std::distance(start, end) > 0 && std::isspace(*end));

        return std::string(start, end + 1);
    }

    std::string Replace(const std::string& s, const std::string& replace,  const std::string& with)
    {
        std::string str = s;
        std::size_t pos = str.find(replace);
        if (pos != std::string::npos)
        {
            str.replace(pos, replace.length(), with);
        }
        return str;
    }

    std::string SecondsToTime(const float seconds)
    {
        int minutes = seconds / 60;
        int secs = seconds - minutes * 60;
        std::stringstream ss;
        if (minutes < 10)
            ss << "0";
        ss << minutes;
        ss << ":";
        if (secs < 10)
            ss << "0";
        ss << secs;
        return ss.str();
    }

    std::vector<std::string> Split(const std::string& s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    std::vector<std::string> LineWrap(const uint32_t maxCharsPerLine, const uint32_t maxLines, const std::string& txt)
    {
        std::vector<std::string> result;

        std::vector<std::string> words = Split(txt, ' ');
        std::stringstream currentLine;
        uint32_t currentLineLength = 0;
        for (auto wordIt = words.begin(); wordIt != words.end(); wordIt++)
        {
            std::string& word = *wordIt;

            if (currentLineLength > 0 && currentLineLength + word.length() + 1 > maxCharsPerLine)
            {
                result.push_back(currentLine.str());
                currentLine.str("");
                currentLine.clear();
                currentLineLength = 0;
                if (result.size() > maxLines) break;
            }

            if (currentLineLength == 0 && word.length() >= maxCharsPerLine)
            {
                for (uint32_t i = 0; i < word.length(); i++)
                {
                    currentLineLength++;
                    currentLine << word[i];
                    if (currentLineLength >= maxCharsPerLine)
                    {
                        result.push_back(currentLine.str());
                        currentLine.str("");
                        currentLine.clear();
                        currentLineLength = 0;
                        if (result.size() > maxLines) break;
                    }
                }
            }
            else
            {
                if (currentLineLength == 0 && currentLineLength + word.length() <= maxCharsPerLine)
                {
                    currentLineLength += word.length();
                    currentLine << word;
                }
                else if (currentLineLength > 0 && currentLineLength + word.length() + 1 <= maxCharsPerLine)
                {
                    currentLineLength++;
                    currentLine << ' ';
                    currentLineLength += word.length();
                    currentLine << word;
                }
            }
        }

        if (currentLineLength > 0)
        {
            result.push_back(currentLine.str());
        }

        if (result.size() > maxLines)
        {
            while (result.size() > maxLines)
            {
                result.pop_back();
            }
            if (result.size() > 0 && maxCharsPerLine > 2)
            {
                std::string& lastLine = result.back();
                lastLine = lastLine.substr(0, std::max((int)lastLine.length() - 3, (int)maxCharsPerLine - 3)) + "...";
            }
        }

        return result;
    }
}