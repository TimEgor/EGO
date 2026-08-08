#pragma once

#include <type_traits>

#include "EgoCore/RTTI/Property/PropertyValue.h"
#include "EgoCore/RTTI/Property/Types/Scalar/ScalarPropertyMetaInfo.h"

namespace ego::rtti
{
    class ScalarPropertyValue : public PropertyValue
    {
    public:
        ScalarPropertyValue(void* _object, const ScalarPropertyMetaInfo& _propertyMetaInfo);
        ScalarPropertyValue(const void* _object, const ScalarPropertyMetaInfo& _propertyMetaInfo);

        const ScalarPropertyMetaInfo& getMetaInfo() const override;

        template <typename T>
        requires std::is_arithmetic_v<T>
        const T& getValue() const;

        template <typename T>
        requires std::is_arithmetic_v<T>
        bool setValue(const T& _value);

        EGO_RTTI_VIRTUAL(ScalarPropertyValue, PropertyValue);
    };

    template <typename T>
    requires std::is_arithmetic_v<T>
    class TypedScalarPropertyValue final : public ScalarPropertyValue
    {
    public:
        TypedScalarPropertyValue(void* _object, const TypedScalarPropertyMetaInfo<T>& _propertyMetaInfo);
        TypedScalarPropertyValue(const void* _object, const TypedScalarPropertyMetaInfo<T>& _propertyMetaInfo);

        const TypedScalarPropertyMetaInfo<T>& getMetaInfo() const override;

        EGO_RTTI_VIRTUAL(TypedScalarPropertyValue, ScalarPropertyValue);
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Scalar/ScalarPropertyValue.hpp"
