#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Positional/ArrayPropertyValue.h"

namespace ego::rtti
{
    template <typename Array>
    ArrayPropertyMetaInfo<Array>::ArrayPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, const PropertyMetaInfo& _elementMetaInfo)
        : PositionalCollectionPropertyMetaInfo(_name, _offset, _isConst, _elementMetaInfo)
    {
    }

    template <typename Array>
    PropertyValuePointer ArrayPropertyMetaInfo<Array>::makePropertyValue(void* _object) const
    {
        return MakeIntrusive<ArrayPropertyValue<Array>>(_object, *this);
    }

    template <typename Array>
    PropertyValuePointer ArrayPropertyMetaInfo<Array>::makePropertyValue(const void* _object) const
    {
        return MakeIntrusive<ArrayPropertyValue<Array>>(_object, *this);
    }
} // namespace ego::rtti
