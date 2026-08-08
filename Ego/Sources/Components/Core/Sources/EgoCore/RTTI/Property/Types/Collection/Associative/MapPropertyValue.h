#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Associative/AssociativeCollectionPropertyValue.h"
#include "EgoCore/RTTI/Property/Types/Collection/Associative/MapPropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename Map>
    class MapPropertyValue final : public AssociativeCollectionPropertyValue
    {
    public:
        MapPropertyValue(void* _object, const MapPropertyMetaInfo<Map>& _propertyMetaInfo);
        MapPropertyValue(const void* _object, const MapPropertyMetaInfo<Map>& _propertyMetaInfo);

        const MapPropertyMetaInfo<Map>& getMetaInfo() const override;
        size_t getSize() const override;
        void visitElements(const ElementVisitor& _visitor) override;
        bool canAddElement() const override;
        bool addElement() override;

        EGO_RTTI_VIRTUAL(MapPropertyValue, AssociativeCollectionPropertyValue);
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Collection/Associative/MapPropertyValue.hpp"
