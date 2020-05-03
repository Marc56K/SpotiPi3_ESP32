#pragma once
#include <Arduino.h>
#include <string>

namespace InputManager
{
    struct InputState
    {
        uint32_t buttons[4];
        float poti;
        std::string rfId;
    };

    void Init();
    InputState GetInputState();
};