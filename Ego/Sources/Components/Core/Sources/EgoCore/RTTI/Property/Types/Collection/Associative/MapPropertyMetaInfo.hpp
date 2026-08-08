#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Associative/MapPropertyValue.h"

namespace ego::rtti
{
    template <typename Map>
    MapPropertyMetaInfo<Map>::MapPropertyMetaInfo(
        const char* _name,
        size_t _offset,
        bool _isConst,
        const PropertyMetaInfo& _keyMetaInfo,
        const PropertyMetaInfo& _mappedMetaInfo)
        : AssociativeCollectionPropertyMetaInfo(_name, _offset, _isConst, _keyMetaInfo, _mappedMetaInfo)
    {
    }

    template <typename Map>
    PropertyValuePointer MapPropertyMetaInfo<Map>::makePropertyValue(void* _object) const
    {
        return MakeIntrusive<MapPropertyValue<Map>>(_object, *this);
    }

    template <typename Map>
    PropertyValuePointer MapPropertyMetaInfo<Map>::makePropertyValue(const void* _object) const
    {
        return MakeIntrusive<MapPropertyValue<Map>>(_object, *this);
    }
} // namespace ego::rtti
