#pragma once

#include <type_traits>

namespace ego::rtti
{
    template <typename PropertyType>
    PropertyType& CastPropertyMetaInfo(PropertyMetaInfo& _propertyMetaInfo)
    {
        static_assert(std::is_base_of_v<PropertyMetaInfo, PropertyType>);

        return static_cast<PropertyType&>(CastPropertyMetaInfo(_propertyMetaInfo, GetTypeMetaInfo<PropertyType>()));
    }

    template <typename PropertyType>
    const PropertyType& CastPropertyMetaInfo(const PropertyMetaInfo& _propertyMetaInfo)
    {
        static_assert(std::is_base_of_v<PropertyMetaInfo, PropertyType>);

        return static_cast<const PropertyType&>(CastPropertyMetaInfo(_propertyMetaInfo, GetTypeMetaInfo<PropertyType>()));
    }
} // namespace ego::rtti
