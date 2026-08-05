#pragma once

#include "ComputeVector3.h"

namespace ego
{
    template <typename T>
    using ComputeVector4Base = ComputeVectorBase<T, 4>;

    template <typename T>
    ComputeVector4Base<T> ComputeVector4FromPoint(const ComputeVector3Base<T>& _vector);

    template <typename T>
    T DotComputeVector4(const ComputeVector4Base<T>& _vector1, const ComputeVector4Base<T>& _vector2);

    template <typename T>
    ComputeVector4Base<T> ComputeVector4ZeroBase()
    {
        return ComputeVector4Base<T>(T(0.0));
    }

    template <typename T>
    ComputeVector4Base<T> ComputeVector4OneBase()
    {
        return ComputeVector4Base<T>(T(1.0));
    }

    template <typename T>
    ComputeVector4Base<T> ComputeVector4NegativeOneBase()
    {
        return ComputeVector4Base<T>(T(-1.0));
    }

    template <typename T>
    ComputeVector4Base<T> ComputeVector4EpsilonBase()
    {
        return ComputeVector4Base<T>(math::TypedEpsilon<T>());
    }

    using ComputeVector4 = ComputeVector4Base<ComputeValueType>;
    using FloatComputeVector4 = ComputeVector4Base<float>;
} // namespace ego

#include "ComputeVector4.hpp"
