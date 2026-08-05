#pragma once

namespace ego::rtti
{
    template <typename Pointer>
    ObjectPropertyMetaInfo<Pointer>::ObjectPropertyMetaInfo(const char* _name, size_t _offset)
        : PropertyMetaInfo(_name, _offset)
    {
    }
} // namespace ego::rtti
