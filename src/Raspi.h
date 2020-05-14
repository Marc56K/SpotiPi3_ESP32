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

class Raspi
{
public:
    Raspi(SettingsManager& settings);
    ~Raspi();

    unsigned long GetTimeInCurrentState();
    bool IsBusy();
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
};