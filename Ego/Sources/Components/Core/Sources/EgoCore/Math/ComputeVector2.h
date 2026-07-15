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

    using ComputeVector2 = ComputeVector2Base<ComputeValueType>;
    using FloatComputeVector2 = ComputeVector2Base<float>;

    inline const ComputeVector2 ComputeVector2Zero = ComputeVector2ZeroBase<ComputeValueType>();
    inline const ComputeVector2 ComputeVector2One = ComputeVector2OneBase<ComputeValueType>();
    inline const ComputeVector2 ComputeVector2NegativeOne = ComputeVector2NegativeOneBase<ComputeValueType>();

    inline const ComputeVector2 ComputeVector2Epsilon = ComputeVector2EpsilonBase<ComputeValueType>();
} // namespace ego

#include "ComputeVector2.hpp"
