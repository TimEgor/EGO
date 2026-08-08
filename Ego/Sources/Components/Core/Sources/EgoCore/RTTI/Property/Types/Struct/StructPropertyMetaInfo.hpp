#pragma once

#include "EgoCore/RTTI/Property/Types/Struct/StructPropertyValue.h"

namespace ego::rtti
{
    template <typename Struct>
    TypedStructPropertyMetaInfo<Struct>::TypedStructPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, const TypeMetaInfo& _valueTypeMetaInfo)
        : StructPropertyMetaInfo(_name, _offset, _isConst, _valueTypeMetaInfo)
    {
    }

    template <typename Struct>
    PropertyValuePointer TypedStructPropertyMetaInfo<Struct>::makePropertyValue(void* _object) const
    {
        return MakeIntrusive<StructPropertyValue<Struct>>(_object, *this);
    }

    template <typename Struct>
    PropertyValuePointer TypedStructPropertyMetaInfo<Struct>::makePropertyValue(const void* _object) const
    {
        return MakeIntrusive<StructPropertyValue<Struct>>(_object, *this);
    }
} // namespace ego::rtti
