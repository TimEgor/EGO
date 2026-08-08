#pragma once

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    PropertyMetaInfo& CastPropertyMetaInfo(PropertyMetaInfo& _propertyMetaInfo, const TypeMetaInfo& _targetTypeMetaInfo);
    const PropertyMetaInfo& CastPropertyMetaInfo(const PropertyMetaInfo& _propertyMetaInfo, const TypeMetaInfo& _targetTypeMetaInfo);

    template <typename PropertyType>
    PropertyType& CastPropertyMetaInfo(PropertyMetaInfo& _propertyMetaInfo);

    template <typename PropertyType>
    const PropertyType& CastPropertyMetaInfo(const PropertyMetaInfo& _propertyMetaInfo);
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/PropertyMetaInfoCast.hpp"
