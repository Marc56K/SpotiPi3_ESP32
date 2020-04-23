#pragma once
#include <Arduino.h>

namespace InputManager
{
    struct InputState
    {
        uint32_t buttons[4];
        float poti;
        String rfId;
    };

    void Init();
    InputState GetInputState();
};