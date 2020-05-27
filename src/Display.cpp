#include "Display.h"
#include "Log.h"
#include "StringUtils.h"

Display::Display(SettingsManager& settings)
    : _settings(settings), _paint(_buffer, 200, 200)
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

void Display::RenderCenterText(const uint32_t x, const uint32_t y, const uint32_t maxWidth, const uint32_t maxLines, const std::string& txt, sFONT* font)
{
    if (font == nullptr)
        font = &Consolas24;

    uint32_t maxCharsPerLine = maxWidth / font->Width;
    auto lines = StringUtils::LineWrap(maxCharsPerLine, maxLines, txt);
    int yPos = y;
    int line = 0;
    for (auto lineIt = lines.begin(); lineIt != lines.end(); lineIt++)
    {
        int textWidth = font->Width * lineIt->length();
        int xStart= (int)x - textWidth / 2;
        _paint.DrawStringAt(xStart, yPos, (*lineIt).c_str(), font, 0);
        yPos += font->Height;
        line++;
    }
}

void Display::RenderRectangle(const uint32_t x0, const uint32_t y0, const uint32_t x1, const uint32_t y1)
{
    _paint.DrawFilledRectangle(x0, y0, x1, y1, 0);
}

void Display::RenderBatteryIndicator(const uint32_t x, const uint32_t y, const bool charging, const bool powerIn, const int batLevel)
{
    int level = batLevel;
    if (charging)
    {
        level = ((millis() / 750) % 5) * 25;
    }

    sIMAGE* img = nullptr;
    if (level <= 10)
    {
        img = &IMG_bat_0;
        if (!charging && (millis() / 750) % 2 == 0)
        {
            img = nullptr;
        }
    }
    else if (level <= 25)
        img = &IMG_bat_25;
    else if (level <= 50)
        img = &IMG_bat_50;
    else if (level <= 75)
        img = &IMG_bat_75;
    else
        img = &IMG_bat_100;

    if (img != nullptr)
    {
        _paint.DrawImage(x, y, img);
    }    

    if (powerIn)
    {
        _paint.DrawImage(x - 7, y + 12, &IMG_power_in);
    }    
}

void Display::RenderVolumeIndicator(const uint32_t x, const uint32_t y, const int volLevel)
{
    if (volLevel == 0)
    {
        _paint.DrawImage(x, y, &IMG_mute);
        return;
    }

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

void Display::RenderStandbyIcon()
{
    _paint.DrawImage(5, 40, &IMG_standby);
}

void Display::RenderPlayIcon()
{
    _paint.DrawImage(174, 40, &IMG_play);
}

void Display::RenderPauseIcon()
{
    _paint.DrawImage(174, 40, &IMG_pause);
}

void Display::RenderNextTrackIcon()
{
    _paint.DrawImage(174, 140, &IMG_next_track);
}

void Display::RenderPrevTrackIcon()
{
    _paint.DrawImage(5, 140, &IMG_prev_track);
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

void Display::RenderSetupScreen(const std::string& wifiSsid)
{    
    RenderCenterText(100, 0, 200, 1, "Setup Mode");
    RenderRectangle(0, 30, 199, 31);

    RenderCenterText(100, 50, 200, 1, "WiFi-SSID:", &Consolas20);
    RenderCenterText(100, 75, 200, 1, wifiSsid);
    RenderCenterText(100, 120, 200, 1, "WiFi-KEY:", &Consolas20);
    RenderCenterText(100, 145, 200, 1, _settings.GetStringValue(Setting::SETUP_KEY));

    RenderStandbyIcon();
}

void Display::RenderTimeoutScreen()
{
    _paint.DrawImage(0, 0, &IMG_timeout);
    RenderStandbyIcon();
}

void Display::RenderPowerOffScreen()
{
    _paint.DrawImage(0, 0, &IMG_power_off);
    RenderStandbyIcon();
}

void Display::RenderLowBatteryScreen()
{
    _paint.DrawImage(0, 0, &IMG_low_bat);
}

void Display::RenderMediaPlayerScreen(const RaspiInfo& pi, const InputState& is, const PowerState& ps)
{
    if (pi.player.playlistName != "")
        RenderCenterText(100, 2, 200, 1, pi.player.playlist);
    else if (is.rfId != "")
        RenderCenterText(100, 2, 200, 1, "[" + is.rfId + "]");
    else
        RenderCenterText(100, 2, 200, 1, _settings.GetStringValue(Setting::DISPLAY_NAME));    

    RenderRectangle(0, 30, 199, 31);

    RenderStandbyIcon();

    if (pi.player.tracks > 0)
    {
        if (pi.state == RaspiState::Playing)
            RenderPauseIcon();
        if (pi.state == RaspiState::Idle)
            RenderPlayIcon();
        
        if (pi.player.track > 0)
            RenderPrevTrackIcon();

        if (pi.player.track < pi.player.tracks - 1)
            RenderNextTrackIcon();
    }

    if (pi.isBusy)
    {
        RenderBusyAnimation(72, 72);
    }
    else if (pi.player.tracks > 0)
    {
        {
            RenderCenterText(100, 38, 140, 1, StringUtils::SecondsToTime(pi.player.time));
        }

        {
            std::stringstream ss;
            ss << pi.player.title;
            RenderCenterText(100, 72, 190, 2, ss.str());
        }

        {
            std::stringstream ss;
            ss << (pi.player.track + 1) << "/" << pi.player.tracks;
            RenderCenterText(100, 138, 140, 1, ss.str());
        }
    }
    else
    {
        _paint.DrawImage(0, 0, &IMG_no_playlist);
    }          

    RenderRectangle(0, 170, 199, 171);
    RenderVolumeIndicator(5, 176, is.potiValue);
    RenderOnlineIndicator(122, 176, pi.online);
    RenderBatteryIndicator(156, 176, ps.isCharging, ps.isOnUsb, ps.batteryLevel);
}