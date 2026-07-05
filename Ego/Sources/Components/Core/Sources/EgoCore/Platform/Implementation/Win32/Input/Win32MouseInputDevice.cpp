#include "Win32MouseInputDevice.h"

namespace
{
    ego::InputDeviceKey ToInputDeviceKey(ego::MouseInputKey _key)
    {
        return static_cast<ego::InputDeviceKey>(_key);
    }

    ego::InputDeviceKeyValue GetButtonValue(int _virtualKey)
    {
        const SHORT keyState = GetAsyncKeyState(_virtualKey);
        return (keyState & 0x8000) ? ego::MaxInputDeviceFixedKeyValue : ego::MinInputDeviceKeyValue;
    }
} // namespace

ego::win32::Win32MouseInputDevice::Win32MouseInputDevice()
{
    resetValues();
}

void ego::win32::Win32MouseInputDevice::update()
{
    updateCursorAxes();
    updateButtons();
}

void ego::win32::Win32MouseInputDevice::updateCursorAxes()
{
    POINT cursorPosition = {};
    if (!GetCursorPos(&cursorPosition))
    {
        setValue(ToInputDeviceKey(MouseInputKey::AxisX), MinInputDeviceKeyValue);
        setValue(ToInputDeviceKey(MouseInputKey::AxisY), MinInputDeviceKeyValue);
        setValue(ToInputDeviceKey(MouseInputKey::Wheel), MinInputDeviceKeyValue);
        return;
    }

    setValue(ToInputDeviceKey(MouseInputKey::AxisX), static_cast<InputDeviceKeyValue>(cursorPosition.x));
    setValue(ToInputDeviceKey(MouseInputKey::AxisY), static_cast<InputDeviceKeyValue>(cursorPosition.y));
    setValue(ToInputDeviceKey(MouseInputKey::Wheel), MinInputDeviceKeyValue);
}

void ego::win32::Win32MouseInputDevice::updateButtons()
{
    setValue(ToInputDeviceKey(MouseInputKey::ButtonLeft), GetButtonValue(VK_LBUTTON));
    setValue(ToInputDeviceKey(MouseInputKey::ButtonRight), GetButtonValue(VK_RBUTTON));
    setValue(ToInputDeviceKey(MouseInputKey::ButtonMiddle), GetButtonValue(VK_MBUTTON));
    setValue(ToInputDeviceKey(MouseInputKey::ButtonFour), GetButtonValue(VK_XBUTTON1));
    setValue(ToInputDeviceKey(MouseInputKey::ButtonFive), GetButtonValue(VK_XBUTTON2));
    setValue(ToInputDeviceKey(MouseInputKey::ButtonSix), MinInputDeviceKeyValue);
    setValue(ToInputDeviceKey(MouseInputKey::ButtonSeven), MinInputDeviceKeyValue);
    setValue(ToInputDeviceKey(MouseInputKey::ButtonEight), MinInputDeviceKeyValue);
}

void ego::win32::Win32MouseInputDevice::resetValues()
{
    setValue(ToInputDeviceKey(MouseInputKey::AxisX), MinInputDeviceKeyValue);
    setValue(ToInputDeviceKey(MouseInputKey::AxisY), MinInputDeviceKeyValue);
    setValue(ToInputDeviceKey(MouseInputKey::Wheel), MinInputDeviceKeyValue);
    setValue(ToInputDeviceKey(MouseInputKey::ButtonLeft), MinInputDeviceKeyValue);
    setValue(ToInputDeviceKey(MouseInputKey::ButtonRight), MinInputDeviceKeyValue);
    setValue(ToInputDeviceKey(MouseInputKey::ButtonMiddle), MinInputDeviceKeyValue);
    setValue(ToInputDeviceKey(MouseInputKey::ButtonFour), MinInputDeviceKeyValue);
    setValue(ToInputDeviceKey(MouseInputKey::ButtonFive), MinInputDeviceKeyValue);
    setValue(ToInputDeviceKey(MouseInputKey::ButtonSix), MinInputDeviceKeyValue);
    setValue(ToInputDeviceKey(MouseInputKey::ButtonSeven), MinInputDeviceKeyValue);
    setValue(ToInputDeviceKey(MouseInputKey::ButtonEight), MinInputDeviceKeyValue);
}
