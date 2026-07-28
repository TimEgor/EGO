#pragma once

#include <cstddef>
#include <functional>

#include "IntrusivePointer.h"

namespace ego
{
    struct IntrusivePointerIdentityHash final
    {
        template <typename T>
        size_t operator()(const IntrusivePointer<T>& _pointer) const noexcept
        {
            return std::hash<const void*>()(static_cast<const void*>(_pointer.get()));
        }
    };

    struct IntrusivePointerIdentityEqual final
    {
        template <typename T>
        bool operator()(const IntrusivePointer<T>& _left, const IntrusivePointer<T>& _right) const noexcept
        {
            return static_cast<const void*>(_left.get()) == static_cast<const void*>(_right.get());
        }
    };
} // namespace ego
