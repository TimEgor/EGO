#pragma once

#include "ComputeVector2.h"

namespace ego
{
	template <typename T>
	class ComputeVector3Base final
	{
		using ValueType = T;
		static_assert(std::is_arithmetic_v<ValueType>);

		using VectorType = Vector3Base<ValueType>;

	private:
		union
		{
			VectorType m_vector;

			ComputeVector2Base<ValueType> m_vectorXY;
		};

	public:
		constexpr ComputeVector3Base()
			: m_vector() {}
		constexpr ComputeVector3Base(ValueType _value)
			: m_vector(_value) {}
		constexpr ComputeVector3Base(ValueType _x, ValueType _y, ValueType _z)
			: m_vector(_x, _y, _z) {}
		constexpr ComputeVector3Base(const VectorType& _vector)
			: m_vector(_vector) {}
		constexpr ComputeVector3Base(const ComputeVector2Base<ValueType>& _vector)
			: m_vector(_vector.getX(), _vector.getY(), VectorType::DefaultValue) {}
		constexpr ComputeVector3Base(const ComputeVector2Base<ValueType>& _vector, ValueType _z)
			: m_vector(_vector.getX(), _vector.getY(), _z) {}
		constexpr ComputeVector3Base(const ComputeVector3Base& _vector)
			: m_vector(_vector.m_vector) {}

		ComputeVector3Base& operator=(const ComputeVector3Base& _vector);

		ComputeVector3Base& operator+=(const ComputeVector3Base& _vector);
		ComputeVector3Base& operator-=(const ComputeVector3Base& _vector);
		ComputeVector3Base& operator*=(T _value);
		ComputeVector3Base& operator/=(T _value);

		bool operator==(const ComputeVector3Base& _vector) const;
		bool operator!=(const ComputeVector3Base& _vector) const;

		ValueType operator[](uint32_t _index) const { return getElement(_index); }
		ValueType& operator[](uint32_t _index) { return getElement(_index); }

		ValueType getElement(uint32_t _index) const { return m_vector.getElement(_index); }
		ValueType& getElement(uint32_t _index) { return m_vector.getElement(_index); }

		FloatVector3 getFloatVector3() const;
		void getFloatVector3(FloatVector3& _out) const;

		ValueType getX() const { return m_vector.m_x; }
		ValueType getY() const { return m_vector.m_y; }
		ValueType getZ() const { return m_vector.m_z; }
		ValueType& getX() { return m_vector.m_x; }
		ValueType& getY() { return m_vector.m_y; }
		ValueType& getZ() { return m_vector.m_z; }
		void setX(ValueType _value) { m_vector.m_x = _value; }
		void setY(ValueType _value) { m_vector.m_y = _value; }
		void setZ(ValueType _value) { m_vector.m_z = _value; }

		const ComputeVector2Base<ValueType>& getXY() const { return m_vectorXY; }
		ComputeVector2Base<ValueType>& getXY() { return m_vectorXY; }
		void setXY(const ComputeVector2Base<ValueType>& _vector) { m_vectorXY = _vector; }

		ValueType getLength() const;
		ValueType getLengthSqr() const;

		ComputeVector3Base& negate();
		ComputeVector3Base& computeReciprocal();

		ComputeVector3Base& multiplyElements(const ComputeVector3Base& _vector);

		ComputeVector3Base getUnit() const;
		void getUnit(ComputeVector3Base& _out) const;
		ComputeVector3Base& normalize();

		ComputeVector3Base getOrthogonal() const;
		void getOrthogonal(ComputeVector3Base& _out) const;

		ComputeVector3Base getAbs() const;
		void getAbs(ComputeVector3Base& _out) const;

		uint32_t getMinElementIndex() const;
		uint32_t getMaxElementIndex() const;

		ValueType dot(const ComputeVector3Base& _vector) const;
		ComputeVector3Base cross(const ComputeVector3Base& _vector) const;

		ComputeVector3Base swizzle(uint32_t _index1, uint32_t _index2, uint32_t _index3) const;

		bool isEqual(const ComputeVector3Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
		bool isLess(const ComputeVector3Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this < vector
		bool isLessOrEqual(const ComputeVector3Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this <= vector
		bool isGreater(const ComputeVector3Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this > vector
		bool isGreaterOrEqual(const ComputeVector3Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this >= vector
		bool isAnyLess(const ComputeVector3Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this < vector
		bool isAnyLessOrEqual(const ComputeVector3Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this <= vector
		bool isAnyGreater(const ComputeVector3Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this > vector
		bool isAnyGreaterOrEqual(const ComputeVector3Base& _vector, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const; // *this >= vector

		bool isUnit() const;
		bool isZero() const;
	};

	template <typename T>
	ComputeVector3Base<T> operator+(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2);
	template <typename T>
	ComputeVector3Base<T> operator-(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2);
	template <typename T>
	ComputeVector3Base<T> operator-(const ComputeVector3Base<T>& _vector);
	template <typename T>
	ComputeVector3Base<T> operator*(const ComputeVector3Base<T>& _vector, T _value);
	template <typename T>
	ComputeVector3Base<T> operator*(T _value, const ComputeVector3Base<T>& _vector);
	template <typename T>
	ComputeVector3Base<T> operator/(const ComputeVector3Base<T>& _vector, T _value);
	template <typename T>
	ComputeVector3Base<T> operator/(T _value, const ComputeVector3Base<T>& _vector);

	template <typename T>
	T DotComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2);
	template <typename T>
	ComputeVector3Base<T> CrossComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2);

	template <typename T>
	ComputeVector3Base<T> NegateComputeVector3(const ComputeVector3Base<T>& _vector);
	template <typename T>
	ComputeVector3Base<T> ComputeReciprocalComputeVector3(const ComputeVector3Base<T>& _vector);
	template <typename T>
	ComputeVector3Base<T> NormalizeComputeVector3(const ComputeVector3Base<T>& _vector);

	template <typename T>
	T LengthComputeVector3(const ComputeVector3Base<T>& _vector);
	template <typename T>
	T LengthSqrComputeVector3(const ComputeVector3Base<T>& _vector);

	template <typename T>
	ComputeVector3Base<T> MultiplyComputeVector3Elements(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2);

	template <typename T>
	ComputeVector3Base<T> MinComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2);
	template <typename T>
	ComputeVector3Base<T> MaxComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2);

	template <typename T>
	constexpr ComputeVector3Base<T> ComputeVector3ZeroBase() { return ComputeVector3Base<T>(0.0); }
	template <typename T>
	constexpr ComputeVector3Base<T> ComputeVector3OneBase() { return ComputeVector3Base<T>(1.0); }
	template <typename T>
	constexpr ComputeVector3Base<T> ComputeVector3NegativeOneBase() { return ComputeVector3Base<T>(-1.0); }

	template <typename T>
	constexpr ComputeVector3Base<T> ComputeVector3EpsilonBase() { return ComputeVector3Base<T>(math::TypedEpsilon<T>()); }

	template <typename T>
	constexpr ComputeVector3Base<T> ComputeVector3UnitXBase() { return ComputeVector3Base<T>(1.0, 0.0, 0.0); }
	template <typename T>
	constexpr ComputeVector3Base<T> ComputeVector3UnitYBase() { return ComputeVector3Base<T>(0.0, 1.0, 0.0); }
	template <typename T>
	constexpr ComputeVector3Base<T> ComputeVector3UnitZBase() { return ComputeVector3Base<T>(0.0, 0.0, 1.0); }
	template <typename T>
	constexpr ComputeVector3Base<T> ComputeVector3NegativeUnitXBase() { return ComputeVector3Base<T>(-1.0, 0.0, 0.0); }
	template <typename T>
	constexpr ComputeVector3Base<T> ComputeVector3NegativeUnitYBase() { return ComputeVector3Base<T>(0.0, -1.0, 0.0); }
	template <typename T>
	constexpr ComputeVector3Base<T> ComputeVector3NegativeUnitZBase() { return ComputeVector3Base<T>(0.0, 0.0, -1.0); }

	using ComputeVector3 = ComputeVector3Base<ComputeValueType>;
	using FloatComputeVector3 = ComputeVector3Base<float>;

	inline constexpr ComputeVector3 ComputeVector3Zero = ComputeVector3ZeroBase<ComputeValueType>();
	inline constexpr ComputeVector3 ComputeVector3One = ComputeVector3OneBase<ComputeValueType>();
	inline constexpr ComputeVector3 ComputeVector3NegativeOne = ComputeVector3NegativeOneBase<ComputeValueType>();

	inline constexpr ComputeVector3 ComputeVector3Epsilon = ComputeVector3EpsilonBase<ComputeValueType>();

	inline constexpr ComputeVector3 ComputeVector3UnitX = ComputeVector3UnitXBase<ComputeValueType>();
	inline constexpr ComputeVector3 ComputeVector3UnitY = ComputeVector3UnitYBase<ComputeValueType>();
	inline constexpr ComputeVector3 ComputeVector3UnitZ = ComputeVector3UnitZBase<ComputeValueType>();
	inline constexpr ComputeVector3 ComputeVector3NegativeUnitX = ComputeVector3NegativeUnitXBase<ComputeValueType>();
	inline constexpr ComputeVector3 ComputeVector3NegativeUnitY = ComputeVector3NegativeUnitYBase<ComputeValueType>();
	inline constexpr ComputeVector3 ComputeVector3NegativeUnitZ = ComputeVector3NegativeUnitZBase<ComputeValueType>();

	inline constexpr FloatComputeVector3 FloatComputeVector3Zero = ComputeVector3ZeroBase<float>();
	inline constexpr FloatComputeVector3 FloatComputeVector3One = ComputeVector3OneBase<float>();
	inline constexpr FloatComputeVector3 FloatComputeVector3NegativeOne = ComputeVector3NegativeOneBase<float>();

	inline constexpr FloatComputeVector3 FloatComputeVector3Epsilon = ComputeVector3EpsilonBase<float>();

	inline constexpr FloatComputeVector3 FloatComputeVector3UnitX = ComputeVector3UnitXBase<float>();
	inline constexpr FloatComputeVector3 FloatComputeVector3UnitY = ComputeVector3UnitYBase<float>();
	inline constexpr FloatComputeVector3 FloatComputeVector3UnitZ = ComputeVector3UnitZBase<float>();
	inline constexpr FloatComputeVector3 FloatComputeVector3NegativeUnitX = ComputeVector3NegativeUnitXBase<float>();
	inline constexpr FloatComputeVector3 FloatComputeVector3NegativeUnitY = ComputeVector3NegativeUnitYBase<float>();
	inline constexpr FloatComputeVector3 FloatComputeVector3NegativeUnitZ = ComputeVector3NegativeUnitZBase<float>();
}

#include "ComputeVector3.hpp"