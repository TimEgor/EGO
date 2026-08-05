#pragma once

namespace ego::rtti
{
    template <typename T>
        requires std::is_arithmetic_v<T>
    ScalarPropertyMetaInfo<T>::ScalarPropertyMetaInfo(const char* _name, size_t _offset)
        : PropertyMetaInfo(_name, _offset)
    {
    }
} // namespace ego::rtti
