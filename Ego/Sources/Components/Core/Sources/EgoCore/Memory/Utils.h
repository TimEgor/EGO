#pragma once

namespace ego
{
    template <typename T>
    inline T GetAdjustment(T _size, T _alignment)
    {
        return _alignment - (_size & (_alignment - 1));
    }

    template <typename T>
    inline T Align(T _size, T _alignment)
    {
        const T adjustment = GetAdjustment(_size, _alignment);
        if (adjustment != _alignment)
        {
            return _size + adjustment;
        }

        return _size;
    }
}
