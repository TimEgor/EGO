#include "Win32InputDeviceProvider.h"

#include "Win32KeyboardInputDevice.h"
#include "Win32MouseInputDevice.h"

ego::InputDeviceProvider::DeviceCollection ego::win32::Win32InputDeviceProvider::createDevices()
{
    DeviceCollection devices;
    devices.push_back(new Win32KeyboardInputDevice());
    devices.push_back(new Win32MouseInputDevice());

    return devices;
}
