#pragma once

#include <type_traits>

#include "EgoCore/RTTI/Type/TypeMetaInfo.h"

namespace ego::rtti
{
    template <typename PropertyValueType>
    PropertyValueType* CastPropertyValue(PropertyValue* _propertyValue)
    {
        static_assert(std::is_base_of_v<PropertyValue, PropertyValueType>);

        if (!_propertyValue || !IsObjectBasedOn<PropertyValueType>(*_propertyValue))
        {
            return nullptr;
        }

        return static_cast<PropertyValueType*>(_propertyValue);
    }

    template <typename PropertyValueType>
    const PropertyValueType* CastPropertyValue(const PropertyValue* _propertyValue)
    {
        static_assert(std::is_base_of_v<PropertyValue, PropertyValueType>);

        if (!_propertyValue || !IsObjectBasedOn<PropertyValueType>(*_propertyValue))
        {
            return nullptr;
        }

        return static_cast<const PropertyValueType*>(_propertyValue);
    }
} // namespace ego::rtti
