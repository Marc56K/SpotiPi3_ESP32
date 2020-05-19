#include "Raspi.h"

#define MAX_START_DURATION 90000 // 90 sec
#define AUTO_SHUTDOWN_DELAY 600000 // 10 min
#define SHUTDOWN_DURATION 15000 // 15 sec

Raspi::Raspi(SettingsManager& settings)
    : _state(RaspiState::Restart), _settings(settings)
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
    if (state != _state)
    {
        _state = state;
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

bool Raspi::IsBusy()
{
    if (_state != RaspiState::Idle && _state != RaspiState::Playing)
    {
        return true;
    }

    if (!_info.mopidyStarted)
    {
        return true;
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
    return delta > 2000;
}

SpotiPiInfo& Raspi::Info()
{
    return _info;
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

RaspiState Raspi::Update(const PowerState& ps, const InputState& is)
{
    auto json = _serial.Read();
    bool jsonReceived = json.size() > 0;

    if (jsonReceived)
    {
        _lastHeartbeat = millis();
    }

    if (_state == RaspiState::ShuttingDown)
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

    if (_state == RaspiState::Shutdown)
    {
        if (is.buttons[0] > 0 && ps.sufficientPower)
        {
            _restartDelay = 0;
            SetState(RaspiState::Restart);
        }
    }

    if (_state == RaspiState::Restart)
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

    if (_state == RaspiState::Starting)
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

    if (_state == RaspiState::Idle || _state == RaspiState::Playing)
    {
        if (jsonReceived)
        {
            if (json.containsKey("state") && json["state"] == "play")
                SetState(RaspiState::Playing);
            else
                SetState(RaspiState::Idle);

            _info.Clear();

            _info.mopidyStarted = !(json.containsKey("error") && json["error"] == "Not connected");

            if (json.containsKey("online"))
                _info.online = json["online"].as<bool>();

            if (json.containsKey("tracks"))
                _info.tracks = json["tracks"].as<int>();

            if (json.containsKey("track"))
                _info.track = json["track"].as<int>();

            if (json.containsKey("time"))
                _info.time = json["time"].as<float>();

            if (json.containsKey("state"))
                _info.state = json["state"].as<char*>();

            if (json.containsKey("playlistId"))
                _info.playlistId = json["playlistId"].as<char*>();

            if (json.containsKey("playlistName"))
                _info.playlistName = StringUtils::Utf8ToLatin1String(json["playlistName"].as<char*>());

            if (json.containsKey("album"))
                _info.album = StringUtils::Utf8ToLatin1String(json["album"].as<char*>());

            if (json.containsKey("artist"))
                _info.artist = StringUtils::Utf8ToLatin1String(json["artist"].as<char*>());

            if (json.containsKey("title"))
                _info.title = StringUtils::Utf8ToLatin1String(json["title"].as<char*>());

            if (_info.playlistId != "" && _info.playlistName != "")
            {
                _info.playlist = StringUtils::Trim(StringUtils::Replace(_info.playlistName, _info.playlistId, ""));
            }

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

        if (is.buttons[0] > 0 || !ps.sufficientPower || (_state == RaspiState::Idle && GetTimeInCurrentState() > AUTO_SHUTDOWN_DELAY))
        {
            _serial.WriteKeyValue("shutdown", 1);
            SetState(RaspiState::ShuttingDown);
        }
    }
    else
    {
        _info.Clear();
    }
    

    return _state;
}