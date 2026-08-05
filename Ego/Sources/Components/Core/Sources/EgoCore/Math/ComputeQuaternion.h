#pragma once

#include "ComputeMatrix44.h"
#include "ComputeVector4.h"
#include "Quaternion.h"

namespace ego
{
    template <typename T>
    class ComputeQuaternionBase final
    {
    public:
        using ValueType = T;
        static_assert(std::is_floating_point_v<ValueType>);

        using QuaternionType = QuaternionBase<ValueType>;

    private:
        ComputeVector4Base<ValueType> m_vector;

    public:
        ComputeQuaternionBase()
            : m_vector()
        {
        }
        ComputeQuaternionBase(ValueType _x, ValueType _y, ValueType _z, ValueType _w)
            : m_vector(_x, _y, _z, _w)
        {
        }
        ComputeQuaternionBase(const QuaternionType& _quaternion)
            : m_vector(_quaternion.m_elements)
        {
        }
        ComputeQuaternionBase(const ComputeVector4Base<ValueType>& _vector)
            : m_vector(_vector)
        {
        }
        ComputeQuaternionBase(const ComputeQuaternionBase& _quaternion) = default;

        explicit ComputeQuaternionBase(const ComputeMatrix3x3Base<ValueType>& _matrix);
        explicit ComputeQuaternionBase(const ComputeMatrix4x4Base<ValueType>& _matrix);
        explicit ComputeQuaternionBase(const ComputeVector3Base<ValueType>& _axis, ValueType _angle);

        ComputeQuaternionBase& operator=(const ComputeQuaternionBase& _quaternion) = default;

        bool operator==(const ComputeQuaternionBase& _quaternion) const;
        bool operator!=(const ComputeQuaternionBase& _quaternion) const;

        ComputeQuaternionBase& operator*=(const ComputeQuaternionBase& _quaternion);

        ValueType operator[](uint32_t _index) const
        {
            return getElement(_index);
        }

        ValueType getElement(uint32_t _index) const
        {
            return m_vector.getElement(_index);
        }
        void setElement(uint32_t _index, ValueType _value)
        {
            m_vector.setElement(_index, _value);
        }

        ComputeQuaternionBase& setupFromRotationMatrix3x3(const ComputeMatrix3x3Base<ValueType>& _matrix);
        ComputeQuaternionBase& setupFromRotationMatrix4x4(const ComputeMatrix4x4Base<ValueType>& _matrix);
        ComputeQuaternionBase& setupFromAxisAngle(const ComputeVector3Base<ValueType>& _axis, ValueType _angle);
        ComputeQuaternionBase& setupFromEulerAngles(const ComputeVector3Base<ValueType>& _eulerAngles);
        ComputeQuaternionBase& setupFromRollPitchYaw(ValueType _roll, ValueType _pitch, ValueType _yaw);

        ValueType getX() const
        {
            return m_vector.getX();
        }
        ValueType getY() const
        {
            return m_vector.getY();
        }
        ValueType getZ() const
        {
            return m_vector.getZ();
        }
        ValueType getW() const
        {
            return m_vector.getW();
        }
        void setX(ValueType _value)
        {
            m_vector.setX(_value);
        }
        void setY(ValueType _value)
        {
            m_vector.setY(_value);
        }
        void setZ(ValueType _value)
        {
            m_vector.setZ(_value);
        }
        void setW(ValueType _value)
        {
            m_vector.setW(_value);
        }

        const ComputeVector4Base<ValueType>& getVector() const
        {
            return m_vector;
        }
        ComputeVector4Base<ValueType>& getVector()
        {
            return m_vector;
        }
        ComputeVector3Base<ValueType> getXYZ() const
        {
            return m_vector.getXYZ();
        }
        void setXYZ(const ComputeVector3Base<ValueType>& _vector)
        {
            m_vector.setXYZ(_vector);
        }

        ValueType getLength() const;
        ValueType getLengthSqr() const;

        ValueType dot(const ComputeQuaternionBase& _quaternion) const;

        ComputeQuaternionBase& normalize();
        ComputeQuaternionBase& conjugate();
        ComputeQuaternionBase& invert();

        ComputeVector3Base<ValueType> rotate(const ComputeVector3Base<ValueType>& _vector) const;
        void rotate(const ComputeVector3Base<ValueType>& _vector, ComputeVector3Base<ValueType>& _out) const;

        bool isEqual(const ComputeQuaternionBase& _quaternion, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
        bool isUnit() const;
        bool isZero() const;

        ComputeMatrix3x3Base<ValueType> getRotationMatrix3x3() const;
        void getRotationMatrix3x3(ComputeMatrix3x3Base<ValueType>& _matrix) const;
        ComputeMatrix4x4Base<ValueType> getRotationMatrix4x4() const;
        void getRotationMatrix4x4(ComputeMatrix4x4Base<ValueType>& _matrix) const;

        ValueType getAxisAngle(const ComputeVector3Base<ValueType>& _axis) const;

        ComputeVector3Base<ValueType> getEulerAngles() const;
        void getEulerAngles(ComputeVector3Base<ValueType>& _angles) const;
    };

    template <typename T>
    ComputeQuaternionBase<T> operator*(const ComputeQuaternionBase<T>& _quaternion1, const ComputeQuaternionBase<T>& _quaternion2);

    template <typename T>
    ComputeQuaternionBase<T> ConjugateComputeQuaternion(const ComputeQuaternionBase<T>& _quaternion);

    template <typename T>
    ComputeQuaternionBase<T> ComputeQuaternionZeroBase()
    {
        return ComputeQuaternionBase<T>(T(0.0), T(0.0), T(0.0), T(0.0));
    }

    template <typename T>
    ComputeQuaternionBase<T> ComputeQuaternionIdentityBase()
    {
        return ComputeQuaternionBase<T>(T(0.0), T(0.0), T(0.0), T(1.0));
    }

    template <typename T>
    ComputeVector3Base<T> EulerAnglesFromQuaternion(const ComputeQuaternionBase<T>& _rotation)
    {
        return _rotation.getEulerAngles();
    }

    using ComputeQuaternion = ComputeQuaternionBase<ComputeValue>;
    using FloatComputeQuaternion = ComputeQuaternionBase<float>;

    inline const ComputeQuaternion ComputeQuaternionZero = ComputeQuaternionZeroBase<ComputeValue>();
    inline const ComputeQuaternion ComputeQuaternionIdentity = ComputeQuaternionIdentityBase<ComputeValue>();
} // namespace ego

#include "ComputeQuaternion.hpp"
