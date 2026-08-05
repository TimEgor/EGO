#pragma once

#include "ComputeVector.h"

namespace ego
{
    template <typename T>
    using ComputeVector2Base = ComputeVectorBase<T, 2>;

    template <typename T>
    T DotComputeVector2(const ComputeVector2Base<T>& _vector1, const ComputeVector2Base<T>& _vector2);

    template <typename T>
    ComputeVector2Base<T> ComputeVector2ZeroBase()
    {
        return ComputeVector2Base<T>(T(0.0));
    }

    template <typename T>
    ComputeVector2Base<T> ComputeVector2OneBase()
    {
        return ComputeVector2Base<T>(T(1.0));
    }

    template <typename T>
    ComputeVector2Base<T> ComputeVector2NegativeOneBase()
    {
        return ComputeVector2Base<T>(T(-1.0));
    }

    template <typename T>
    ComputeVector2Base<T> ComputeVector2EpsilonBase()
    {
        return ComputeVector2Base<T>(math::TypedEpsilon<T>());
    }

    using ComputeVector2 = ComputeVector2Base<ComputeValue>;
    using FloatComputeVector2 = ComputeVector2Base<float>;
} // namespace ego

#include "ComputeVector2.hpp"
