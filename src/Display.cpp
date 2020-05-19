#include "Display.h"
#include "Log.h"
#include "StringUtils.h"

Display::Display()
    : _paint(_buffer, 200, 200)
{

}

Display::~Display()
{
    
}

void Display::Init()
{
    Log().Info("MAIN") << "e-Paper init" << std::endl;
    if (_epd.Init() != 0)
      Log().Error("MAIN") << "e-Paper init failed" << std::endl;

    _paint.Clear(0);
    _epd.DisplayPart(_paint.GetImage(), true);
    _paint.Clear(1);
    _epd.DisplayPart(_paint.GetImage(), false);
}

void Display::Clear()
{
    _paint.Clear(1);
}

bool Display::Present(bool wait)
{
    bool result = false;

    if (wait)
    {
        WaitUntilIdle();
    }

    if (!_epd.IsBusy())
    {
        _epd.DisplayPart(_paint.GetImage(), false);
        result = true;
    }

    if (wait)
    {
        WaitUntilIdle();
    }

    return result;
}

void Display::WaitUntilIdle()
{
    _epd.WaitUntilIdle();
}

void Display::RenderCenterText(const uint32_t x, const uint32_t y, const uint32_t maxWidth, const uint32_t maxLines, const std::string& txt)
{
    sFONT* font = &Consolas24;
    int textWidth = font->Width * txt.length();
    if (textWidth <= maxWidth)
    {
        int xStart= (int)x - textWidth / 2;
        _paint.DrawStringAt(xStart, y, txt.c_str(), font, 0);
    }
    else if (maxLines > 1)
    {
        int maxCharsPerLine = max(0, (int)maxWidth / font->Width);
        int yPos = y;
        int line = 0;
        for (int i = 0; i < txt.length() && line < maxLines; i+= maxCharsPerLine)
        {
            std::string s = StringUtils::Trim(txt.substr(i, min((int)txt.length(), maxCharsPerLine)));
            textWidth = font->Width * s.length();
            int xStart= (int)x - textWidth / 2;
            _paint.DrawStringAt(xStart, yPos, s.c_str(), font, 0);
            yPos += font->Height;
            line++;
        }      
    }
    else
    {
        int maxChars = max(0, (int)maxWidth / font->Width - 3);
        std::string s = txt.substr(0, min((int)txt.length(), maxChars)) + "...";
        textWidth = font->Width * s.length();
        int xStart= (int)x - textWidth / 2;
        _paint.DrawStringAt(xStart, y, s.c_str(), font, 0);
    }    
}

void Display::RenderRectangle(const uint32_t x0, const uint32_t y0, const uint32_t x1, const uint32_t y1)
{
    _paint.DrawFilledRectangle(x0, y0, x1, y1, 0);
}

void Display::RenderSetupScreen(const std::string& wifiSsid, const std::string& setupKey)
{    
    _paint.DrawStringAt(0, 0, "  Setup Mode", &Font20, 0);
    _paint.DrawStringAt(0, 40,  "WiFi-SSID: ", &Font16, 0);
    _paint.DrawStringAt(0, 60,  wifiSsid.c_str(), &Font20, 0);
    _paint.DrawStringAt(0, 100, "WiFi-KEY:", &Font16, 0);
    _paint.DrawStringAt(0, 120, setupKey.c_str(), &Font24, 0);    
}

void Display::RenderPowerOffScreen()
{
    _paint.DrawImage(0, 0, &IMG_power_off);
}

void Display::RenderBatteryIndicator(const uint32_t x, const uint32_t y, const bool charging, const bool powerIn, const int batLevel)
{
    int level = batLevel;
    if (charging)
    {
        level = ((millis() / 750) % 5) * 25;
    }

    level = (int)round((float)level / 25);
    sIMAGE* img = nullptr;
    switch(level)
    {
        case 0:
            img = &IMG_bat_0;
            break;
        case 1:
            img = &IMG_bat_25;
            break;
        case 2:
            img = &IMG_bat_50;
            break;
        case 3:
            img = &IMG_bat_75;
            break;
        default:
            img = &IMG_bat_100;
    }

    _paint.DrawImage(x, y, img);

    if (powerIn)
    {
        _paint.DrawImage(x - 7, y + 12, &IMG_power_in);
    }    
}

void Display::RenderVolumeIndicator(const uint32_t x, const uint32_t y, const int volLevel)
{
    _paint.DrawImage(x, y, &IMG_volume);

    uint32_t xStart = x + 28;
    uint32_t yStart = y + 3;

    int vol = (int)round((float)volLevel / 10);
    for (int i = 1; i <= vol; i++)
    {
        uint32_t xPos = xStart + (i - 1) * 8;
        _paint.DrawFilledRectangle(xPos, yStart, xPos + 5, yStart + 13, 0);
    }
}

void Display::RenderOnlineIndicator(const uint32_t x, const uint32_t y, const bool online)
{
    if (online)
    {
        _paint.DrawImage(x, y, &IMG_online);
    }
    else
    {
        _paint.DrawImage(x, y, &IMG_offline);
    }    
}

void Display::RenderStandbyIcon(const uint32_t x, const uint32_t y)
{
    _paint.DrawImage(x, y, &IMG_standby);
}

void Display::RenderPlayIcon(const uint32_t x, const uint32_t y)
{
    _paint.DrawImage(x, y, &IMG_play);
}

void Display::RenderPauseIcon(const uint32_t x, const uint32_t y)
{
    _paint.DrawImage(x, y, &IMG_pause);
}

void Display::RenderNextTrackIcon(const uint32_t x, const uint32_t y)
{
    _paint.DrawImage(x, y, &IMG_next_track);
}

void Display::RenderPrevTrackIcon(const uint32_t x, const uint32_t y)
{
    _paint.DrawImage(x, y, &IMG_prev_track);
}

void Display::RenderBusyAnimation(const uint32_t x, const uint32_t y)
{
    int i = ((millis() / 750) % 6);
    switch(i)
    {
        case 0: 
            _paint.DrawImage(x, y, &IMG_spinner_0);
            break;
        case 1: 
            _paint.DrawImage(x, y, &IMG_spinner_1);
            break;
        case 2: 
            _paint.DrawImage(x, y, &IMG_spinner_2);
            break;
        case 3: 
            _paint.DrawImage(x, y, &IMG_spinner_3);
            break;
        case 4: 
            _paint.DrawImage(x, y, &IMG_spinner_4);
            break;
        default:
            _paint.DrawImage(x, y, &IMG_spinner_5);
    }
}