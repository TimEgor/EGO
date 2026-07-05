#include "Win32KeyboardInputDevice.h"

void ego::win32::Win32KeyboardInputDevice::update()
{
    for (InputDeviceKey key = 0; key < KeyboardInputKeyCount; ++key)
    {
        const SHORT keyState = GetAsyncKeyState(static_cast<int>(key));
        setValue(key, (keyState & 0x8000) ? MaxInputDeviceFixedKeyValue : MinInputDeviceKeyValue);
    }
}
