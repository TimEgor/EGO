#pragma once

#include "Vector.h"

namespace ego
{
	template <typename T>
	struct QuaternionBase final
	{
		using VectorType = Vector4Base<T>;

		VectorType m_elements;

		constexpr QuaternionBase() = default;
		constexpr QuaternionBase(T _x, T _y, T _z, T _w) : m_elements(_x, _y, _z, _w) {}
		constexpr QuaternionBase(const VectorType& _elements) : m_elements(_elements) {}
		constexpr QuaternionBase(const QuaternionBase& _quaternion) = default;

		QuaternionBase& operator=(const QuaternionBase& _quaternion) = default;
	};

	using FloatQuaternion = QuaternionBase<float>;

	inline constexpr FloatQuaternion FloatQuaternionZero = FloatQuaternion(0.0f, 0.0f, 0.0f, 0.0f);
	inline constexpr FloatQuaternion FloatQuaternionIdentity = FloatQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
}