#pragma once

namespace ego
{
    template <typename T>
    ComputeMatrix4x4Base<T> CrossProductSkewSymmetricComputeMatrix4x4(const ComputeVector4Base<T>& _vector)
    {
        const ComputeVector4Base<T> column0(T(0.0), _vector[2], -_vector[1], T(0.0));
        const ComputeVector4Base<T> column1(-_vector[2], T(0.0), _vector[0], T(0.0));
        const ComputeVector4Base<T> column2(_vector[1], -_vector[0], T(0.0), T(0.0));
        const ComputeVector4Base<T> column3(T(0.0), T(0.0), T(0.0), T(1.0));

        return ComputeMatrix4x4Base<T>(column0, column1, column2, column3);
    }

    template <typename T>
    ComputeMatrix4x4Base<T> TranslationComputeMatrix4x4(const ComputeVector3Base<T>& _vector)
    {
        ComputeMatrix4x4Base<T> result = ComputeMatrix4x4IdentityBase<T>();
        result.setColumn(3, ComputeVector4Base<T>(_vector, T(1.0)));

        return result;
    }

    template <typename T>
    ComputeMatrix4x4Base<T> RotationEulerComputeMatrix4x4(const ComputeVector3Base<T>& _vector)
    {
        ComputeMatrix4x4Base<T> result = RotationEulerComputeMatrix3x3(_vector);
        result.setElement(3, 3, T(1.0));

        return result;
    }

    template <typename T>
    ComputeMatrix4x4Base<T> ScaleComputeMatrix4x4(const ComputeVector3Base<T>& _scale)
    {
        return ComputeMatrix4x4Base<T>(
            ComputeVector4Base<T>(_scale.getX(), T(0.0), T(0.0), T(0.0)),
            ComputeVector4Base<T>(T(0.0), _scale.getY(), T(0.0), T(0.0)),
            ComputeVector4Base<T>(T(0.0), T(0.0), _scale.getZ(), T(0.0)),
            ComputeVector4Base<T>(T(0.0), T(0.0), T(0.0), T(1.0)));
    }

    template <typename T>
    ComputeMatrix4x4Base<T> RotationEulerComputeMatrix4x4(T _roll, T _pitch, T _yaw)
    {
        return RotationEulerComputeMatrix4x4(ComputeVector3Base<T>(_roll, _pitch, _yaw));
    }

    template <typename T>
    ComputeMatrix4x4Base<T> InvertComputeMatrix4x4(const ComputeMatrix4x4Base<T>& _matrix)
    {
        ComputeMatrix4x4Base<T> result(_matrix);
        return result.invert();
    }
} // namespace ego
