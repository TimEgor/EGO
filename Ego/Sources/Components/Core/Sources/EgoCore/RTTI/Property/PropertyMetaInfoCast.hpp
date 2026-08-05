#pragma once

#include <type_traits>

#include "EgoCore/RTTI/Type/TypeMetaInfo.h"

namespace ego::rtti
{
    template <typename PropertyType>
    PropertyType* CastPropertyMetaInfo(PropertyMetaInfo* _propertyMetaInfo)
    {
        static_assert(std::is_base_of_v<PropertyMetaInfo, PropertyType>);

        if (!_propertyMetaInfo || !IsObjectBasedOn<PropertyType>(*_propertyMetaInfo))
        {
            return nullptr;
        }

        return static_cast<PropertyType*>(_propertyMetaInfo);
    }

    template <typename PropertyType>
    const PropertyType* CastPropertyMetaInfo(const PropertyMetaInfo* _propertyMetaInfo)
    {
        static_assert(std::is_base_of_v<PropertyMetaInfo, PropertyType>);

        if (!_propertyMetaInfo || !IsObjectBasedOn<PropertyType>(*_propertyMetaInfo))
        {
            return nullptr;
        }

        return static_cast<const PropertyType*>(_propertyMetaInfo);
    }
} // namespace ego::rtti
