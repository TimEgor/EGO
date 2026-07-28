#pragma once

#include <cstdint>

#include "GraphicObject.h"

namespace ego::gpu
{
    class Fence : public GraphicObject
    {
    public:
        using FenceValue = uint64_t;

        Fence() = default;

        virtual FenceValue getCompletedValue() const = 0;
        virtual void waitValue(FenceValue _value) = 0;
    };

    EGO_INTRUSIVE_POINTER(Fence);
} // namespace ego::gpu
