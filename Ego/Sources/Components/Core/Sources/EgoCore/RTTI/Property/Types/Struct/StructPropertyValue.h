#pragma once

#include "EgoCore/RTTI/Property/PropertyValue.h"
#include "EgoCore/RTTI/Property/Types/Struct/StructPropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename Struct>
    class StructPropertyValue final : public PropertyValue
    {
    public:
        StructPropertyValue(void* _object, const TypedStructPropertyMetaInfo<Struct>& _propertyMetaInfo);
        StructPropertyValue(const void* _object, const TypedStructPropertyMetaInfo<Struct>& _propertyMetaInfo);

        const TypedStructPropertyMetaInfo<Struct>& getMetaInfo() const override;
        Struct& getStruct();
        const Struct& getStruct() const;

        EGO_RTTI_VIRTUAL(StructPropertyValue, PropertyValue);
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Struct/StructPropertyValue.hpp"
