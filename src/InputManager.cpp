#include "InputManager.h"
#include "PinMapping.h"
#include "Log.h"
#include <MFRC522.h>
//#include <esp32-hal.h>

MFRC522 mfrc522(RC522_SS_PIN, RC522_RST_PIN); 

#define DEBOUNCE_DELAY 150
#define BUTTON_TIMEOUT 500

namespace InputManager
{
    struct ButtonHandler
    {
        volatile uint32_t actualEventCount;
        uint32_t lastEventCount;
        uint32_t pressedCount;
        uint32_t lastPressedTime;

        ButtonHandler()
            : actualEventCount(0), lastEventCount(0), pressedCount(0), lastPressedTime(0)
        {
        }

        uint32_t ComputeNumButtonPresses()
        {
            uint32_t result = 0;
            uint32_t now = millis();
            uint32_t eventCount = actualEventCount;
            if (lastEventCount != eventCount)
            {
                lastEventCount = eventCount;                
                if ((now - lastPressedTime) > DEBOUNCE_DELAY)
                {
                    pressedCount++;
                }              
                lastPressedTime = now;
            }

            if (pressedCount > 0 && (now - lastPressedTime) > BUTTON_TIMEOUT)
            {
                result = pressedCount;
                pressedCount = 0;
            }
            return result;
        }

        void HandleButton(bool down)
        {
            if (down)
            {
                actualEventCount++;
            }
        }
    };

    ButtonHandler buttonHandler[4];
    void IRAM_ATTR HandleButton0() { buttonHandler[0].HandleButton(digitalRead(BT0_PIN)); }
    void IRAM_ATTR HandleButton1() { buttonHandler[1].HandleButton(digitalRead(BT1_PIN)); }
    void IRAM_ATTR HandleButton2() { buttonHandler[2].HandleButton(digitalRead(BT2_PIN)); }
    void IRAM_ATTR HandleButton3() { buttonHandler[3].HandleButton(digitalRead(BT3_PIN)); }

    float potiAvg = -1;
    float GetPoti()
    {
        float value = min(4095.0f, (float)analogRead(POTI_PIN));
        if (potiAvg < 0)
            potiAvg = value;
        else
            potiAvg = 0.8 * potiAvg + 0.2 * value;
        return potiAvg / 4095;
    }

    std::string _lastRfid = "";
    std::string GetRfid()
    {
        if(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
        {
            String content = "";
            for (byte i = 0; i < mfrc522.uid.size; i++) 
            {
                content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
                content.concat(String(mfrc522.uid.uidByte[i], HEX));
            }
            content.toUpperCase();
            _lastRfid = content.c_str();
        }
        return _lastRfid;
    }

    void Init()
    {
        pinMode(BT0_PIN, INPUT);
        attachInterrupt(BT0_PIN, HandleButton0, RISING);
        pinMode(BT1_PIN, INPUT);
        attachInterrupt(BT1_PIN, HandleButton1, RISING);
        pinMode(BT2_PIN, INPUT);
        attachInterrupt(BT2_PIN, HandleButton2, RISING);
        pinMode(BT3_PIN, INPUT);
        attachInterrupt(BT3_PIN, HandleButton3, RISING);

        pinMode(POTI_PIN, INPUT);

        mfrc522.PCD_Init();
    }

    InputState GetInputState()
    {
        InputState result = {};
        for (int i = 0; i < 4; i++)
        {
            result.buttons[i] = buttonHandler[i].ComputeNumButtonPresses();
        }
        result.poti = GetPoti();
        result.rfId = GetRfid();
        return result;
    }
}