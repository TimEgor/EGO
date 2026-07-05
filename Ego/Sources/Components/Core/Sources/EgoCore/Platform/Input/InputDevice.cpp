#include "InputDevice.h"

#include <algorithm>

#include "EgoCore/Assert/AssertCore.h"

ego::InputDevice::InputDevice(InputDeviceKey _valueCount)
{
    resizeValues(_valueCount);
}

ego::InputDeviceType ego::InputDevice::getType() const
{
    return getObjectTypeMetaInfoID();
}

const ego::InputDevice::ValueCollection& ego::InputDevice::getValues() const
{
    return m_values;
}

const ego::InputDevice::ValueCollection& ego::InputDevice::getPreviousValues() const
{
    return m_previousValues;
}

const ego::InputDevice::KeyTypeCollection& ego::InputDevice::getKeyTypes() const
{
    return m_keyTypes;
}

ego::InputDeviceKeyType ego::InputDevice::getKeyType(InputDeviceKey _valueIndex) const
{
    EGO_ASSERT(_valueIndex < m_keyTypes.size());
    if (_valueIndex >= m_keyTypes.size())
    {
        return InputDeviceKeyType::Button;
    }

    return m_keyTypes[_valueIndex];
}

ego::InputDeviceKeyValue ego::InputDevice::getValue(InputDeviceKey _valueIndex) const
{
    EGO_ASSERT(_valueIndex < m_values.size());
    if (_valueIndex >= m_values.size())
    {
        return MinInputDeviceKeyValue;
    }

    return m_values[_valueIndex];
}

ego::InputDeviceKeyValue ego::InputDevice::getPreviousValue(InputDeviceKey _valueIndex) const
{
    EGO_ASSERT(_valueIndex < m_previousValues.size());
    if (_valueIndex >= m_previousValues.size())
    {
        return MinInputDeviceKeyValue;
    }

    return m_previousValues[_valueIndex];
}

ego::InputDeviceKey ego::InputDevice::getValueCount() const
{
    return static_cast<InputDeviceKey>(m_values.size());
}

void ego::InputDevice::resizeValues(InputDeviceKey _valueCount)
{
    m_values.resize(_valueCount, MinInputDeviceKeyValue);
    m_previousValues.resize(_valueCount, MinInputDeviceKeyValue);
    m_keyTypes.resize(_valueCount, InputDeviceKeyType::Button);
}

void ego::InputDevice::setKeyType(InputDeviceKey _valueIndex, InputDeviceKeyType _keyType)
{
    EGO_ASSERT(_valueIndex < m_keyTypes.size());
    if (_valueIndex >= m_keyTypes.size())
    {
        return;
    }

    m_keyTypes[_valueIndex] = _keyType;
}

void ego::InputDevice::setValue(InputDeviceKey _valueIndex, InputDeviceKeyValue _value)
{
    EGO_ASSERT(_valueIndex < m_values.size());
    if (_valueIndex >= m_values.size())
    {
        return;
    }

    if (getKeyType(_valueIndex) == InputDeviceKeyType::Button)
    {
        m_values[_valueIndex] = std::clamp(_value, MinInputDeviceKeyValue, MaxInputDeviceFixedKeyValue);
        return;
    }

    m_values[_valueIndex] = _value;
}

void ego::InputDevice::storePreviousValues()
{
    m_previousValues = m_values;
}
