#pragma once
#include <Arduino.h>
#include <string>

struct InputState
{
    uint32_t buttons[4];
    uint32_t potiValue;
    int32_t potiDelta;
    std::string rfId;

    bool AnyButton()
    {
        return buttons[0] + buttons[1] + buttons[2] + buttons[3] > 0;
    }
};

namespace InputManager
{
    void Init();
    InputState GetInputState();
};