#pragma once

#include "ComputeMatrix33.h"
#include "ComputeVector4.h"

namespace ego
{
    template <typename T>
    using ComputeMatrix4x4Base = ComputeMatrixBase<T, 4>;

    template <typename T>
    ComputeMatrix4x4Base<T> CrossProductSkewSymmetricComputeMatrix4x4(const ComputeVector4Base<T>& _vector);
    template <typename T>
    ComputeMatrix4x4Base<T> TranslationComputeMatrix4x4(const ComputeVector3Base<T>& _vector);
    template <typename T>
    ComputeMatrix4x4Base<T> RotationEulerComputeMatrix4x4(const ComputeVector3Base<T>& _vector);
    template <typename T>
    ComputeMatrix4x4Base<T> RotationEulerComputeMatrix4x4(T _pitch, T _yaw, T _roll);
    template <typename T>
    ComputeMatrix4x4Base<T> ScaleComputeMatrix4x4(const ComputeVector3Base<T>& _scale);

    template <typename T>
    ComputeMatrix4x4Base<T> InvertComputeMatrix4x4(const ComputeMatrix4x4Base<T>& _matrix);

    template <typename T>
    ComputeMatrix4x4Base<T> ComputeMatrix4x4ZeroBase()
    {
        return ComputeMatrix4x4Base<T>(T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0));
    }

    template <typename T>
    ComputeMatrix4x4Base<T> ComputeMatrix4x4IdentityBase()
    {
        return ComputeMatrix4x4Base<T>(T(1.0), T(0.0), T(0.0), T(0.0), T(0.0), T(1.0), T(0.0), T(0.0), T(0.0), T(0.0), T(1.0), T(0.0), T(0.0), T(0.0), T(0.0), T(1.0));
    }

    template <typename T>
    ComputeMatrix4x4Base<T> ComputeMatrix4x4ZeroIdentityBase()
    {
        return ComputeMatrix4x4Base<T>(T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(0.0), T(1.0));
    }

    using ComputeMatrix4x4 = ComputeMatrix4x4Base<ComputeValueType>;
    using FloatComputeMatrix4x4 = ComputeMatrix4x4Base<float>;
} // namespace ego

#include "ComputeMatrix44.hpp"
