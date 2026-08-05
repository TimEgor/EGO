#pragma once

#include "ComputeVector2.h"

namespace ego
{
    template <typename T>
    using ComputeVector3Base = ComputeVectorBase<T, 3>;

    template <typename T>
    T DotComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2);
    template <typename T>
    ComputeVector3Base<T> CrossComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2);

    template <typename T>
    ComputeVector3Base<T> NegateComputeVector3(const ComputeVector3Base<T>& _vector);
    template <typename T>
    ComputeVector3Base<T> ComputeReciprocalComputeVector3(const ComputeVector3Base<T>& _vector);
    template <typename T>
    ComputeVector3Base<T> NormalizeComputeVector3(const ComputeVector3Base<T>& _vector);

    template <typename T>
    T LengthComputeVector3(const ComputeVector3Base<T>& _vector);
    template <typename T>
    T LengthSqrComputeVector3(const ComputeVector3Base<T>& _vector);

    template <typename T>
    ComputeVector3Base<T> MultiplyComputeVector3Elements(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2);

    template <typename T>
    ComputeVector3Base<T> MinComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2);
    template <typename T>
    ComputeVector3Base<T> MaxComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2);

    template <typename T>
    ComputeVector3Base<T> ComputeVector3ZeroBase()
    {
        return ComputeVector3Base<T>(T(0.0));
    }

    template <typename T>
    ComputeVector3Base<T> ComputeVector3OneBase()
    {
        return ComputeVector3Base<T>(T(1.0));
    }

    template <typename T>
    ComputeVector3Base<T> ComputeVector3NegativeOneBase()
    {
        return ComputeVector3Base<T>(T(-1.0));
    }

    template <typename T>
    ComputeVector3Base<T> ComputeVector3EpsilonBase()
    {
        return ComputeVector3Base<T>(math::TypedEpsilon<T>());
    }

    template <typename T>
    ComputeVector3Base<T> ComputeVector3UnitXBase()
    {
        return ComputeVector3Base<T>(T(1.0), T(0.0), T(0.0));
    }

    template <typename T>
    ComputeVector3Base<T> ComputeVector3UnitYBase()
    {
        return ComputeVector3Base<T>(T(0.0), T(1.0), T(0.0));
    }

    template <typename T>
    ComputeVector3Base<T> ComputeVector3UnitZBase()
    {
        return ComputeVector3Base<T>(T(0.0), T(0.0), T(1.0));
    }

    template <typename T>
    ComputeVector3Base<T> ComputeVector3NegativeUnitXBase()
    {
        return ComputeVector3Base<T>(T(-1.0), T(0.0), T(0.0));
    }

    template <typename T>
    ComputeVector3Base<T> ComputeVector3NegativeUnitYBase()
    {
        return ComputeVector3Base<T>(T(0.0), T(-1.0), T(0.0));
    }

    template <typename T>
    ComputeVector3Base<T> ComputeVector3NegativeUnitZBase()
    {
        return ComputeVector3Base<T>(T(0.0), T(0.0), T(-1.0));
    }

    using ComputeVector3 = ComputeVector3Base<ComputeValue>;
    using FloatComputeVector3 = ComputeVector3Base<float>;
} // namespace ego

#include "ComputeVector3.hpp"
