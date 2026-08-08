#pragma once

#include "EgoCore/RTTI/Property/PropertyValue.h"
#include "EgoCore/RTTI/Property/Types/Collection/CollectionPropertyMetaInfo.h"

namespace ego::rtti
{
    class CollectionPropertyValue : public PropertyValue
    {
    public:
        CollectionPropertyValue(void* _object, const CollectionPropertyMetaInfo& _propertyMetaInfo);
        CollectionPropertyValue(const void* _object, const CollectionPropertyMetaInfo& _propertyMetaInfo);

        const CollectionPropertyMetaInfo& getMetaInfo() const override;
        virtual size_t getSize() const = 0;

        EGO_RTTI_VIRTUAL(CollectionPropertyValue, PropertyValue);
    };
} // namespace ego::rtti
