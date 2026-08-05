#pragma once

#include "ComputeMatrix22.h"
#include "ComputeVector3.h"

namespace ego
{
    template <typename T>
    using ComputeMatrix3x3Base = ComputeMatrixBase<T, 3>;

    template <typename T>
    ComputeMatrix3x3Base<T> CrossProductSkewSymmetricComputeMatrix3x3(const ComputeVector3Base<T>& _vector);
    template <typename T>
    ComputeMatrix3x3Base<T> RotationEulerComputeMatrix3x3(const ComputeVector3Base<T>& _vector);
    template <typename T>
    ComputeMatrix3x3Base<T> RotationEulerComputeMatrix3x3(T _roll, T _pitch, T _yaw);
    template <typename T>
    ComputeVector3Base<T> EulerAnglesFromRotationComputeMatrix3x3(const ComputeMatrix3x3Base<T>& _rotation);
    template <typename T>
    ComputeMatrix3x3Base<T> ScaleComputeMatrix3x3(const ComputeVector3Base<T>& _scale);

    template <typename T>
    ComputeMatrix3x3Base<T> InvertComputeMatrix3x3(const ComputeMatrix3x3Base<T>& _matrix);
    template <typename T>
    ComputeMatrix3x3Base<T> TransposeComputeMatrix3x3(const ComputeMatrix3x3Base<T>& _matrix);

    template <typename T>
    ComputeMatrix3x3Base<T> ComputeMatrix3x3ZeroBase()
    {
        return ComputeMatrix3x3Base<T>(T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0));
    }

    template <typename T>
    ComputeMatrix3x3Base<T> ComputeMatrix3x3IdentityBase()
    {
        return ComputeMatrix3x3Base<T>(T(1.0), T(0.0), T(0.0), T(0.0), T(1.0), T(0.0), T(0.0), T(0.0), T(1.0));
    }

    using ComputeMatrix3x3 = ComputeMatrix3x3Base<ComputeValueType>;
    using FloatComputeMatrix3x3 = ComputeMatrix3x3Base<float>;
} // namespace ego

#include "ComputeMatrix33.hpp"
