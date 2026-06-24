#pragma once

namespace ego
{
    template <typename T>
    ComputeQuaternionBase<T>::ComputeQuaternionBase(const ComputeMatrix3x3Base<T>& _matrix)
    {
        setupFromRotationMatrix3x3(_matrix);
    }

    template <typename T>
    ComputeQuaternionBase<T>::ComputeQuaternionBase(const ComputeMatrix4x4Base<T>& _matrix)
    {
        setupFromRotationMatrix4x4(_matrix);
    }

    template <typename T>
    ComputeQuaternionBase<T>::ComputeQuaternionBase(const ComputeVector3Base<T>& _axis, T _angle)
    {
        setupFromAxisAngle(_axis, _angle);
    }

    template <typename T>
    ComputeQuaternionBase<T>& ComputeQuaternionBase<T>::operator=(const ComputeQuaternionBase<T>& _quaternion)
    {
        m_quaternion = _quaternion.m_quaternion;
        return *this;
    }

    template <typename T>
    bool ComputeQuaternionBase<T>::operator==(const ComputeQuaternionBase<T>& _quaternion) const
    {
        return isEqual(_quaternion);
    }

    template <typename T>
    bool ComputeQuaternionBase<T>::operator!=(const ComputeQuaternionBase<T>& _quaternion) const
    {
        return !isEqual(_quaternion);
    }

    template <typename T>
    ComputeQuaternionBase<T>& ComputeQuaternionBase<T>::operator*=(const ComputeQuaternionBase<T>& _quaternion)
    {
        const T x = m_quaternion.m_elements.m_w * _quaternion.m_quaternion.m_elements.m_x + m_quaternion.m_elements.m_x * _quaternion.m_quaternion.m_elements.m_w +
                    m_quaternion.m_elements.m_y * _quaternion.m_quaternion.m_elements.m_z - m_quaternion.m_elements.m_z * _quaternion.m_quaternion.m_elements.m_y;
        const T y = m_quaternion.m_elements.m_w * _quaternion.m_quaternion.m_elements.m_y - m_quaternion.m_elements.m_x * _quaternion.m_quaternion.m_elements.m_z +
                    m_quaternion.m_elements.m_y * _quaternion.m_quaternion.m_elements.m_w + m_quaternion.m_elements.m_z * _quaternion.m_quaternion.m_elements.m_x;
        const T z = m_quaternion.m_elements.m_w * _quaternion.m_quaternion.m_elements.m_z + m_quaternion.m_elements.m_x * _quaternion.m_quaternion.m_elements.m_y -
                    m_quaternion.m_elements.m_y * _quaternion.m_quaternion.m_elements.m_x + m_quaternion.m_elements.m_z * _quaternion.m_quaternion.m_elements.m_w;
        const T w = m_quaternion.m_elements.m_w * _quaternion.m_quaternion.m_elements.m_w - m_quaternion.m_elements.m_x * _quaternion.m_quaternion.m_elements.m_x -
                    m_quaternion.m_elements.m_y * _quaternion.m_quaternion.m_elements.m_y - m_quaternion.m_elements.m_z * _quaternion.m_quaternion.m_elements.m_z;

        *this = ComputeQuaternionBase<T>(x, y, z, w);
        return *this;
    }

    template <typename T>
    ComputeQuaternionBase<T>& ComputeQuaternionBase<T>::setupFromRotationMatrix3x3(const ComputeMatrix3x3Base<T>& _matrix)
    {
        const T trace = _matrix.getElement(0, 0) + _matrix.getElement(1, 1) + _matrix.getElement(2, 2);

        if (trace > 0.0)
        {
            T s = sqrt(trace + T(1.0)) * T(2.0);

            m_quaternion.m_elements.m_w = T(0.25) * s;
            m_quaternion.m_elements.m_x = (_matrix.getElement(2, 1) - _matrix.getElement(1, 2)) / s;
            m_quaternion.m_elements.m_y = (_matrix.getElement(0, 2) - _matrix.getElement(2, 0)) / s;
            m_quaternion.m_elements.m_z = (_matrix.getElement(1, 0) - _matrix.getElement(0, 1)) / s;
        }
        else if (_matrix.getElement(0, 0) > _matrix.getElement(1, 1) && _matrix.getElement(0, 0) > _matrix.getElement(2, 2))
        {
            T s = sqrt(T(1.0) + _matrix.getElement(0, 0) - _matrix.getElement(1, 1) - _matrix.getElement(2, 2)) * T(2.0);

            m_quaternion.m_elements.m_w = (_matrix.getElement(2, 1) - _matrix.getElement(1, 2)) / s;
            m_quaternion.m_elements.m_x = T(0.25) * s;
            m_quaternion.m_elements.m_y = (_matrix.getElement(0, 1) + _matrix.getElement(1, 0)) / s;
            m_quaternion.m_elements.m_z = (_matrix.getElement(0, 2) + _matrix.getElement(2, 0)) / s;
        }
        else if (_matrix.getElement(1, 1) > _matrix.getElement(2, 2))
        {
            T s = sqrt(T(1.0) + _matrix.getElement(1, 1) - _matrix.getElement(0, 0) - _matrix.getElement(2, 2)) * T(2.0);

            m_quaternion.m_elements.m_w = (_matrix.getElement(0, 2) - _matrix.getElement(2, 0)) / s;
            m_quaternion.m_elements.m_x = (_matrix.getElement(0, 1) + _matrix.getElement(1, 0)) / s;
            m_quaternion.m_elements.m_y = T(0.25) * s;
            m_quaternion.m_elements.m_z = (_matrix.getElement(1, 2) + _matrix.getElement(2, 1)) / s;
        }
        else
        {
            T s = sqrt(T(1.0) + _matrix.getElement(2, 2) - _matrix.getElement(0, 0) - _matrix.getElement(1, 1)) * T(2.0);

            m_quaternion.m_elements.m_w = (_matrix.getElement(1, 0) - _matrix.getElement(0, 1)) / s;
            m_quaternion.m_elements.m_x = (_matrix.getElement(0, 2) + _matrix.getElement(2, 0)) / s;
            m_quaternion.m_elements.m_y = (_matrix.getElement(1, 2) + _matrix.getElement(2, 1)) / s;
            m_quaternion.m_elements.m_z = T(0.25) * s;
        }

        m_quaternion.m_elements.m_x = -m_quaternion.m_elements.m_x;
        m_quaternion.m_elements.m_y = -m_quaternion.m_elements.m_y;
        m_quaternion.m_elements.m_z = -m_quaternion.m_elements.m_z;

        return *this;
    }

    template <typename T>
    ComputeQuaternionBase<T>& ComputeQuaternionBase<T>::setupFromRotationMatrix4x4(const ComputeMatrix4x4Base<T>& _matrix)
    {
        setupFromRotationMatrix3x3(ComputeMatrix3x3Base<T>(_matrix.getColumn(0).getXYZ(), _matrix.getColumn(1).getXYZ(), _matrix.getColumn(2).getXYZ()));

        return *this;
    }

    template <typename T>
    ComputeQuaternionBase<T>& ComputeQuaternionBase<T>::setupFromAxisAngle(const ComputeVector3Base<T>& _axis, T _angle)
    {
        const ComputeVector3 normAxis = NormalizeComputeVector3(_axis);

        const T halfAngle = _angle * T(0.5);
        const T halfAngleSin = sin(halfAngle);
        const T halfAngleCos = cos(halfAngle);

        m_quaternion.m_elements.m_x = normAxis.getX() * halfAngleSin;
        m_quaternion.m_elements.m_y = normAxis.getY() * halfAngleSin;
        m_quaternion.m_elements.m_z = normAxis.getZ() * halfAngleSin;
        m_quaternion.m_elements.m_w = halfAngleCos;

        return *this;
    }

    template <typename T>
    ComputeQuaternionBase<T>& ComputeQuaternionBase<T>::setupFromEulerAngles(const ComputeVector3Base<T>& _eulerAngles)
    {
        return setupFromRollPitchYaw(_eulerAngles.getZ(), _eulerAngles.getX(), _eulerAngles.getY());
    }

    template <typename T>
    ComputeQuaternionBase<T>& ComputeQuaternionBase<T>::setupFromRollPitchYaw(ValueType _roll, ValueType _pitch, ValueType _yaw)
    {
        const ValueType cr = std::cos(_roll * T(0.5));
        const ValueType sr = std::sin(_roll * T(0.5));
        const ValueType cp = std::cos(_pitch * T(0.5));
        const ValueType sp = std::sin(_pitch * T(0.5));
        const ValueType cy = std::cos(_yaw * T(0.5));
        const ValueType sy = std::sin(_yaw * T(0.5));

        m_quaternion.m_elements.m_x = sr * cp * cy - cr * sp * sy;
        m_quaternion.m_elements.m_y = cr * sp * cy + sr * cp * sy;
        m_quaternion.m_elements.m_z = cr * cp * sy - sr * sp * cy;
        m_quaternion.m_elements.m_w = cr * cp * cy + sr * sp * sy;

        return *this;
    }

    template <typename T>
    T ComputeQuaternionBase<T>::getLength() const
    {
        return std::sqrt(getLengthSqr());
    }

    template <typename T>
    T ComputeQuaternionBase<T>::getLengthSqr() const
    {
        return dot(*this);
    }

    template <typename T>
    T ComputeQuaternionBase<T>::dot(const ComputeQuaternionBase<T>& _quaternion) const
    {
        return m_quaternion.m_elements.m_x * _quaternion.m_quaternion.m_elements.m_x + m_quaternion.m_elements.m_y * _quaternion.m_quaternion.m_elements.m_y +
               m_quaternion.m_elements.m_z * _quaternion.m_quaternion.m_elements.m_z + m_quaternion.m_elements.m_w * _quaternion.m_quaternion.m_elements.m_w;
    }

    template <typename T>
    ComputeQuaternionBase<T>& ComputeQuaternionBase<T>::normalize()
    {
        const T length = getLength();
        if (length <= math::TypedEpsilon<T>())
        {
            return *this;
        }

        m_quaternion.m_elements.m_x /= length;
        m_quaternion.m_elements.m_y /= length;
        m_quaternion.m_elements.m_z /= length;
        m_quaternion.m_elements.m_w /= length;

        return *this;
    }

    template <typename T>
    ComputeQuaternionBase<T>& ComputeQuaternionBase<T>::conjugate()
    {
        m_quaternion.m_elements.m_x = -m_quaternion.m_elements.m_x;
        m_quaternion.m_elements.m_y = -m_quaternion.m_elements.m_y;
        m_quaternion.m_elements.m_z = -m_quaternion.m_elements.m_z;

        return *this;
    }

    template <typename T>
    ComputeQuaternionBase<T>& ComputeQuaternionBase<T>::invert()
    {
        return conjugate() / getLength();
    }

    template <typename T>
    ComputeVector3Base<T> ComputeQuaternionBase<T>::rotate(const ComputeVector3Base<T>& _vector) const
    {
        ComputeVector3Base<T> result;
        rotate(_vector, result);

        return result;
    }

    template <typename T>
    void ComputeQuaternionBase<T>::rotate(const ComputeVector3Base<T>& _vector, ComputeVector3Base<T>& _out) const
    {
        EGO_ASSERT(isUnit());

        const ComputeQuaternionBase<T> vQ(_vector.getX(), _vector.getY(), _vector.getZ(), T(0.0));
        const ComputeQuaternionBase<T> conjugatedQ = ConjugateComputeQuaternion(*this);

        _out = (*this * vQ * conjugatedQ).getXYZ();
    }

    template <typename T>
    bool ComputeQuaternionBase<T>::isEqual(const ComputeQuaternionBase<T>& _quaternion, T _epsilon) const
    {
        return math::IsApproxEqual(m_quaternion.m_elements.m_x, _quaternion.m_quaternion.m_elements.m_x, _epsilon) &&
               math::IsApproxEqual(m_quaternion.m_elements.m_y, _quaternion.m_quaternion.m_elements.m_y, _epsilon) &&
               math::IsApproxEqual(m_quaternion.m_elements.m_z, _quaternion.m_quaternion.m_elements.m_z, _epsilon) &&
               math::IsApproxEqual(m_quaternion.m_elements.m_w, _quaternion.m_quaternion.m_elements.m_w, _epsilon);
    }

    template <typename T>
    bool ComputeQuaternionBase<T>::isUnit() const
    {
        return math::IsApproxEqual(getLengthSqr(), T(1.0), T(0.00001));
    }

    template <typename T>
    bool ComputeQuaternionBase<T>::isZero() const
    {
        return math::IsApproxEqual(getLengthSqr(), T(0.0), T(0.00001));
    }

    template <typename T>
    ComputeMatrix3x3Base<T> ComputeQuaternionBase<T>::getRotationMatrix3x3() const
    {
        ComputeMatrix3x3Base<T> matrix;
        getRotationMatrix3x3(matrix);

        return matrix;
    }

    template <typename T>
    void ComputeQuaternionBase<T>::getRotationMatrix3x3(ComputeMatrix3x3Base<T>& _matrix) const
    {
        const T x = getX();
        const T y = getY();
        const T z = getZ();
        const T w = getW();

        const T ww = w * w;
        const T xx = x * x;
        const T yy = y * y;
        const T zz = z * z;
        const T wx = w * x;
        const T wy = w * y;
        const T wz = w * z;
        const T xy = x * y;
        const T xz = x * z;
        const T yz = y * z;

        _matrix = ComputeMatrix3x3Base<T>(
            T(1.0) - T(2.0) * (yy + zz),
            T(2.0) * (xy - wz),
            T(2.0) * (xz + wy),

            T(2.0) * (xy + wz),
            T(1.0) - T(2.0) * (xx + zz),
            T(2.0) * (yz - wx),

            T(2.0) * (xz - wy),
            T(2.0) * (yz + wx),
            T(1.0) - T(2.0) * (xx + yy));
    }

    template <typename T>
    ComputeMatrix4x4Base<T> ComputeQuaternionBase<T>::getRotationMatrix4x4() const
    {
        ComputeMatrix4x4Base<T> matrix;
        getRotationMatrix4x4(matrix);

        return matrix;
    }

    template <typename T>
    void ComputeQuaternionBase<T>::getRotationMatrix4x4(ComputeMatrix4x4Base<T>& _matrix) const
    {
        ComputeMatrix3x3Base<T> matrix3x3;
        getRotationMatrix3x3(matrix3x3);

        _matrix = matrix3x3;
        _matrix.setElement(3, 3, T(1.0));
    }

    template <typename T>
    T ComputeQuaternionBase<T>::getAxisAngle(const ComputeVector3Base<T>& _axis) const
    {
        return std::atan(getXYZ().dot(_axis) / getW()) * T(2.0);
    }

    template <typename T>
    ComputeVector3Base<T> ComputeQuaternionBase<T>::getEulerAngles() const
    {
        ComputeVector3Base<T> angles;
        getEulerAngles(angles);

        return angles;
    }

    template <typename T>
    void ComputeQuaternionBase<T>::getEulerAngles(ComputeVector3Base<T>& _angles) const
    {
        const T sinr_cosp = T(2.0) * (m_quaternion.m_elements.m_w * m_quaternion.m_elements.m_x + m_quaternion.m_elements.m_y * m_quaternion.m_elements.m_z);
        const T cosr_cosp = T(1.0) - T(2.0) * (m_quaternion.m_elements.m_x * m_quaternion.m_elements.m_x + m_quaternion.m_elements.m_y * m_quaternion.m_elements.m_y);
        const T sinp = T(2.0) * (m_quaternion.m_elements.m_w * m_quaternion.m_elements.m_y - m_quaternion.m_elements.m_z * m_quaternion.m_elements.m_x);
        const T siny_cosp = T(2.0) * (m_quaternion.m_elements.m_w * m_quaternion.m_elements.m_z + m_quaternion.m_elements.m_x * m_quaternion.m_elements.m_y);
        const T cosy_cosp = T(1.0) - T(2.0) * (m_quaternion.m_elements.m_y * m_quaternion.m_elements.m_y + m_quaternion.m_elements.m_z * m_quaternion.m_elements.m_z);

        if (std::abs(sinp) >= T(1.0))
        {
            _angles.setY(std::copysign(math::HalfPi, sinp));
        }
        else
        {
            _angles.setY(std::asin(sinp));
        }

        _angles.setZ(std::atan2(sinr_cosp, cosr_cosp));
        _angles.setX(std::atan2(siny_cosp, cosy_cosp));
    }

    template <typename T>
    ComputeQuaternionBase<T> operator*(const ComputeQuaternionBase<T>& _quaternion1, const ComputeQuaternionBase<T>& _quaternion2)
    {
        ComputeQuaternionBase<T> result(_quaternion1);
        return result *= _quaternion2;
    }

    template <typename T>
    ComputeQuaternionBase<T> ConjugateComputeQuaternion(const ComputeQuaternionBase<T>& _quaternion)
    {
        ComputeQuaternionBase<T> result(_quaternion);
        return result.conjugate();
    }
} // namespace ego
