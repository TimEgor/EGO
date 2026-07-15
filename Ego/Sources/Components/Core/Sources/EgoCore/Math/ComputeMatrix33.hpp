#pragma once

namespace ego
{
    template <typename T>
    ComputeMatrix3x3Base<T> CrossProductSkewSymmetricComputeMatrix3x3(const ComputeVector3Base<T>& _vector)
    {
        return ComputeMatrix3x3Base<T>(T(0.0), _vector[2], -_vector[1], -_vector[2], T(0.0), _vector[0], _vector[1], -_vector[0], T(0.0));
    }

    template <typename T>
    ComputeMatrix3x3Base<T> RotationEulerComputeMatrix3x3(const ComputeVector3Base<T>& _vector)
    {
        const T cx = std::cos(_vector.getX());
        const T sx = std::sin(_vector.getX());

        const T cy = std::cos(_vector.getY());
        const T sy = std::sin(_vector.getY());

        const T cz = std::cos(_vector.getZ());
        const T sz = std::sin(_vector.getZ());

        const ComputeVector3Base<T> column0(cz * cy + sz * sx * sy, sz * cx, sz * sx * cy - cz * sy);
        const ComputeVector3Base<T> column1(cz * sx * sy - sz * cy, cz * cx, sz * sy + cz * sx * cy);
        const ComputeVector3Base<T> column2(cx * sy, -sx, cx * cy);

        return ComputeMatrix3x3Base<T>(column0, column1, column2);
    }

    template <typename T>
    ComputeMatrix3x3Base<T> RotationEulerComputeMatrix3x3(T _roll, T _pitch, T _yaw)
    {
        return RotationEulerComputeMatrix3x3(ComputeVector3Base<T>(_roll, _pitch, _yaw));
    }

    template <typename T>
    ComputeVector3Base<T> EulerAnglesFromRotationComputeMatrix3x3(const ComputeMatrix3x3Base<T>& _rotation)
    {
        if (std::abs(_rotation.getElement(2, 0)) < T(1.0))
        {
            return ComputeVector3Base<T>(
                std::atan2(_rotation.getElement(2, 1), _rotation.getElement(2, 2)),
                -std::asin(_rotation.getElement(2, 0)),
                std::atan2(_rotation.getElement(1, 0), _rotation.getElement(0, 0)));
        }

        return ComputeVector3Base<T>(
            std::atan2(_rotation.getElement(0, 1), _rotation.getElement(1, 1)),
            (_rotation.getElement(2, 0) <= T(-1.0)) ? math::TypedHalfPi<T>() : -math::TypedHalfPi<T>(),
            T(0.0));
    }

    template <typename T>
    ComputeMatrix3x3Base<T> ScaleComputeMatrix3x3(const ComputeVector3Base<T>& _scale)
    {
        return ComputeMatrix3x3Base<T>(
            ComputeVector3Base<T>(_scale.getX(), T(0.0), T(0.0)),
            ComputeVector3Base<T>(T(0.0), _scale.getY(), T(0.0)),
            ComputeVector3Base<T>(T(0.0), T(0.0), _scale.getZ()));
    }

    template <typename T>
    ComputeMatrix3x3Base<T> InvertComputeMatrix3x3(const ComputeMatrix3x3Base<T>& _matrix)
    {
        ComputeMatrix3x3Base<T> result(_matrix);
        return result.invert();
    }

    template <typename T>
    ComputeMatrix3x3Base<T> TransposeComputeMatrix3x3(const ComputeMatrix3x3Base<T>& _matrix)
    {
        ComputeMatrix3x3Base<T> result(_matrix);
        return result.transpose();
    }
} // namespace ego
