#pragma once

#include <cstddef>
#include <functional>

#include "Pointer.h"

namespace ego
{
    struct PointerIdentityHash final
    {
        template <typename T>
        size_t operator()(const SharedPointer<T>& _pointer) const noexcept
        {
            return std::hash<const void*>()(static_cast<const void*>(_pointer.get()));
        }
    };

    struct PointerIdentityEqual final
    {
        template <typename T>
        bool operator()(const SharedPointer<T>& _left, const SharedPointer<T>& _right) const noexcept
        {
            return static_cast<const void*>(_left.get()) == static_cast<const void*>(_right.get());
        }
    };
} // namespace ego
