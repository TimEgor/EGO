#pragma once

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename PropertyType>
    PropertyType* CastPropertyMetaInfo(PropertyMetaInfo* _propertyMetaInfo);

    template <typename PropertyType>
    const PropertyType* CastPropertyMetaInfo(const PropertyMetaInfo* _propertyMetaInfo);
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/PropertyMetaInfoCast.hpp"
