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

    using ComputeVector3 = ComputeVector3Base<ComputeValueType>;
    using FloatComputeVector3 = ComputeVector3Base<float>;

    inline const ComputeVector3 ComputeVector3Zero = ComputeVector3ZeroBase<ComputeValueType>();
    inline const ComputeVector3 ComputeVector3One = ComputeVector3OneBase<ComputeValueType>();
    inline const ComputeVector3 ComputeVector3NegativeOne = ComputeVector3NegativeOneBase<ComputeValueType>();

    inline const ComputeVector3 ComputeVector3Epsilon = ComputeVector3EpsilonBase<ComputeValueType>();

    inline const ComputeVector3 ComputeVector3UnitX = ComputeVector3UnitXBase<ComputeValueType>();
    inline const ComputeVector3 ComputeVector3UnitY = ComputeVector3UnitYBase<ComputeValueType>();
    inline const ComputeVector3 ComputeVector3UnitZ = ComputeVector3UnitZBase<ComputeValueType>();
    inline const ComputeVector3 ComputeVector3NegativeUnitX = ComputeVector3NegativeUnitXBase<ComputeValueType>();
    inline const ComputeVector3 ComputeVector3NegativeUnitY = ComputeVector3NegativeUnitYBase<ComputeValueType>();
    inline const ComputeVector3 ComputeVector3NegativeUnitZ = ComputeVector3NegativeUnitZBase<ComputeValueType>();

    inline const FloatComputeVector3 FloatComputeVector3Zero = ComputeVector3ZeroBase<float>();
    inline const FloatComputeVector3 FloatComputeVector3One = ComputeVector3OneBase<float>();
    inline const FloatComputeVector3 FloatComputeVector3NegativeOne = ComputeVector3NegativeOneBase<float>();

    inline const FloatComputeVector3 FloatComputeVector3Epsilon = ComputeVector3EpsilonBase<float>();

    inline const FloatComputeVector3 FloatComputeVector3UnitX = ComputeVector3UnitXBase<float>();
    inline const FloatComputeVector3 FloatComputeVector3UnitY = ComputeVector3UnitYBase<float>();
    inline const FloatComputeVector3 FloatComputeVector3UnitZ = ComputeVector3UnitZBase<float>();
    inline const FloatComputeVector3 FloatComputeVector3NegativeUnitX = ComputeVector3NegativeUnitXBase<float>();
    inline const FloatComputeVector3 FloatComputeVector3NegativeUnitY = ComputeVector3NegativeUnitYBase<float>();
    inline const FloatComputeVector3 FloatComputeVector3NegativeUnitZ = ComputeVector3NegativeUnitZBase<float>();
} // namespace ego

#include "ComputeVector3.hpp"
