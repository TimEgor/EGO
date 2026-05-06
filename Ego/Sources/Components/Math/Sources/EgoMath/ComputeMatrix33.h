#pragma once

#include "ComputeMatrix22.h"
#include "ComputeVector3.h"

namespace ego
{
	template <typename T>
	class ComputeMatrix3x3Base final
	{
	public:
		using ValueType = T;
		static_assert(std::is_arithmetic_v<ValueType>);

		using MatrixType = Matrix3x3Base<ValueType>;

	private:
		union
		{
			MatrixType m_matrix; // store values in a transposed form (column-major)

			ComputeVector3Base<ValueType> m_columns[3];

			struct
			{
				ComputeVector3Base<ValueType> m_column1;
				ComputeVector3Base<ValueType> m_column2;
				ComputeVector3Base<ValueType> m_column3;
			};
		};

	public:
		constexpr ComputeMatrix3x3Base()
			: m_matrix()
		{}
		constexpr ComputeMatrix3x3Base(
			ValueType _m11, ValueType _m12, ValueType _m13,
			ValueType _m21, ValueType _m22, ValueType _m23,
			ValueType _m31, ValueType _m32, ValueType _m33
		)
			: m_matrix(
				_m11, _m12, _m13,
				_m21, _m22, _m23,
				_m31, _m32, _m33
			)
		{}

		constexpr ComputeMatrix3x3Base(const MatrixType& _matrix)
			: m_matrix(_matrix) {}

		constexpr ComputeMatrix3x3Base(
			const ComputeVector3Base<ValueType>& _column1,
			const ComputeVector3Base<ValueType>& _column2,
			const ComputeVector3Base<ValueType>& _column3
		)
			: m_column1(_column1),
			m_column2(_column2),
			m_column3(_column3)
		{}

		constexpr ComputeMatrix3x3Base(const ComputeMatrix2x2Base<ValueType>& _matrix)
			: m_column1(_matrix.getColumn(0)),
			m_column2(_matrix.getColumn(1)),
			m_column3(ValueType(0.0))
		{}

		constexpr ComputeMatrix3x3Base(const ComputeMatrix3x3Base& _matrix)
			: m_matrix(_matrix.m_matrix) {}

		ComputeMatrix3x3Base& operator=(const ComputeMatrix3x3Base& _matrix);

		ComputeMatrix3x3Base& operator+=(const ComputeMatrix3x3Base& _matrix);
		ComputeMatrix3x3Base& operator-=(const ComputeMatrix3x3Base& _matrix);
		ComputeMatrix3x3Base& operator*=(const ComputeMatrix3x3Base& _matrix);
		ComputeMatrix3x3Base& operator*=(ValueType _value);
		ComputeMatrix3x3Base& operator/=(ValueType _value);

		bool operator==(const ComputeMatrix3x3Base& _matrix) const;
		bool operator!=(const ComputeMatrix3x3Base& _matrix) const;

		FloatMatrix3x3 getFloatMatrix3x3() const;
		void getFloatMatrix3x3(FloatMatrix3x3& _out) const { _out = m_matrix; }

		ValueType getElement(uint32_t _row, uint32_t _column) const { return m_matrix.getElement(_column, _row); }
		ValueType& getElement(uint32_t _row, uint32_t _column) { return m_matrix.getElement(_column, _row); }
		void setElement(uint32_t _row, uint32_t _column, ValueType _value) { return m_matrix.setElement(_column, _row, _value); }

		const ComputeVector3Base<ValueType>& getColumn(uint32_t _index) const;
		ComputeVector3Base<ValueType>& getColumn(uint32_t _index);
		ComputeVector3Base<ValueType> getRow(uint32_t _index) const;

		void setColumn(uint32_t _index, const ComputeVector3Base<ValueType>& _column);
		void setRow(uint32_t _index, const ComputeVector3Base<ValueType>& _row);

		ComputeMatrix3x3Base& negate();

		ComputeMatrix3x3Base& transpose();

		ValueType getDeterminant() const;
		ComputeMatrix3x3Base& invert();

		ComputeVector3Base<ValueType> transform(const ComputeVector3Base<ValueType>& _vector) const;
		void transform(const ComputeVector3Base<ValueType>& _vector, ComputeVector3Base<ValueType>& _out) const;

		bool isEqual(const ComputeMatrix3x3Base& _matrix, ValueType _epsilon = math::TypedEpsilon<ValueType>()) const;
	};

	template <typename T>
	ComputeMatrix3x3Base<T> operator+(const ComputeMatrix3x3Base<T>& _matrix1, const ComputeMatrix3x3Base<T>& _matrix2);
	template <typename T>
	ComputeMatrix3x3Base<T> operator-(const ComputeMatrix3x3Base<T>& _matrix1, const ComputeMatrix3x3Base<T>& _matrix2);
	template <typename T>
	ComputeMatrix3x3Base<T> operator-(const ComputeMatrix3x3Base<T>& _matrix);
	template <typename T>
	ComputeMatrix3x3Base<T> operator*(const ComputeMatrix3x3Base<T>& _matrix1, const ComputeMatrix3x3Base<T>& _matrix2);
	template <typename T>
	ComputeMatrix3x3Base<T> operator*(const ComputeMatrix3x3Base<T>& _matrix, T _value);
	template <typename T>
	ComputeMatrix3x3Base<T> operator*(T _value, const ComputeMatrix3x3Base<T>& _matrix);
	template <typename T>
	ComputeMatrix3x3Base<T> operator/(const ComputeMatrix3x3Base<T>& _matrix, T _value);
	template <typename T>
	ComputeMatrix3x3Base<T> operator/(T _value, const ComputeMatrix3x3Base<T>& _matrix);

	template <typename T>
	ComputeVector3Base<T> operator*(const ComputeMatrix3x3Base<T>& _matrix, const ComputeVector3Base<T>& _vector);

	template <typename T>
	ComputeMatrix3x3Base<T> CrossProductSkewSymmetricComputeMatrix3x3(const ComputeVector3Base<T>& _vector);
	template <typename T>
	ComputeMatrix3x3Base<T> RotationEulerComputeMatrix3x3(const ComputeVector3Base<T>& _vector);
	template <typename T>
	ComputeMatrix3x3Base<T> RotationEulerComputeMatrix3x3(T _roll, T _pitch, T _yaw);
	template <typename T>
	ComputeVector3Base<T> EulerAnglesFromRotationComputeMatrix3x3(const ComputeMatrix3x3Base<T>& _rotation);
	template <typename T>
	ComputeMatrix3x3Base<T> ScaleComputeMatrix3x3(const ComputeVector3Base<T>& _scale);

	template <typename T>
	ComputeMatrix3x3Base<T> InvertComputeMatrix3x3(const ComputeMatrix3x3Base<T>& _matrix);
	template <typename T>
	ComputeMatrix3x3Base<T> TransposeComputeMatrix3x3(const ComputeMatrix3x3Base<T>& _matrix);

	template <typename T>
	constexpr ComputeMatrix3x3Base<T> ComputeMatrix3x3ZeroBase() {
		return ComputeMatrix3x3Base<T>(
			T(0.0), T(0.0), T(0.0),
			T(0.0), T(0.0), T(0.0),
			T(0.0), T(0.0), T(0.0)
		);
	}

	template <typename T>
	constexpr ComputeMatrix3x3Base<T> ComputeMatrix3x3IdentityBase() {
		return ComputeMatrix3x3Base<T>(
			T(1.0), T(0.0), T(0.0),
			T(0.0), T(1.0), T(0.0),
			T(0.0), T(0.0), T(1.0)
		);
	}

	using ComputeMatrix3x3 = ComputeMatrix3x3Base<ComputeValueType>;
	using FloatComputeMatrix3x3 = ComputeMatrix3x3Base<float>;

	constexpr ComputeMatrix3x3 ComputeMatrix3x3Zero = ComputeMatrix3x3ZeroBase<ComputeValueType>();
	constexpr ComputeMatrix3x3 ComputeMatrix3x3Identity = ComputeMatrix3x3IdentityBase<ComputeValueType>();

	constexpr FloatComputeMatrix3x3 FloatComputeMatrix3x3Zero = ComputeMatrix3x3ZeroBase<float>();
	constexpr FloatComputeMatrix3x3 FloatComputeMatrix3x3Identity = ComputeMatrix3x3IdentityBase<float>();
}

#include "ComputeMatrix33.hpp"