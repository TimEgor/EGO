#pragma once

#include "ComputeMath.h"
#include "Vector.h"

namespace ego
{
    template <typename T, uint32_t Size>
    struct ComputeVectorStorageTraits;

    template <typename T, uint32_t Size>
    class ComputeMatrixBase;

    template <typename T>
    struct ComputeVectorStorageTraits<T, 2>
    {
        using StorageType = Vector2Base<T>;
    };

    template <typename T>
    struct ComputeVectorStorageTraits<T, 3>
    {
        using StorageType = Vector3Base<T>;
    };

    template <typename T>
    struct ComputeVectorStorageTraits<T, 4>
    {
        using StorageType = Vector4Base<T>;
    };

    template <typename T, uint32_t Size>
    class ComputeVectorBase final
    {
    public:
        using ValueType = T;
        static_assert(std::is_arithmetic_v<ValueType>);
        static_assert(Size >= 2 && Size <= 4);

        static constexpr uint32_t ElementCount = Size;
        static constexpr uint32_t NativeElementCount = 4;

        using VectorType = typename ComputeVectorStorageTraits<ValueType, Size>::StorageType;
        using NativeVectorType = math::simd::NativeComputeVectorStorage<ValueType>;

        ComputeVectorBase();
        explicit ComputeVectorBase(ValueType _value);
        ComputeVectorBase(ValueType _x, ValueType _y)
            requires(Size == 2);
        ComputeVectorBase(ValueType _x, ValueType _y, ValueType _z)
            requires(Size == 3);
        ComputeVectorBase(ValueType _x, ValueType _y, ValueType _z, ValueType _w)
            requires(Size == 4);
        template <typename SourceValueType>
        ComputeVectorBase(const Vector2Base<SourceValueType>& _vector)
            requires(Size == 2);
        template <typename SourceValueType>
        ComputeVectorBase(const Vector3Base<SourceValueType>& _vector)
            requires(Size == 3);
        template <typename SourceValueType>
        ComputeVectorBase(const Vector4Base<SourceValueType>& _vector)
            requires(Size == 4);
        ComputeVectorBase(const ComputeVectorBase<ValueType, 2>& _vector, ValueType _z)
            requires(Size == 3);
        ComputeVectorBase(const ComputeVectorBase<ValueType, 2>& _vector1, const ComputeVectorBase<ValueType, 2>& _vector2)
            requires(Size == 4);
        ComputeVectorBase(const ComputeVectorBase<ValueType, 2>& _vector, ValueType _z, ValueType _w)
            requires(Size == 4);
        ComputeVectorBase(const ComputeVectorBase<ValueType, 2>& _vector)
            requires(Size == 3 || Size == 4);
        ComputeVectorBase(const ComputeVectorBase<ValueType, 3>& _vector)
            requires(Size == 4);
        ComputeVectorBase(const ComputeVectorBase<ValueType, 3>& _vector, ValueType _w)
            requires(Size == 4);
        ComputeVectorBase(const ComputeVectorBase& _vector) = default;

        ComputeVectorBase& operator=(const ComputeVectorBase& _vector) = default;

        ComputeVectorBase& operator+=(const ComputeVectorBase& _vector);
        ComputeVectorBase& operator-=(const ComputeVectorBase& _vector);
        ComputeVectorBase& operator*=(ValueType _value);
        ComputeVectorBase& operator/=(ValueType _value);

        bool operator==(const ComputeVectorBase& _vector) const;
        bool operator!=(const ComputeVectorBase& _vector) const;

        ValueType operator[](uint32_t _index) const;

        ValueType getElement(uint32_t _index) const;
        void setElement(uint32_t _index, ValueType _value);

        const NativeVectorType& getNativeValues() const;
        NativeVectorType& getNativeValues();
        void setNativeValues(const NativeVectorType& _values);

        VectorType getVector() const;
        void getVector(VectorType& _out) const;

        template <typename ResultValueType>
        typename ComputeVectorStorageTraits<ResultValueType, Size>::StorageType getVector() const;
        template <typename ResultValueType>
        void getVector(typename ComputeVectorStorageTraits<ResultValueType, Size>::StorageType& _out) const;

        ValueType getX() const;
        ValueType getY() const;
        ValueType getZ() const
            requires(Size >= 3);
        ValueType getW() const
            requires(Size == 4);

        void setX(ValueType _value);
        void setY(ValueType _value);
        void setZ(ValueType _value)
            requires(Size >= 3);
        void setW(ValueType _value)
            requires(Size == 4);

        ComputeVectorBase<ValueType, 2> getXY() const
            requires(Size >= 3);
        ComputeVectorBase<ValueType, 2> getZW() const
            requires(Size == 4);
        ComputeVectorBase<ValueType, 3> getXYZ() const
            requires(Size == 4);

        void setXY(const ComputeVectorBase<ValueType, 2>& _vector)
            requires(Size >= 3);
        void setZW(const ComputeVectorBase<ValueType, 2>& _vector)
            requires(Size == 4);
        void setXYZ(const ComputeVectorBase<ValueType, 3>& _vector)
            requires(Size == 4);

        ValueType getLength() const;
        ValueType getLengthSqr() const;

        ComputeVectorBase& negate();
        ComputeVectorBase& computeReciprocal()
            requires(Size == 3);
        ComputeVectorBase& multiplyElements(const ComputeVectorBase& _vector)
            requires(Size == 3);

        ComputeVectorBase getUnit() const;
        void getUnit(ComputeVectorBase& _out) const
            requires(Size == 3);
        ComputeVectorBase& normalize();

        ComputeVectorBase getOrthogonal() const
            requires(Size == 3);
        void getOrthogonal(ComputeVectorBase& _out) const
            requires(Size == 3);

        ComputeVectorBase getAbs() const
            requires(Size == 3);
        void getAbs(ComputeVectorBase& _out) const
            requires(Size == 3);

        uint32_t getMinElementIndex() const
            requires(Size == 3);
        uint32_t getMaxElementIndex() const
            requires(Size == 3);

        ValueType dot(const ComputeVectorBase& _vector) const;
        ComputeVectorBase cross(const ComputeVectorBase& _vector) const
            requires(Size == 3);

        ComputeVectorBase swizzle(uint32_t _index1, uint32_t _index2, uint32_t _index3) const
            requires(Size == 3);

        bool isEqual(const ComputeVectorBase& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
        bool isLess(const ComputeVectorBase& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
        bool isLessOrEqual(const ComputeVectorBase& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
        bool isGreater(const ComputeVectorBase& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
        bool isGreaterOrEqual(const ComputeVectorBase& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
        bool isAnyLess(const ComputeVectorBase& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
        bool isAnyLessOrEqual(const ComputeVectorBase& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
        bool isAnyGreater(const ComputeVectorBase& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
        bool isAnyGreaterOrEqual(const ComputeVectorBase& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;

        bool isUnit() const;
        bool isZero() const;

    private:
        template <typename, uint32_t>
        friend class ComputeVectorBase;
        template <typename, uint32_t>
        friend class ComputeMatrixBase;

        NativeVectorType m_nativeValues;
    };

    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator+(const ComputeVectorBase<T, Size>& _vector1, const ComputeVectorBase<T, Size>& _vector2);
    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator-(const ComputeVectorBase<T, Size>& _vector1, const ComputeVectorBase<T, Size>& _vector2);
    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator-(const ComputeVectorBase<T, Size>& _vector);
    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator*(const ComputeVectorBase<T, Size>& _vector, T _value);
    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator*(T _value, const ComputeVectorBase<T, Size>& _vector);
    template <typename T, uint32_t Size>
    ComputeVectorBase<T, Size> operator/(const ComputeVectorBase<T, Size>& _vector, T _value);
} // namespace ego

#include "ComputeVector.hpp"
