#pragma once

#include <vector>

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "InputTypes.h"

namespace ego
{
    class InputDeviceController;

    class InputDevice
    {
    public:
        using ValueCollection = std::vector<InputDeviceKeyValue>;
        using KeyTypeCollection = std::vector<InputDeviceKeyType>;

        explicit InputDevice(InputDeviceKey _valueCount = 0);
        virtual ~InputDevice() = default;

        virtual InputDeviceType getType() const;

        const ValueCollection& getValues() const;
        const ValueCollection& getPreviousValues() const;
        const KeyTypeCollection& getKeyTypes() const;

        InputDeviceKeyType getKeyType(InputDeviceKey _valueIndex) const;
        InputDeviceKeyValue getValue(InputDeviceKey _valueIndex) const;
        InputDeviceKeyValue getPreviousValue(InputDeviceKey _valueIndex) const;
        InputDeviceKey getValueCount() const;

        virtual void update() = 0;

        EGO_RTTI_VIRTUAL_BASE(InputDevice);

    protected:
        void resizeValues(InputDeviceKey _valueCount);
        void setKeyType(InputDeviceKey _valueIndex, InputDeviceKeyType _keyType);
        void setValue(InputDeviceKey _valueIndex, InputDeviceKeyValue _value);

    private:
        friend class InputDeviceController;

        void storePreviousValues();

        ValueCollection m_values;
        ValueCollection m_previousValues;
        KeyTypeCollection m_keyTypes;
    };

    EGO_POINTER(InputDevice);
} // namespace ego
