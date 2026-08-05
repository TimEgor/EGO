#pragma once

namespace ego
{
    template <typename T>
    ComputeMatrix3x3Base<T> CrossProductSkewSymmetricComputeMatrix3x3(const ComputeVector3Base<T>& _vector)
    {
        const ComputeVector3Base<T> column0(T(0.0), _vector[2], -_vector[1]);
        const ComputeVector3Base<T> column1(-_vector[2], T(0.0), _vector[0]);
        const ComputeVector3Base<T> column2(_vector[1], -_vector[0], T(0.0));

        return ComputeMatrix3x3Base<T>(column0, column1, column2);
    }

    template <typename T>
    ComputeMatrix3x3Base<T> RotationEulerComputeMatrix3x3(const ComputeVector3Base<T>& _vector)
    {
        const T cr = std::cos(_vector.getX());
        const T sr = std::sin(_vector.getX());

        const T cp = std::cos(_vector.getY());
        const T sp = std::sin(_vector.getY());

        const T cy = std::cos(_vector.getZ());
        const T sy = std::sin(_vector.getZ());

        const ComputeVector3Base<T> column0(cy * cp, sy * cp, -sp);
        const ComputeVector3Base<T> column1(cy * sp * sr - sy * cr, sy * sp * sr + cy * cr, cp * sr);
        const ComputeVector3Base<T> column2(cy * sp * cr + sy * sr, sy * sp * cr - cy * sr, cp * cr);

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
        const T sinPitch = math::Clamp(-_rotation.getElement(2, 0), T(-1.0), T(1.0));
        const T pitch = std::asin(sinPitch);

        if (std::abs(std::cos(pitch)) > math::TypedEpsilon<T>())
        {
            return ComputeVector3Base<T>(
                std::atan2(_rotation.getElement(2, 1), _rotation.getElement(2, 2)),
                pitch,
                std::atan2(_rotation.getElement(1, 0), _rotation.getElement(0, 0)));
        }

        const T roll = sinPitch > T(0.0) ? std::atan2(_rotation.getElement(0, 1), _rotation.getElement(1, 1)) :
                                           std::atan2(-_rotation.getElement(0, 1), _rotation.getElement(1, 1));

        return ComputeVector3Base<T>(roll, pitch, T(0.0));
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
