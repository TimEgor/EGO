#pragma once

#include "EgoCore/RTTI/Property/Types/Scalar/ScalarPropertyValue.h"

namespace ego::rtti
{
    template <typename T>
    requires std::is_arithmetic_v<T>
    template <typename Speed>
    requires std::same_as<std::remove_cvref_t<Speed>, T>
    constexpr ScalarPropertyOptions<T> TypedScalarPropertyMetaInfo<T>::MakePropertyOptions(Speed&& _speed)
    {
        return ScalarPropertyOptions<T>{std::forward<Speed>(_speed)};
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    template <typename Speed, typename MinValue>
    requires std::same_as<std::remove_cvref_t<Speed>, T> && std::same_as<std::remove_cvref_t<MinValue>, T>
    constexpr ScalarPropertyOptions<T> TypedScalarPropertyMetaInfo<T>::MakePropertyOptions(Speed&& _speed, MinValue&& _minValue)
    {
        return ScalarPropertyOptions<T>{std::forward<Speed>(_speed), std::forward<MinValue>(_minValue)};
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    template <typename Speed, typename MinValue, typename MaxValue>
    requires std::same_as<std::remove_cvref_t<Speed>, T> && std::same_as<std::remove_cvref_t<MinValue>, T> && std::same_as<std::remove_cvref_t<MaxValue>, T>
    constexpr ScalarPropertyOptions<T> TypedScalarPropertyMetaInfo<T>::MakePropertyOptions(Speed&& _speed, MinValue&& _minValue, MaxValue&& _maxValue)
    {
        return ScalarPropertyOptions<T>{std::forward<Speed>(_speed), std::forward<MinValue>(_minValue), std::forward<MaxValue>(_maxValue)};
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    TypedScalarPropertyMetaInfo<T>::TypedScalarPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst)
        : TypedScalarPropertyMetaInfo(_name, _offset, _isConst, ScalarPropertyOptions<T>())
    {
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    TypedScalarPropertyMetaInfo<T>::TypedScalarPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, ScalarPropertyOptions<T> _options)
        : ScalarPropertyMetaInfo(_name, _offset, _isConst),
          m_options(_options)
    {
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    template <typename... PropertyArguments>
    requires(
        sizeof...(PropertyArguments) > 0 && (!std::is_same_v<std::remove_cvref_t<PropertyArguments>, ScalarPropertyOptions<T>> && ...) &&
        requires { TypedScalarPropertyMetaInfo<T>::MakePropertyOptions(std::declval<PropertyArguments>()...); })
    TypedScalarPropertyMetaInfo<T>::TypedScalarPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, PropertyArguments&&... _propertyArguments)
        : TypedScalarPropertyMetaInfo(_name, _offset, _isConst, MakePropertyOptions(std::forward<PropertyArguments>(_propertyArguments)...))
    {
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    PropertyValuePointer TypedScalarPropertyMetaInfo<T>::makePropertyValue(void* _object) const
    {
        return MakeIntrusive<TypedScalarPropertyValue<T>>(_object, *this);
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    PropertyValuePointer TypedScalarPropertyMetaInfo<T>::makePropertyValue(const void* _object) const
    {
        return MakeIntrusive<TypedScalarPropertyValue<T>>(_object, *this);
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    ScalarType TypedScalarPropertyMetaInfo<T>::getScalarType() const
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            return ScalarType{ScalarKind::Boolean, sizeof(T)};
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            return ScalarType{ScalarKind::FloatingPoint, sizeof(T)};
        }
        else if constexpr (std::is_signed_v<T>)
        {
            return ScalarType{ScalarKind::SignedInteger, sizeof(T)};
        }
        else
        {
            return ScalarType{ScalarKind::UnsignedInteger, sizeof(T)};
        }
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    const T& TypedScalarPropertyMetaInfo<T>::getSpeed() const
    {
        return m_options.m_speed;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    const T& TypedScalarPropertyMetaInfo<T>::getMinValue() const
    {
        return m_options.m_minValue;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    const T& TypedScalarPropertyMetaInfo<T>::getMaxValue() const
    {
        return m_options.m_maxValue;
    }
} // namespace ego::rtti
