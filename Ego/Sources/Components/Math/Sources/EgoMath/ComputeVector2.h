#pragma once

#include "ComputeMath.h"
#include "Vector.h"

namespace ego
{
	template <typename T>
	class ComputeVector2Base final
	{
	public:
		using ValueType = T;
		static_assert(std::is_arithmetic_v<ValueType>);

		using VectorType = Vector2Base<ValueType>;

	private:
		VectorType m_vector;

	public:
		constexpr ComputeVector2Base()
			: m_vector() {}
		constexpr ComputeVector2Base(ValueType _value)
			: m_vector(_value) {}
		constexpr ComputeVector2Base(ValueType _x, ValueType _y)
			: m_vector(_x, _y) {}
		constexpr ComputeVector2Base(const VectorType& _vector)
			: m_vector(_vector) {}
		constexpr ComputeVector2Base(const ComputeVector2Base& _vector)
			: m_vector(_vector.m_vector) {}

		ComputeVector2Base& operator=(const ComputeVector2Base& _vector);

		ComputeVector2Base& operator+=(const ComputeVector2Base& _vector);
		ComputeVector2Base& operator-=(const ComputeVector2Base& _vector);
		ComputeVector2Base& operator*=(ValueType _value);
		ComputeVector2Base& operator/=(ValueType _value);

		bool operator==(const ComputeVector2Base& _vector) const;
		bool operator!=(const ComputeVector2Base& _vector) const;

		ValueType operator[](uint32_t _index) const { return getElement(_index); }
		ValueType& operator[](uint32_t _index) { return getElement(_index); }

		ValueType getElement(uint32_t _index) const { return m_vector.getElement(_index); }
		ValueType& getElement(uint32_t _index) { return m_vector.getElement(_index); }

		FloatVector2 getFloatVector2() const;
		void getFloatVector2(FloatVector2& _out) const;

		ValueType getX() const { return m_vector.m_x; }
		ValueType getY() const { return m_vector.m_y; }
		ValueType& getX() { return m_vector.m_x; }
		ValueType& getY() { return m_vector.m_y; }
		void setX(ValueType _value) { m_vector.m_x = _value; }
		void setY(ValueType _value) { m_vector.m_y = _value; }

		ValueType getLength() const;
		ValueType getLengthSqr() const;

		ComputeVector2Base& negate();

		ComputeVector2Base getUnit() const;
		ComputeVector2Base& normalize();

		T dot(const ComputeVector2Base& _vector) const;

		bool isEqual(const ComputeVector2Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
		bool isLess(const ComputeVector2Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this < vector
		bool isLessOrEqual(const ComputeVector2Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this <= vector
		bool isGreater(const ComputeVector2Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this > vector
		bool isGreaterOrEqual(const ComputeVector2Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this >= vector

		bool isUnit() const;
		bool isZero() const;
	};

	template <typename T>
	ComputeVector2Base<T> operator+(const ComputeVector2Base<T>& _vector1, const ComputeVector2Base<T>& _vector2);
	template <typename T>
	ComputeVector2Base<T> operator-(const ComputeVector2Base<T>& _vector1, const ComputeVector2Base<T>& _vector2);
	template <typename T>
	ComputeVector2Base<T> operator-(const ComputeVector2Base<T>& _vector);
	template <typename T>
	ComputeVector2Base<T> operator*(const ComputeVector2Base<T>& _vector, T _value);
	template <typename T>
	ComputeVector2Base<T> operator*(T _value, const ComputeVector2Base<T>& _vector);
	template <typename T>
	ComputeVector2Base<T> operator/(const ComputeVector2Base<T>& _vector, T _value);
	template <typename T>
	ComputeVector2Base<T> operator/(T _value, const ComputeVector2Base<T>& _vector);

	template <typename T>
	T DotComputeVector2(const ComputeVector2Base<T>& _vector1, const ComputeVector2Base<T>& _vector2);

	template <typename T>
	constexpr ComputeVector2Base<T> ComputeVector2ZeroBase() { return ComputeVector2Base<T>(0.0); }
	template <typename T>
	constexpr ComputeVector2Base<T> ComputeVector2OneBase() { return ComputeVector2Base<T>(1.0); }
	template <typename T>
	constexpr ComputeVector2Base<T> ComputeVector2NegativeOneBase() { return ComputeVector2Base<T>(-1.0); }

	template <typename T>
	constexpr ComputeVector2Base<T> ComputeVector2EpsilonBase() { return ComputeVector2Base<T>(math::TypedEpsilon<T>()); }

	using ComputeVector2 = ComputeVector2Base<ComputeValueType>;
	using FloatComputeVector2 = ComputeVector2Base<float>;

	inline constexpr ComputeVector2 ComputeVector2Zero = ComputeVector2ZeroBase<ComputeValueType>();
	inline constexpr ComputeVector2 ComputeVector2One = ComputeVector2OneBase<ComputeValueType>();
	inline constexpr ComputeVector2 ComputeVector2NegativeOne = ComputeVector2NegativeOneBase<ComputeValueType>();

	inline constexpr ComputeVector2 ComputeVector2Epsilon = ComputeVector2EpsilonBase<ComputeValueType>();
}

#include "ComputeVector2.hpp"