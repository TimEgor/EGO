#include "KeyboardInputDevice.h"

ego::KeyboardInputDevice::KeyboardInputDevice()
    : InputDevice(KeyboardInputKeyCount)
{
}

ego::InputDeviceType ego::KeyboardInputDevice::getType() const
{
    return KeyboardInputDevice::GetMetaInfoID();
}
