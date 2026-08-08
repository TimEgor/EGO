#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Associative/AssociativeCollectionPropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename Map>
    class MapPropertyValue;

    template <typename Map>
    class MapPropertyMetaInfo final : public AssociativeCollectionPropertyMetaInfo
    {
    public:
        MapPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, const PropertyMetaInfo& _keyMetaInfo, const PropertyMetaInfo& _mappedMetaInfo);

        EGO_RTTI_VIRTUAL(MapPropertyMetaInfo, AssociativeCollectionPropertyMetaInfo);

        PropertyValuePointer makePropertyValue(void* _object) const override;
        PropertyValuePointer makePropertyValue(const void* _object) const override;
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Collection/Associative/MapPropertyMetaInfo.hpp"
