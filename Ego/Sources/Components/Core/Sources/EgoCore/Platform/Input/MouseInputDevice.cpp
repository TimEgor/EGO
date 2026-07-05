#include "MouseInputDevice.h"

namespace
{
    ego::InputDeviceKey ToInputDeviceKey(ego::MouseInputKey _key)
    {
        return static_cast<ego::InputDeviceKey>(_key);
    }
} // namespace

ego::MouseInputDevice::MouseInputDevice()
    : InputDevice(MouseInputKeyCount)
{
    setKeyType(ToInputDeviceKey(MouseInputKey::AxisX), InputDeviceKeyType::Position);
    setKeyType(ToInputDeviceKey(MouseInputKey::AxisY), InputDeviceKeyType::Position);
    setKeyType(ToInputDeviceKey(MouseInputKey::Wheel), InputDeviceKeyType::Axis);
}

ego::InputDeviceType ego::MouseInputDevice::getType() const
{
    return MouseInputDevice::GetMetaInfoID();
}
