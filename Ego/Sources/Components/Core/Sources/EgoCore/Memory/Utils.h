#pragma once

namespace ego
{
    template <typename T>
    inline T GetAdjustment(T _size, T _alignment)
    {
        if (_alignment == T(0))
        {
            return T(0);
        }

        const T remainder = _size % _alignment;
        if (remainder == T(0))
        {
            return T(0);
        }

        return _alignment - remainder;
    }

    template <typename T>
    inline T Align(T _size, T _alignment)
    {
        return _size + GetAdjustment(_size, _alignment);
    }
} // namespace ego
