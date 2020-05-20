#pragma once
#include "SetupManager.h"
#include "Raspi.h"
#include <epd1in54_V2.h>
#include <epdpaint.h>
#include <string>

class Display
{
public:
    Display();
    ~Display();

    void Init();

    void Clear();
    bool Present(bool wait = false);
    void WaitUntilIdle();

    void RenderCenterText(const uint32_t x, const uint32_t y, const uint32_t maxWidth, const uint32_t maxLines, const std::string& txt, sFONT* font = nullptr);
    void RenderRectangle(const uint32_t x0, const uint32_t y0, const uint32_t x1, const uint32_t y1);

    void RenderBatteryIndicator(const uint32_t x, const uint32_t y, const bool charging, const bool powerIn, const int batLevel);
    void RenderVolumeIndicator(const uint32_t x, const uint32_t y, const int volLevel);
    void RenderOnlineIndicator(const uint32_t x, const uint32_t y, const bool online);
    void RenderStandbyIcon();
    void RenderPlayIcon();
    void RenderPauseIcon();
    void RenderNextTrackIcon();
    void RenderPrevTrackIcon();
    void RenderBusyAnimation(const uint32_t x, const uint32_t y);

    void RenderSetupScreen(const std::string& wifiSsid, const std::string& setupKey);
    void RenderTimeoutScreen();
    void RenderPowerOffScreen();
    void RenderLowBatteryScreen();
    void RenderMediaPlayerScreen(const RaspiInfo& info, const InputState& is, const PowerState& ps);

private:
    unsigned char _buffer[200 * 200];
    Paint _paint;
    Epd _epd;
};