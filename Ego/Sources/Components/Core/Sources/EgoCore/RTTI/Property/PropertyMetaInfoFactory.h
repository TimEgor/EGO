#pragma once

#include <cstddef>
#include <utility>

#include "EgoCore/RTTI/Type/TypeMetaInfo.h"

namespace ego::rtti
{
    template <typename Value, typename... PropertyArguments>
    const PropertyMetaInfo& AddPropertyMetaInfo(
        PropertyMetaInfoCollection& _propertyMetaInfos,
        const char* _name,
        size_t _offset,
        PropertyArguments&&... _propertyArguments);
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/PropertyMetaInfoFactory.hpp"
