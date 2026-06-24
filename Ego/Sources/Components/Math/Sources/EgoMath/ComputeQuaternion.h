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
        static_assert(std::is_arithmetic_v<ValueType>);

        using QuaternionType = QuaternionBase<ValueType>;

    private:
        union
        {
            QuaternionType m_quaternion;

            ComputeVector4Base<ValueType> m_xyzw;

            ComputeVector3Base<ValueType> m_xyz;
        };

    public:
        constexpr ComputeQuaternionBase()
            : m_quaternion()
        {
        }
        constexpr ComputeQuaternionBase(ValueType _x, ValueType _y, ValueType _z, ValueType _w)
            : m_quaternion(_x, _y, _z, _w)
        {
        }
        constexpr ComputeQuaternionBase(const QuaternionType& _quaternion)
            : m_quaternion(_quaternion)
        {
        }
        constexpr ComputeQuaternionBase(const ComputeVector4Base<ValueType>& _vector)
            : m_xyzw(_vector)
        {
        }
        constexpr ComputeQuaternionBase(const ComputeQuaternionBase& _quaternion)
            : m_quaternion(_quaternion.m_quaternion)
        {
        }

        explicit ComputeQuaternionBase(const ComputeMatrix3x3Base<ValueType>& _matrix);
        explicit ComputeQuaternionBase(const ComputeMatrix4x4Base<ValueType>& _matrix);
        explicit ComputeQuaternionBase(const ComputeVector3Base<ValueType>& _axis, ValueType _angle);

        ComputeQuaternionBase& operator=(const ComputeQuaternionBase& _quaternion);

        bool operator==(const ComputeQuaternionBase& _quaternion) const;
        bool operator!=(const ComputeQuaternionBase& _quaternion) const;

        ComputeQuaternionBase& operator*=(const ComputeQuaternionBase& _quaternion);

        ValueType operator[](uint32_t _index) const
        {
            return getElement(_index);
        }
        ValueType& operator[](uint32_t _index)
        {
            return getElement(_index);
        }

        ValueType getElement(uint32_t _index) const
        {
            return m_quaternion.m_elements.getElement(_index);
        }
        ValueType& getElement(uint32_t _index)
        {
            return m_quaternion.m_elements.getElement(_index);
        }

        ComputeQuaternionBase& setupFromRotationMatrix3x3(const ComputeMatrix3x3Base<ValueType>& _matrix);
        ComputeQuaternionBase& setupFromRotationMatrix4x4(const ComputeMatrix4x4Base<ValueType>& _matrix);
        ComputeQuaternionBase& setupFromAxisAngle(const ComputeVector3Base<ValueType>& _axis, ValueType _angle);
        ComputeQuaternionBase& setupFromEulerAngles(const ComputeVector3Base<ValueType>& _eulerAngles);
        ComputeQuaternionBase& setupFromRollPitchYaw(ValueType _roll, ValueType _pitch, ValueType _yaw);

        ValueType getX() const
        {
            return m_quaternion.m_elements.m_x;
        }
        ValueType getY() const
        {
            return m_quaternion.m_elements.m_y;
        }
        ValueType getZ() const
        {
            return m_quaternion.m_elements.m_z;
        }
        ValueType getW() const
        {
            return m_quaternion.m_elements.m_w;
        }
        ValueType& getX()
        {
            return m_quaternion.m_elements.m_x;
        }
        ValueType& getY()
        {
            return m_quaternion.m_elements.m_y;
        }
        ValueType& getZ()
        {
            return m_quaternion.m_elements.m_z;
        }
        ValueType& getW()
        {
            return m_quaternion.m_elements.m_w;
        }
        void setX(ValueType _value)
        {
            m_quaternion.m_elements.m_x = _value;
        }
        void setY(ValueType _value)
        {
            m_quaternion.m_elements.m_y = _value;
        }
        void setZ(ValueType _value)
        {
            m_quaternion.m_elements.m_z = _value;
        }
        void setW(ValueType _value)
        {
            m_quaternion.m_elements.m_w = _value;
        }

        const ComputeVector4Base<ValueType>& getVector() const
        {
            return m_xyzw;
        }
        ComputeVector4Base<ValueType>& getVector()
        {
            return m_xyzw;
        }
        const ComputeVector3Base<ValueType>& getXYZ() const
        {
            return m_xyz;
        }
        ComputeVector3Base<ValueType>& getXYZ()
        {
            return m_xyz;
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
    constexpr ComputeQuaternionBase<T> ComputeQuaternionZeroBase()
    {
        return ComputeQuaternionBase<T>(T(0.0), T(0.0), T(0.0), T(0.0));
    }
    template <typename T>
    constexpr ComputeQuaternionBase<T> ComputeQuaternionIdentityBase()
    {
        return ComputeQuaternionBase<T>(T(0.0), T(0.0), T(0.0), T(1.0));
    }

    template <typename T>
    ComputeVector3Base<T> EulerAnglesFromQuaternion(const ComputeQuaternionBase<T>& _rotation)
    {
        return _rotation.getEulerAngles();
    }

    using ComputeQuaternion = ComputeQuaternionBase<ComputeValueType>;
    using FloatComputeQuaternion = ComputeQuaternionBase<float>;

    inline constexpr ComputeQuaternion ComputeQuaternionZero = ComputeQuaternionZeroBase<ComputeValueType>();
    inline constexpr ComputeQuaternion ComputeQuaternionIdentity = ComputeQuaternionIdentityBase<ComputeValueType>();
} // namespace ego

#include "ComputeQuaternion.hpp"