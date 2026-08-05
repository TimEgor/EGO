#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numbers>
#include <type_traits>
#include <utility>

#include "EgoCore/Assert/Assert.h"

namespace ego
{
#if defined(EGO_COMPUTE_TYPE_DOUBLE)
    using ComputeValue = double;
#else
    using ComputeValue = float;
    #ifndef EGO_COMPUTE_TYPE_FLOAT
        #define EGO_COMPUTE_TYPE_FLOAT
    #endif
#endif

    using ComputeValueType = ComputeValue;

    namespace literals
    {
        constexpr ComputeValue operator""_ecv(long double _value)
        {
            return static_cast<ComputeValue>(_value);
        }
    } // namespace literals

    namespace math
    {
        template <typename T>
        constexpr T TypedPi()
        {
            return std::numbers::pi_v<T>;
        }

        template <typename T>
        constexpr T TypedPi2()
        {
            return TypedPi<T>() * T(2.0);
        }

        template <typename T>
        constexpr T TypedHalfPi()
        {
            return TypedPi<T>() * T(0.5);
        }

        template <typename T>
        constexpr T TypedDegToRad()
        {
            return TypedPi<T>() / T(180.0);
        }

        template <typename T>
        constexpr T TypedRadToDeg()
        {
            return T(180.0) / TypedPi<T>();
        }

        template <typename T>
        constexpr T TypedEpsilon()
        {
            return std::numeric_limits<T>::epsilon();
        }

        template <typename T>
        constexpr T TypedEpsilonSqr()
        {
            return TypedEpsilon<T>() * TypedEpsilon<T>();
        }

        template <typename T>
        constexpr T TypedMax()
        {
            return std::numeric_limits<T>::max();
        }

        template <typename T>
        constexpr T TypedMin()
        {
            return std::numeric_limits<T>::lowest();
        }

        template <typename T>
        constexpr T TypedPositiveMin()
        {
            return std::numeric_limits<T>::min();
        }

        inline constexpr ComputeValue Pi = TypedPi<ComputeValue>();
        inline constexpr ComputeValue Pi2 = TypedPi2<ComputeValue>();
        inline constexpr ComputeValue HalfPi = TypedHalfPi<ComputeValue>();

        inline constexpr ComputeValue DegToRad = TypedDegToRad<ComputeValue>();
        inline constexpr ComputeValue RadToDeg = TypedRadToDeg<ComputeValue>();

        inline constexpr ComputeValue Epsilon = TypedEpsilon<ComputeValue>();
        inline constexpr ComputeValue EpsilonSqr = TypedEpsilonSqr<ComputeValue>();

        inline constexpr ComputeValue Max = TypedMax<ComputeValue>();
        inline constexpr ComputeValue Min = TypedMin<ComputeValue>();
        inline constexpr ComputeValue PositiveMin = TypedPositiveMin<ComputeValue>();

        template <typename T>
        T ConvertDegToRad(T _angle)
        {
            return _angle * TypedDegToRad<T>();
        }

        template <typename T>
        T ConvertRadToDeg(T _angle)
        {
            return _angle * TypedRadToDeg<T>();
        }

        template <typename T>
        bool IsApproxEqual(T _value1, T _value2, T _epsilon = TypedEpsilon<T>())
        {
            EGO_ASSERT(_epsilon > T(0.0));
            return std::abs(_value1 - _value2) < _epsilon;
        }

        template <typename T>
        bool IsApproxLess(T _value1, T _value2, T _epsilon = TypedEpsilon<T>()) // is value1 less than value2
        {
            EGO_ASSERT(_epsilon > T(0.0));
            return (_value2 - _value1) > _epsilon;
        }

        template <typename T>
        bool IsApproxLessOrEqual(
            T _value1,
            T _value2,
            T _epsilon = TypedEpsilon<T>()) // is value1 less or equal than value2
        {
            EGO_ASSERT(_epsilon > T(0.0));
            return (_value1 - _value2) <= _epsilon;
        }

        template <typename T>
        bool IsApproxGreater(T _value1, T _value2, T _epsilon = TypedEpsilon<T>()) // is value1 greater than value2
        {
            EGO_ASSERT(_epsilon > T(0.0));
            return (_value1 - _value2) > _epsilon;
        }

        template <typename T>
        bool IsApproxGreaterOrEqual(
            T _value1,
            T _value2,
            T _epsilon = TypedEpsilon<T>()) // is value1 greater or equal than value2
        {
            EGO_ASSERT(_epsilon > T(0.0));
            return (_value2 - _value1) <= _epsilon;
        }

        template <typename T>
        T Clamp(T _value, T _minValue, T _maxValue)
        {
            return std::min(std::max(_value, _minValue), _maxValue);
        }
    } // namespace math
} // namespace ego
