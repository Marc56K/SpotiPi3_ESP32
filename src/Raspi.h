#pragma	once
#include "Log.h"
#include "PinMapping.h"
#include "InputManager.h"
#include "PowerManager.h"
#include "SettingsManager.h"
#include "SerialInterface.h"
#include "StringUtils.h"

enum RaspiState
{
    Restart,
    Starting,
    StartTimeout,
    Idle,
    Playing,
    ShuttingDown,
    Shutdown,
};

struct SpotiPiInfo
{
    bool online;
    bool mopidyStarted;
    int tracks;
    int track;
    float time;
    std::string state;
    std::string playlistId;
    std::string playlistName;
    std::string playlist;
    std::string album;
    std::string artist;
    std::string title;

    void Clear()
    {
        online = false;
        mopidyStarted = false;
        tracks = 0;
        track = 0;
        time = 0;
        state = "";
        playlistId = "";
        playlistName = "";
        playlist = "";
        album = "";
        artist = "";
        title = "";
    }

    SpotiPiInfo()
    {
        Clear();
    }
};

class Raspi
{
public:
    Raspi(SettingsManager& settings);
    ~Raspi();

    unsigned long GetTimeInCurrentState();
    bool IsBusy();
    SpotiPiInfo& Info();
    SerialInterface& Serial();
    RaspiState Update(const PowerState& ps, const InputState& is);

private:
    void SetState(RaspiState state);
    void SendSettings();

private:
    RaspiState _state;
    SettingsManager& _settings;
    SerialInterface _serial;
    unsigned long _lastStateChange;
    unsigned long _lastHeartbeat;
    unsigned long _restartDelay;

    SpotiPiInfo _info;
};