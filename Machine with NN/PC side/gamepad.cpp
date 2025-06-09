#include "gamepad.h"
#include <math.h>

namespace HID
{

    int Gamepad::GetPort()
    {
        return cId + 1;
    }

    XINPUT_GAMEPAD *Gamepad::GetState()
    {
        return &state.Gamepad;
    }

    const Gamepad_Data &Gamepad::GetData()
    {
        return data;
    }

    bool Gamepad::CheckConnection()
    {
        int controllerId = -1;

        for (DWORD i = 0; i < XUSER_MAX_COUNT && controllerId == -1; i++)
        {
            XINPUT_STATE state;
            ZeroMemory(&state, sizeof(XINPUT_STATE));

            if (XInputGetState(i, &state) == ERROR_SUCCESS)
                controllerId = i;
        }

        cId = controllerId;

        return controllerId != -1;
    }

    // Returns false if the controller has been disconnected
    bool Gamepad::Refresh()
    {
        if (cId == -1)
            CheckConnection();

        if (cId != -1)
        {
            ZeroMemory(&state, sizeof(XINPUT_STATE));
            if (XInputGetState(cId, &state) != ERROR_SUCCESS)
            {
                cId = -1;
                return false;
            }

            float rawLX = state.Gamepad.sThumbLX;
            float rawLY = state.Gamepad.sThumbLY;

            float magnitude = sqrt(rawLX * rawLX + rawLY * rawLY);

            float normalizedLX = rawLX / magnitude;
            float normalizedLY = rawLY / magnitude;

            float normalizedMagnitude = 0;

            if (magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            {
                if (magnitude > 32767)
                    magnitude = 32767;

                magnitude -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

                normalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            }
            else
            {
                magnitude = 0.0;
                normalizedMagnitude = 0.0;
            }

            data.LX = normalizedLX * normalizedMagnitude;
            data.LY = normalizedLY * normalizedMagnitude;

            if (fabs(data.LX) < 0.01)
                data.LX = 0;
            if (fabs(data.LY) < 0.01)
                data.LY = 0;

            float rawRX = state.Gamepad.sThumbRX;
            float rawRY = state.Gamepad.sThumbRY;

            magnitude = sqrt(rawRX * rawRX + rawRY * rawRY);

            float normalizedRX = rawRX / magnitude;
            float normalizedRY = rawRY / magnitude;

            normalizedMagnitude = 0;

            if (magnitude > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
            {
                if (magnitude > 32767)
                    magnitude = 32767;

                magnitude -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

                normalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
            }
            else
            {
                magnitude = 0.0;
                normalizedMagnitude = 0.0;
            }

            data.RX = normalizedRX * normalizedMagnitude;
            data.RY = normalizedRY * normalizedMagnitude;

            if (fabs(data.RX) < 0.01)
                data.RX = 0;
            if (fabs(data.RY) < 0.01)
                data.RY = 0;

            data.LT = (float)state.Gamepad.bLeftTrigger / 255;
            data.RT = (float)state.Gamepad.bRightTrigger / 255;

            data.buttonsPressed = state.Gamepad.wButtons;

            return true;
        }
        return false;
    }

    bool Gamepad::IsPressed(WORD button)
    {
        return (state.Gamepad.wButtons & button) != 0;
    }

    void Gamepad::SetVibration(WORD left, WORD right)
    {
        XINPUT_VIBRATION vibration;
        ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
        vibration.wLeftMotorSpeed = left;  // use any value between 0-65535 here
        vibration.wRightMotorSpeed = right; // use any value between 0-65535 here
        XInputSetState(cId, &vibration);
    }
}