#pragma once

namespace ego::rtti
{
    template <typename Struct>
    StructPropertyMetaInfo<Struct>::StructPropertyMetaInfo(const char* _name, size_t _offset, const TypeMetaInfo* _valueTypeMetaInfo)
        : PropertyMetaInfo(_name, _offset, _valueTypeMetaInfo)
    {
    }
} // namespace ego::rtti
