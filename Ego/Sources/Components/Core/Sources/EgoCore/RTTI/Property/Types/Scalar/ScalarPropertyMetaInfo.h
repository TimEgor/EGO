#pragma once

#include <concepts>
#include <limits>
#include <type_traits>
#include <utility>

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename T>
    requires std::is_arithmetic_v<T>
    class TypedScalarPropertyValue;

    enum class ScalarKind
    {
        Boolean,
        SignedInteger,
        UnsignedInteger,
        FloatingPoint
    };

    struct ScalarType final
    {
        ScalarKind m_kind;
        size_t m_size;
    };

    class ScalarPropertyMetaInfo : public PropertyMetaInfo
    {
    public:
        ScalarPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst);

        virtual ScalarType getScalarType() const = 0;

        EGO_RTTI_VIRTUAL(ScalarPropertyMetaInfo, PropertyMetaInfo);
    };

    template <typename T>
    requires std::is_arithmetic_v<T>
    struct ScalarPropertyOptions final
    {
        T m_speed = T{1};
        T m_minValue = std::numeric_limits<T>::lowest();
        T m_maxValue = std::numeric_limits<T>::max();
    };

    template <typename T>
    requires std::is_arithmetic_v<T>
    class TypedScalarPropertyMetaInfo final : public ScalarPropertyMetaInfo
    {
    public:
        template <typename Speed>
        requires std::same_as<std::remove_cvref_t<Speed>, T>
        static constexpr ScalarPropertyOptions<T> MakePropertyOptions(Speed&& _speed);

        template <typename Speed, typename MinValue>
        requires std::same_as<std::remove_cvref_t<Speed>, T> && std::same_as<std::remove_cvref_t<MinValue>, T>
        static constexpr ScalarPropertyOptions<T> MakePropertyOptions(Speed&& _speed, MinValue&& _minValue);

        template <typename Speed, typename MinValue, typename MaxValue>
        requires std::same_as<std::remove_cvref_t<Speed>, T> && std::same_as<std::remove_cvref_t<MinValue>, T> && std::same_as<std::remove_cvref_t<MaxValue>, T>
        static constexpr ScalarPropertyOptions<T> MakePropertyOptions(Speed&& _speed, MinValue&& _minValue, MaxValue&& _maxValue);

        TypedScalarPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst);

        TypedScalarPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, ScalarPropertyOptions<T> _options);

        template <typename... PropertyArguments>
        requires(
            sizeof...(PropertyArguments) > 0 && (!std::is_same_v<std::remove_cvref_t<PropertyArguments>, ScalarPropertyOptions<T>> && ...) &&
            requires { TypedScalarPropertyMetaInfo<T>::MakePropertyOptions(std::declval<PropertyArguments>()...); })
        TypedScalarPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, PropertyArguments&&... _propertyArguments);

        EGO_RTTI_VIRTUAL(TypedScalarPropertyMetaInfo, ScalarPropertyMetaInfo);

        PropertyValuePointer makePropertyValue(void* _object) const override;
        PropertyValuePointer makePropertyValue(const void* _object) const override;

        ScalarType getScalarType() const override;
        const T& getSpeed() const;
        const T& getMinValue() const;
        const T& getMaxValue() const;

    private:
        const ScalarPropertyOptions<T> m_options;
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Scalar/ScalarPropertyMetaInfo.hpp"
