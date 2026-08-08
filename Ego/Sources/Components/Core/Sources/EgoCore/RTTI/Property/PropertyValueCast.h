#pragma once

#include "EgoCore/RTTI/Property/PropertyValue.h"

namespace ego::rtti
{
    template <typename PropertyValueType>
    PropertyValueType* CastPropertyValue(PropertyValue* _propertyValue);

    template <typename PropertyValueType>
    const PropertyValueType* CastPropertyValue(const PropertyValue* _propertyValue);
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/PropertyValueCast.hpp"
