#pragma once

namespace ego::rtti
{
    template <typename T>
    requires std::is_arithmetic_v<T>
    TypedScalarPropertyValue<T>::TypedScalarPropertyValue(void* _object, const TypedScalarPropertyMetaInfo<T>& _propertyMetaInfo)
        : ScalarPropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    TypedScalarPropertyValue<T>::TypedScalarPropertyValue(const void* _object, const TypedScalarPropertyMetaInfo<T>& _propertyMetaInfo)
        : ScalarPropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    const T& ScalarPropertyValue::getValue() const
    {
        return PropertyValue::getValue<T>();
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    bool ScalarPropertyValue::setValue(const T& _value)
    {
        T* value = PropertyValue::tryGetMutableValue<T>();
        if (!value)
        {
            return false;
        }

        *value = _value;

        return true;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    const TypedScalarPropertyMetaInfo<T>& TypedScalarPropertyValue<T>::getMetaInfo() const
    {
        return static_cast<const TypedScalarPropertyMetaInfo<T>&>(PropertyValue::getMetaInfo());
    }
} // namespace ego::rtti
