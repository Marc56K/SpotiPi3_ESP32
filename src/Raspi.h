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

struct PlayerInfo
{
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

    PlayerInfo()
    {
        Clear();
    }
};

struct RaspiInfo
{
    RaspiState state;
    bool isBusy;
    bool online;
    bool playerStarted;
    PlayerInfo player;

    RaspiInfo()
    {
        state = RaspiState::Restart;
        isBusy = false;
        online = false;
        playerStarted = false;
    }
};

class Raspi
{
public:
    Raspi(SettingsManager& settings);
    ~Raspi();

    unsigned long GetTimeInCurrentState();
    SerialInterface& Serial();
    RaspiInfo& Update(const PowerState& ps, const InputState& is);

private:
    void RefreshBusyState();
    void SetState(RaspiState state);
    void SendSettings();

private:
    RaspiInfo _raspiInfo;
    SettingsManager& _settings;
    SerialInterface _serial;
    unsigned long _lastStateChange;
    unsigned long _lastHeartbeat;
    unsigned long _restartDelay;
};