#include "Raspi.h"

#define MAX_START_DURATION 90000 // 90 sec
#define AUTO_SHUTDOWN_DELAY 600000 // 10 min
#define SHUTDOWN_DURATION 15000 // 15 sec

Raspi::Raspi(SettingsManager& settings)
    : _settings(settings)
{
    _lastStateChange = millis();
    _lastHeartbeat = millis();
    _restartDelay = 0;
}

Raspi::~Raspi()
{

}

void Raspi::SetState(RaspiState state)
{
    if (state != _raspiInfo.state)
    {
        _raspiInfo.state = state;
        _lastStateChange = millis();
    }
}

unsigned long Raspi::GetTimeInCurrentState()
{
    auto now = millis();
    if (now >= _lastStateChange)
    {
        return now - _lastStateChange;
    }

    // handle time overflow
    unsigned long t = -1;
    t -= _lastStateChange;
    return t + now;
}

 void Raspi::RefreshBusyState()
{
    if (_raspiInfo.state != RaspiState::Idle && _raspiInfo.state != RaspiState::Playing)
    {
        _raspiInfo.isBusy = true;
        return;
    }

    if (!_raspiInfo.playerStarted)
    {
        _raspiInfo.isBusy = true;
        return;
    }

    unsigned long delta = 0;
    auto now = millis();
    if (now >= _lastHeartbeat)
    {
        delta = now - _lastHeartbeat;
    }
    else
    {
        // handle time overflow
        unsigned long t = -1;
        t -= _lastHeartbeat;
        delta = t + now;
    }
    _raspiInfo.isBusy = delta > 2000;
}

SerialInterface& Raspi::Serial()
{
    return _serial;
}

void Raspi::SendSettings()
{
    _serial.WriteKeyValue("wifiSsid", _settings.GetStringValue(Setting::WIFI_SSID));
    _serial.WriteKeyValue("wifiKey", _settings.GetStringValue(Setting::WIFI_KEY));
    _serial.WriteKeyValue("spotifyUser", _settings.GetStringValue(Setting::SPOTIFY_USER));
    _serial.WriteKeyValue("spotifyPassword", _settings.GetStringValue(Setting::SPOTIFY_PASSWORD));
    _serial.WriteKeyValue("spotifyClientId", _settings.GetStringValue(Setting::SPOTIFY_CLIENT_ID));
    _serial.WriteKeyValue("spotifyClientSecret", _settings.GetStringValue(Setting::SPOTIFY_CLIENT_SECRET));    
}

RaspiInfo& Raspi::Update(const PowerState& ps, const InputState& is)
{
    auto json = _serial.Read();
    bool jsonReceived = json.size() > 0;

    if (jsonReceived)
    {
        _lastHeartbeat = millis();
    }

    if (_raspiInfo.state == RaspiState::ShuttingDown)
    {
        if (is.buttons[0] > 0 && ps.sufficientPower)
        {
            _restartDelay = GetTimeInCurrentState();
            _restartDelay = _restartDelay < SHUTDOWN_DURATION ? (SHUTDOWN_DURATION - _restartDelay) : 0;
            SetState(RaspiState::Restart);
        }
        else if (GetTimeInCurrentState() > SHUTDOWN_DURATION)
        {
            digitalWrite(PI_POWER_PIN, LOW);
            SetState(RaspiState::Shutdown);
        }
    }

    if (_raspiInfo.state == RaspiState::Shutdown)
    {
        if (is.buttons[0] > 0 && ps.sufficientPower)
        {
            _restartDelay = 0;
            SetState(RaspiState::Restart);
        }
    }

    if (_raspiInfo.state == RaspiState::Restart)
    {
        if (GetTimeInCurrentState() >= _restartDelay)
        {
            digitalWrite(PI_POWER_PIN, LOW);
            if (ps.sufficientPower)
            {
                delay(10);
                digitalWrite(PI_POWER_PIN, HIGH);
                SetState(RaspiState::Starting);
            }
            else
            {
                SetState(RaspiState::Shutdown);
            }
        }
    }

    if (_raspiInfo.state == RaspiState::Starting)
    {
        if (jsonReceived)
        {
            SendSettings();
            SetState(RaspiState::Idle);
        }
        else if (GetTimeInCurrentState() > MAX_START_DURATION)
        {
            digitalWrite(PI_POWER_PIN, LOW);
            SetState(RaspiState::StartTimeout);
        }
    }

    if (_raspiInfo.state == RaspiState::Idle || _raspiInfo.state == RaspiState::Playing)
    {
        if (jsonReceived)
        {
            if (json.containsKey("state") && json["state"] == "play")
                SetState(RaspiState::Playing);
            else
                SetState(RaspiState::Idle);

            _raspiInfo.playerStarted = !(json.containsKey("error") && json["error"] == "Not connected");

            if (json.containsKey("online"))
                _raspiInfo.online = json["online"].as<bool>();
            else
                _raspiInfo.online = false;

            _raspiInfo.player.Clear();

            if (json.containsKey("tracks"))
                _raspiInfo.player.tracks = json["tracks"].as<int>();

            if (json.containsKey("track"))
                _raspiInfo.player.track = json["track"].as<int>();

            if (json.containsKey("time"))
                _raspiInfo.player.time = json["time"].as<float>();

            if (json.containsKey("state"))
                _raspiInfo.player.state = json["state"].as<char*>();

            if (json.containsKey("playlistId"))
                _raspiInfo.player.playlistId = json["playlistId"].as<char*>();

            if (json.containsKey("playlistName"))
                _raspiInfo.player.playlistName = StringUtils::Utf8ToLatin1String(json["playlistName"].as<char*>());

            if (json.containsKey("album"))
                _raspiInfo.player.album = StringUtils::Utf8ToLatin1String(json["album"].as<char*>());

            if (json.containsKey("artist"))
                _raspiInfo.player.artist = StringUtils::Utf8ToLatin1String(json["artist"].as<char*>());

            if (json.containsKey("title"))
                _raspiInfo.player.title = StringUtils::Utf8ToLatin1String(json["title"].as<char*>());

            if (_raspiInfo.player.playlistId != "" && _raspiInfo.player.playlistName != "")
                _raspiInfo.player.playlist = StringUtils::Trim(StringUtils::Replace(_raspiInfo.player.playlistName, _raspiInfo.player.playlistId, ""));

            _serial.WriteKeyValue("playlist", is.rfId);
        }

        if (jsonReceived || abs(is.potiDelta) > 1)
        {
            _serial.WriteKeyValue("volume",  is.potiValue);
        }

        if (is.buttons[1] > 0)
        {
            if (is.buttons[1] < 3)
                _serial.WriteKeyValue("skipPrevious", is.buttons[1]);
            else
                _serial.WriteKeyValue("skipToStart", 1);
        }

        if (is.buttons[3] > 0)
        {
            if (is.buttons[3] < 3)
                _serial.WriteKeyValue("skipNext", is.buttons[3]);
            else
                _serial.WriteKeyValue("skipNext", 10);
        }

        if (is.buttons[2] % 2 == 1)
        {
            _serial.WriteKeyValue("togglePlayPause", 1);
        }

        if (is.buttons[0] > 0 || (_raspiInfo.state == RaspiState::Idle && GetTimeInCurrentState() > AUTO_SHUTDOWN_DELAY))
        {
            _serial.WriteKeyValue("shutdown", 1);
            SetState(RaspiState::ShuttingDown);
        }
    }
    else
    {
        _raspiInfo.player.Clear();
    }
    
    RefreshBusyState();

    return _raspiInfo;
}