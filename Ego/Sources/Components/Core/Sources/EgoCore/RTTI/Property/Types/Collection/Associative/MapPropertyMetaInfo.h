#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Associative/AssociativeCollectionPropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename Map>
    class MapPropertyMetaInfo final : public AssociativeCollectionPropertyMetaInfo
    {
    public:
        MapPropertyMetaInfo(const char* _name, size_t _offset, const PropertyMetaInfo* _keyMetaInfo, const PropertyMetaInfo* _mappedMetaInfo);

        size_t getSize(const void* _value) const override;
        void visitElements(void* _value, const ElementVisitor& _visitor) const override;
        bool canAddElement() const override;
        bool addElement(void* _value) const override;

        EGO_RTTI_VIRTUAL(MapPropertyMetaInfo, AssociativeCollectionPropertyMetaInfo);
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Collection/Associative/MapPropertyMetaInfo.hpp"
