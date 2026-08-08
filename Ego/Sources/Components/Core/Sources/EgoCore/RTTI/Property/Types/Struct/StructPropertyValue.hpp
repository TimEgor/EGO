#pragma once

#include "EgoCore/Assert/Assert.h"

namespace ego::rtti
{
    template <typename Struct>
    StructPropertyValue<Struct>::StructPropertyValue(void* _object, const TypedStructPropertyMetaInfo<Struct>& _propertyMetaInfo)
        : PropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename Struct>
    StructPropertyValue<Struct>::StructPropertyValue(const void* _object, const TypedStructPropertyMetaInfo<Struct>& _propertyMetaInfo)
        : PropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename Struct>
    const TypedStructPropertyMetaInfo<Struct>& StructPropertyValue<Struct>::getMetaInfo() const
    {
        return static_cast<const TypedStructPropertyMetaInfo<Struct>&>(PropertyValue::getMetaInfo());
    }

    template <typename Struct>
    Struct& StructPropertyValue<Struct>::getStruct()
    {
        Struct* value = PropertyValue::tryGetMutableValue<Struct>();
        EGO_ASSERT(value);

        return *value;
    }

    template <typename Struct>
    const Struct& StructPropertyValue<Struct>::getStruct() const
    {
        return PropertyValue::getValue<Struct>();
    }
} // namespace ego::rtti
