#pragma once

#include <Xinput.h>
#include <windows.h>

namespace HID
{
    struct Gamepad_Data
    {
        float LX;
        float LY;
        float RX;
        float RY;
        float LT;
        float RT;
        WORD buttonsPressed;
    };

    class Gamepad
    {
        int cId;
        XINPUT_STATE state;

        Gamepad_Data data;

    public:
        Gamepad() {}

        int GetPort();
        XINPUT_GAMEPAD *GetState();
        const Gamepad_Data& GetData();
        bool CheckConnection();
        bool Refresh();
        bool IsPressed(WORD);
        void SetVibration(WORD left, WORD right);
    };
}