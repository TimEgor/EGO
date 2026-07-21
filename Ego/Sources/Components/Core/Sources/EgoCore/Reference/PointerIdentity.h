#pragma once

#include <concepts>
#include <cstddef>
#include <functional>

namespace ego
{
    namespace pointer_identity_details
    {
        template <typename TPointer>
        concept PointerLike = requires(const TPointer& _pointer) {
            { _pointer.get() } -> std::convertible_to<const void*>;
        };
    } // namespace pointer_identity_details

    struct PointerIdentityHash final
    {
        using is_transparent = void;

        template <pointer_identity_details::PointerLike TPointer>
        size_t operator()(const TPointer& _pointer) const noexcept
        {
            return std::hash<const void*>()(static_cast<const void*>(_pointer.get()));
        }
    };

    struct PointerIdentityEqual final
    {
        using is_transparent = void;

        template <pointer_identity_details::PointerLike TLeftPointer, pointer_identity_details::PointerLike TRightPointer>
        bool operator()(const TLeftPointer& _left, const TRightPointer& _right) const noexcept
        {
            return static_cast<const void*>(_left.get()) == static_cast<const void*>(_right.get());
        }
    };
} // namespace ego
