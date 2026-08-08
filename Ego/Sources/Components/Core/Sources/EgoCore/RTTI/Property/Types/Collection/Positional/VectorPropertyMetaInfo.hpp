#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Positional/VectorPropertyValue.h"

namespace ego::rtti
{
    template <typename Vector>
    VectorPropertyMetaInfo<Vector>::VectorPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, const PropertyMetaInfo& _elementMetaInfo)
        : PositionalCollectionPropertyMetaInfo(_name, _offset, _isConst, _elementMetaInfo)
    {
    }

    template <typename Vector>
    PropertyValuePointer VectorPropertyMetaInfo<Vector>::makePropertyValue(void* _object) const
    {
        return MakeIntrusive<VectorPropertyValue<Vector>>(_object, *this);
    }

    template <typename Vector>
    PropertyValuePointer VectorPropertyMetaInfo<Vector>::makePropertyValue(const void* _object) const
    {
        return MakeIntrusive<VectorPropertyValue<Vector>>(_object, *this);
    }
} // namespace ego::rtti
