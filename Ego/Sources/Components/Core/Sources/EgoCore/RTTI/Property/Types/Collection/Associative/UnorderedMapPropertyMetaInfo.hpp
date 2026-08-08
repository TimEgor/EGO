#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Associative/UnorderedMapPropertyValue.h"

namespace ego::rtti
{
    template <typename UnorderedMap>
    UnorderedMapPropertyMetaInfo<UnorderedMap>::UnorderedMapPropertyMetaInfo(
        const char* _name,
        size_t _offset,
        bool _isConst,
        const PropertyMetaInfo& _keyMetaInfo,
        const PropertyMetaInfo& _mappedMetaInfo)
        : AssociativeCollectionPropertyMetaInfo(_name, _offset, _isConst, _keyMetaInfo, _mappedMetaInfo)
    {
    }

    template <typename UnorderedMap>
    PropertyValuePointer UnorderedMapPropertyMetaInfo<UnorderedMap>::makePropertyValue(void* _object) const
    {
        return MakeIntrusive<UnorderedMapPropertyValue<UnorderedMap>>(_object, *this);
    }

    template <typename UnorderedMap>
    PropertyValuePointer UnorderedMapPropertyMetaInfo<UnorderedMap>::makePropertyValue(const void* _object) const
    {
        return MakeIntrusive<UnorderedMapPropertyValue<UnorderedMap>>(_object, *this);
    }
} // namespace ego::rtti
