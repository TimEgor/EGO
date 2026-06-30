#pragma once

#include <cstdint>
#include <type_traits>

#include "ComputeNativeStorage.h"

namespace ego
{
    namespace math
    {
        namespace simd
        {
#if EGO_MATH_SIMD_SSE2
            inline __m128 LoadNativeFloat32x4(const NativeComputeVectorStorage<float>& _values)
            {
                return _values.m_value;
            }

            inline void StoreNativeFloat32x4(NativeComputeVectorStorage<float>& _target, __m128 _values)
            {
                _target.m_value = _values;
            }

            inline __m128d LoadNativeFloat64x2(const NativeComputeVectorStorage<double>& _values, uint32_t _elementIndex)
            {
                return _elementIndex < Float64x2ElementCount ? _values.m_lowValue : _values.m_highValue;
            }

            inline void StoreNativeFloat64x2(NativeComputeVectorStorage<double>& _target, __m128d _values, uint32_t _elementIndex)
            {
                if (_elementIndex < Float64x2ElementCount)
                {
                    _target.m_lowValue = _values;
                    return;
                }

                _target.m_highValue = _values;
            }

            template <typename VectorType>
            __m128 LoadFloat32x4(const VectorType& _vector, uint32_t _elementCount)
            {
                if constexpr (std::is_same_v<typename VectorType::ValueType, float> && requires { _vector.getNativeValues(); })
                {
                    return LoadNativeFloat32x4(_vector.getNativeValues());
                }

                alignas(16) float values[Float32x4ElementCount] = {};

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    values[elementIndex] = _vector.getElement(elementIndex);
                }

                return _mm_load_ps(values);
            }

            template <typename VectorType>
            void StoreFloat32x4(VectorType& _vector, __m128 _values, uint32_t _elementCount)
            {
                if constexpr (std::is_same_v<typename VectorType::ValueType, float> && requires { _vector.getNativeValues(); })
                {
                    StoreNativeFloat32x4(_vector.getNativeValues(), _values);
                    return;
                }

                alignas(16) float values[Float32x4ElementCount];
                _mm_store_ps(values, _values);

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    _vector.setElement(elementIndex, values[elementIndex]);
                }
            }

            inline float SumFloat32x4(__m128 _values, uint32_t _elementCount)
            {
                alignas(16) float values[Float32x4ElementCount];
                _mm_store_ps(values, _values);

                float result = 0.0f;
                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    result += values[elementIndex];
                }

                return result;
            }

            template <typename VectorType>
            __m128d LoadFloat64x2(const VectorType& _vector, uint32_t _elementIndex)
            {
                if constexpr (std::is_same_v<typename VectorType::ValueType, double> && requires { _vector.getNativeValues(); })
                {
                    return LoadNativeFloat64x2(_vector.getNativeValues(), _elementIndex);
                }

                return _mm_set_pd(_vector.getElement(_elementIndex + 1), _vector.getElement(_elementIndex));
            }

            template <typename VectorType>
            void StoreFloat64x2(VectorType& _vector, __m128d _values, uint32_t _elementIndex)
            {
                if constexpr (std::is_same_v<typename VectorType::ValueType, double> && requires { _vector.getNativeValues(); })
                {
                    StoreNativeFloat64x2(_vector.getNativeValues(), _values, _elementIndex);
                    return;
                }

                alignas(16) double values[Float64x2ElementCount];
                _mm_store_pd(values, _values);

                _vector.setElement(_elementIndex, values[0]);
                _vector.setElement(_elementIndex + 1, values[1]);
            }

            inline double SumFloat64x2(__m128d _values)
            {
                alignas(16) double values[Float64x2ElementCount];
                _mm_store_pd(values, _values);

                return values[0] + values[1];
            }

            enum class VectorCompareOperation
            {
                Equal,
                Less,
                LessOrEqual,
                Greater,
                GreaterOrEqual
            };

            inline uint32_t MakeElementMask(uint32_t _elementCount)
            {
                return (uint32_t(1) << _elementCount) - uint32_t(1);
            }

            inline __m128 AbsFloat32x4(__m128 _values)
            {
                return _mm_andnot_ps(_mm_set1_ps(-0.0f), _values);
            }

            inline __m128d AbsFloat64x2(__m128d _values)
            {
                return _mm_andnot_pd(_mm_set1_pd(-0.0), _values);
            }

            inline __m128 CompareFloat32x4(__m128 _value1, __m128 _value2, __m128 _epsilon, VectorCompareOperation _operation)
            {
                switch (_operation)
                {
                    case VectorCompareOperation::Equal:
                        return _mm_cmplt_ps(AbsFloat32x4(_mm_sub_ps(_value1, _value2)), _epsilon);
                    case VectorCompareOperation::Less:
                        return _mm_cmpgt_ps(_mm_sub_ps(_value2, _value1), _epsilon);
                    case VectorCompareOperation::LessOrEqual:
                        return _mm_cmple_ps(_mm_sub_ps(_value1, _value2), _epsilon);
                    case VectorCompareOperation::Greater:
                        return _mm_cmpgt_ps(_mm_sub_ps(_value1, _value2), _epsilon);
                    case VectorCompareOperation::GreaterOrEqual:
                        return _mm_cmple_ps(_mm_sub_ps(_value2, _value1), _epsilon);
                    default:
                        return _mm_setzero_ps();
                }
            }

            inline __m128d CompareFloat64x2(__m128d _value1, __m128d _value2, __m128d _epsilon, VectorCompareOperation _operation)
            {
                switch (_operation)
                {
                    case VectorCompareOperation::Equal:
                        return _mm_cmplt_pd(AbsFloat64x2(_mm_sub_pd(_value1, _value2)), _epsilon);
                    case VectorCompareOperation::Less:
                        return _mm_cmpgt_pd(_mm_sub_pd(_value2, _value1), _epsilon);
                    case VectorCompareOperation::LessOrEqual:
                        return _mm_cmple_pd(_mm_sub_pd(_value1, _value2), _epsilon);
                    case VectorCompareOperation::Greater:
                        return _mm_cmpgt_pd(_mm_sub_pd(_value1, _value2), _epsilon);
                    case VectorCompareOperation::GreaterOrEqual:
                        return _mm_cmple_pd(_mm_sub_pd(_value2, _value1), _epsilon);
                    default:
                        return _mm_setzero_pd();
                }
            }

            template <typename T>
            bool CompareScalarElement(T _value1, T _value2, T _epsilon, VectorCompareOperation _operation)
            {
                switch (_operation)
                {
                    case VectorCompareOperation::Equal:
                        return (_value1 - _value2) < _epsilon && (_value2 - _value1) < _epsilon;
                    case VectorCompareOperation::Less:
                        return (_value2 - _value1) > _epsilon;
                    case VectorCompareOperation::LessOrEqual:
                        return (_value1 - _value2) <= _epsilon;
                    case VectorCompareOperation::Greater:
                        return (_value1 - _value2) > _epsilon;
                    case VectorCompareOperation::GreaterOrEqual:
                        return (_value2 - _value1) <= _epsilon;
                    default:
                        return false;
                }
            }

            template <typename VectorType>
            VectorType& AddFloat64VectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
            {
                uint32_t elementIndex = 0;

                for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                {
                    const __m128d targetValues = LoadFloat64x2(_target, elementIndex);
                    const __m128d sourceValues = LoadFloat64x2(_source, elementIndex);
                    StoreFloat64x2(_target, _mm_add_pd(targetValues, sourceValues), elementIndex);
                }

                for (; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, _target.getElement(elementIndex) + _source.getElement(elementIndex));
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& SubtractFloat64VectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
            {
                uint32_t elementIndex = 0;

                for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                {
                    const __m128d targetValues = LoadFloat64x2(_target, elementIndex);
                    const __m128d sourceValues = LoadFloat64x2(_source, elementIndex);
                    StoreFloat64x2(_target, _mm_sub_pd(targetValues, sourceValues), elementIndex);
                }

                for (; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, _target.getElement(elementIndex) - _source.getElement(elementIndex));
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& MultiplyFloat64VectorElements(VectorType& _target, double _value, uint32_t _elementCount)
            {
                uint32_t elementIndex = 0;

                for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                {
                    const __m128d targetValues = LoadFloat64x2(_target, elementIndex);
                    const __m128d scalarValues = _mm_set1_pd(_value);
                    StoreFloat64x2(_target, _mm_mul_pd(targetValues, scalarValues), elementIndex);
                }

                for (; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, _target.getElement(elementIndex) * _value);
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& MultiplyFloat64VectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
            {
                uint32_t elementIndex = 0;

                for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                {
                    const __m128d targetValues = LoadFloat64x2(_target, elementIndex);
                    const __m128d sourceValues = LoadFloat64x2(_source, elementIndex);
                    StoreFloat64x2(_target, _mm_mul_pd(targetValues, sourceValues), elementIndex);
                }

                for (; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, _target.getElement(elementIndex) * _source.getElement(elementIndex));
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& DivideFloat64VectorElements(VectorType& _target, double _value, uint32_t _elementCount)
            {
                uint32_t elementIndex = 0;

                for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                {
                    const __m128d targetValues = LoadFloat64x2(_target, elementIndex);
                    const __m128d scalarValues = _mm_set1_pd(_value);
                    StoreFloat64x2(_target, _mm_div_pd(targetValues, scalarValues), elementIndex);
                }

                for (; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, _target.getElement(elementIndex) / _value);
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& NegateFloat64VectorElements(VectorType& _target, uint32_t _elementCount)
            {
                uint32_t elementIndex = 0;

                for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                {
                    const __m128d targetValues = LoadFloat64x2(_target, elementIndex);
                    StoreFloat64x2(_target, _mm_sub_pd(_mm_setzero_pd(), targetValues), elementIndex);
                }

                for (; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, -_target.getElement(elementIndex));
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& ReciprocalFloat64VectorElements(VectorType& _target, uint32_t _elementCount)
            {
                uint32_t elementIndex = 0;

                for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                {
                    const __m128d targetValues = LoadFloat64x2(_target, elementIndex);
                    StoreFloat64x2(_target, _mm_div_pd(_mm_set1_pd(1.0), targetValues), elementIndex);
                }

                for (; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, 1.0 / _target.getElement(elementIndex));
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& AbsFloat64VectorElements(VectorType& _target, uint32_t _elementCount)
            {
                uint32_t elementIndex = 0;

                for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                {
                    const __m128d targetValues = LoadFloat64x2(_target, elementIndex);
                    StoreFloat64x2(_target, AbsFloat64x2(targetValues), elementIndex);
                }

                for (; elementIndex < _elementCount; ++elementIndex)
                {
                    const double value = _target.getElement(elementIndex);
                    _target.setElement(elementIndex, value < 0.0 ? -value : value);
                }

                return _target;
            }

            template <typename VectorType>
            typename VectorType::ValueType DotFloat64VectorElements(const VectorType& _vector1, const VectorType& _vector2, uint32_t _elementCount)
            {
                uint32_t elementIndex = 0;
                __m128d values = _mm_setzero_pd();

                for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                {
                    const __m128d vector1Values = LoadFloat64x2(_vector1, elementIndex);
                    const __m128d vector2Values = LoadFloat64x2(_vector2, elementIndex);
                    values = _mm_add_pd(values, _mm_mul_pd(vector1Values, vector2Values));
                }

                double value = SumFloat64x2(values);

                for (; elementIndex < _elementCount; ++elementIndex)
                {
                    value += _vector1.getElement(elementIndex) * _vector2.getElement(elementIndex);
                }

                return value;
            }

            template <typename VectorType>
            bool TryCompareVectorElementsSimd(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount,
                VectorCompareOperation _operation,
                bool _allElements,
                bool& _out)
            {
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    if (_elementCount <= Float32x4ElementCount)
                    {
                        const __m128 vector1Values = LoadFloat32x4(_vector1, _elementCount);
                        const __m128 vector2Values = LoadFloat32x4(_vector2, _elementCount);
                        const __m128 epsilonValues = _mm_set1_ps(_epsilon);
                        const uint32_t activeMask = MakeElementMask(_elementCount);
                        const uint32_t compareMask = static_cast<uint32_t>(
                            _mm_movemask_ps(CompareFloat32x4(vector1Values, vector2Values, epsilonValues, _operation)));

                        _out = _allElements ? (compareMask & activeMask) == activeMask : (compareMask & activeMask) != 0;
                        return true;
                    }
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    uint32_t elementIndex = 0;

                    for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                    {
                        const __m128d vector1Values = LoadFloat64x2(_vector1, elementIndex);
                        const __m128d vector2Values = LoadFloat64x2(_vector2, elementIndex);
                        const __m128d epsilonValues = _mm_set1_pd(_epsilon);
                        const uint32_t compareMask = static_cast<uint32_t>(
                            _mm_movemask_pd(CompareFloat64x2(vector1Values, vector2Values, epsilonValues, _operation)));

                        if (_allElements)
                        {
                            if ((compareMask & MakeElementMask(Float64x2ElementCount)) != MakeElementMask(Float64x2ElementCount))
                            {
                                _out = false;
                                return true;
                            }
                        }
                        else if ((compareMask & MakeElementMask(Float64x2ElementCount)) != 0)
                        {
                            _out = true;
                            return true;
                        }
                    }

                    for (; elementIndex < _elementCount; ++elementIndex)
                    {
                        const bool isMatch = CompareScalarElement(
                            _vector1.getElement(elementIndex),
                            _vector2.getElement(elementIndex),
                            _epsilon,
                            _operation);

                        if (_allElements)
                        {
                            if (!isMatch)
                            {
                                _out = false;
                                return true;
                            }
                        }
                        else if (isMatch)
                        {
                            _out = true;
                            return true;
                        }
                    }

                    _out = _allElements;
                    return true;
                }

                return false;
            }
#endif
            template <typename VectorType>
            bool TryAddVectorElementsSimd(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
            {
#if EGO_MATH_SIMD_SSE2
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    if (_elementCount <= Float32x4ElementCount)
                    {
                        const __m128 targetValues = LoadFloat32x4(_target, _elementCount);
                        const __m128 sourceValues = LoadFloat32x4(_source, _elementCount);
                        StoreFloat32x4(_target, _mm_add_ps(targetValues, sourceValues), _elementCount);

                        return true;
                    }
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    AddFloat64VectorElements(_target, _source, _elementCount);
                    return true;
                }
#endif

                return false;
            }

            template <typename VectorType>
            bool TrySubtractVectorElementsSimd(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
            {
#if EGO_MATH_SIMD_SSE2
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    if (_elementCount <= Float32x4ElementCount)
                    {
                        const __m128 targetValues = LoadFloat32x4(_target, _elementCount);
                        const __m128 sourceValues = LoadFloat32x4(_source, _elementCount);
                        StoreFloat32x4(_target, _mm_sub_ps(targetValues, sourceValues), _elementCount);

                        return true;
                    }
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    SubtractFloat64VectorElements(_target, _source, _elementCount);
                    return true;
                }
#endif

                return false;
            }

            template <typename VectorType>
            bool TryMultiplyVectorElementsSimd(VectorType& _target, typename VectorType::ValueType _value, uint32_t _elementCount)
            {
#if EGO_MATH_SIMD_SSE2
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    if (_elementCount <= Float32x4ElementCount)
                    {
                        const __m128 targetValues = LoadFloat32x4(_target, _elementCount);
                        const __m128 scalarValues = _mm_set1_ps(_value);
                        StoreFloat32x4(_target, _mm_mul_ps(targetValues, scalarValues), _elementCount);

                        return true;
                    }
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    MultiplyFloat64VectorElements(_target, _value, _elementCount);
                    return true;
                }
#endif

                return false;
            }

            template <typename VectorType>
            bool TryMultiplyVectorElementsSimd(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
            {
#if EGO_MATH_SIMD_SSE2
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    if (_elementCount <= Float32x4ElementCount)
                    {
                        const __m128 targetValues = LoadFloat32x4(_target, _elementCount);
                        const __m128 sourceValues = LoadFloat32x4(_source, _elementCount);
                        StoreFloat32x4(_target, _mm_mul_ps(targetValues, sourceValues), _elementCount);

                        return true;
                    }
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    MultiplyFloat64VectorElements(_target, _source, _elementCount);
                    return true;
                }
#endif

                return false;
            }

            template <typename VectorType>
            bool TryDivideVectorElementsSimd(VectorType& _target, typename VectorType::ValueType _value, uint32_t _elementCount)
            {
#if EGO_MATH_SIMD_SSE2
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    if (_elementCount <= Float32x4ElementCount)
                    {
                        const __m128 targetValues = LoadFloat32x4(_target, _elementCount);
                        const __m128 scalarValues = _mm_set1_ps(_value);
                        StoreFloat32x4(_target, _mm_div_ps(targetValues, scalarValues), _elementCount);

                        return true;
                    }
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    DivideFloat64VectorElements(_target, _value, _elementCount);
                    return true;
                }
#endif

                return false;
            }

            template <typename VectorType>
            bool TryNegateVectorElementsSimd(VectorType& _target, uint32_t _elementCount)
            {
#if EGO_MATH_SIMD_SSE2
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    if (_elementCount <= Float32x4ElementCount)
                    {
                        const __m128 targetValues = LoadFloat32x4(_target, _elementCount);
                        StoreFloat32x4(_target, _mm_sub_ps(_mm_setzero_ps(), targetValues), _elementCount);

                        return true;
                    }
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    NegateFloat64VectorElements(_target, _elementCount);
                    return true;
                }
#endif

                return false;
            }

            template <typename VectorType>
            bool TryReciprocalVectorElementsSimd(VectorType& _target, uint32_t _elementCount)
            {
#if EGO_MATH_SIMD_SSE2
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    if (_elementCount <= Float32x4ElementCount)
                    {
                        const __m128 targetValues = LoadFloat32x4(_target, _elementCount);
                        const __m128 reciprocalValues = _mm_div_ps(_mm_set1_ps(1.0f), targetValues);

                        if (_elementCount == Float32x4ElementCount)
                        {
                            StoreFloat32x4(_target, reciprocalValues, _elementCount);
                        }
                        else
                        {
                            alignas(16) float values[Float32x4ElementCount];
                            _mm_store_ps(values, reciprocalValues);

                            for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                            {
                                _target.setElement(elementIndex, values[elementIndex]);
                            }
                        }

                        return true;
                    }
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    ReciprocalFloat64VectorElements(_target, _elementCount);
                    return true;
                }
#endif

                return false;
            }

            template <typename VectorType>
            bool TryAbsVectorElementsSimd(VectorType& _target, uint32_t _elementCount)
            {
#if EGO_MATH_SIMD_SSE2
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    if (_elementCount <= Float32x4ElementCount)
                    {
                        const __m128 targetValues = LoadFloat32x4(_target, _elementCount);
                        StoreFloat32x4(_target, AbsFloat32x4(targetValues), _elementCount);

                        return true;
                    }
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    AbsFloat64VectorElements(_target, _elementCount);
                    return true;
                }
#endif

                return false;
            }

            template <typename VectorType>
            bool TryDotVectorElementsSimd(
                const VectorType& _vector1,
                const VectorType& _vector2,
                uint32_t _elementCount,
                typename VectorType::ValueType& _out)
            {
#if EGO_MATH_SIMD_SSE2
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    if (_elementCount <= Float32x4ElementCount)
                    {
                        const __m128 vector1Values = LoadFloat32x4(_vector1, _elementCount);
                        const __m128 vector2Values = LoadFloat32x4(_vector2, _elementCount);
                        _out = SumFloat32x4(_mm_mul_ps(vector1Values, vector2Values), _elementCount);

                        return true;
                    }
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    _out = DotFloat64VectorElements(_vector1, _vector2, _elementCount);
                    return true;
                }
#endif

                return false;
            }

            template <typename VectorType>
            bool TryCrossVector3ElementsSimd(const VectorType& _vector1, const VectorType& _vector2, VectorType& _out)
            {
#if EGO_MATH_SIMD_SSE2
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    const __m128 vector1Values = LoadFloat32x4(_vector1, 3);
                    const __m128 vector2Values = LoadFloat32x4(_vector2, 3);
                    const __m128 vector1YzxValues = _mm_shuffle_ps(vector1Values, vector1Values, _MM_SHUFFLE(3, 0, 2, 1));
                    const __m128 vector1ZxyValues = _mm_shuffle_ps(vector1Values, vector1Values, _MM_SHUFFLE(3, 1, 0, 2));
                    const __m128 vector2YzxValues = _mm_shuffle_ps(vector2Values, vector2Values, _MM_SHUFFLE(3, 0, 2, 1));
                    const __m128 vector2ZxyValues = _mm_shuffle_ps(vector2Values, vector2Values, _MM_SHUFFLE(3, 1, 0, 2));

                    StoreFloat32x4(
                        _out,
                        _mm_sub_ps(_mm_mul_ps(vector1YzxValues, vector2ZxyValues), _mm_mul_ps(vector1ZxyValues, vector2YzxValues)),
                        3);
                    return true;
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    const __m128d vector1LowValues = LoadFloat64x2(_vector1, 0);
                    const __m128d vector2LowValues = LoadFloat64x2(_vector2, 0);
                    const __m128d vector1YxValues = _mm_shuffle_pd(vector1LowValues, vector1LowValues, 1);
                    const __m128d vector2YxValues = _mm_shuffle_pd(vector2LowValues, vector2LowValues, 1);
                    const __m128d vector1ZValues = _mm_set1_pd(_vector1.getElement(2));
                    const __m128d vector2ZValues = _mm_set1_pd(_vector2.getElement(2));
                    const __m128d xyValues = _mm_xor_pd(
                        _mm_sub_pd(_mm_mul_pd(vector1YxValues, vector2ZValues), _mm_mul_pd(vector1ZValues, vector2YxValues)),
                        _mm_set_pd(-0.0, 0.0));

                    alignas(16) double zValues[Float64x2ElementCount];
                    _mm_store_pd(zValues, _mm_mul_pd(vector1LowValues, vector2YxValues));

                    StoreFloat64x2(_out, xyValues, 0);
                    _out.setElement(2, zValues[0] - zValues[1]);
                    return true;
                }
#endif

                return false;
            }

            template <typename VectorType>
            bool TryAreVectorElementsEqualSimd(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount,
                bool& _out)
            {
#if EGO_MATH_SIMD_SSE2
                return TryCompareVectorElementsSimd(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Equal, true, _out);
#else
                return false;
#endif
            }

            template <typename VectorType>
            bool TryAreVectorElementsLessSimd(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount,
                bool& _out)
            {
#if EGO_MATH_SIMD_SSE2
                return TryCompareVectorElementsSimd(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Less, true, _out);
#else
                return false;
#endif
            }

            template <typename VectorType>
            bool TryAreVectorElementsLessOrEqualSimd(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount,
                bool& _out)
            {
#if EGO_MATH_SIMD_SSE2
                return TryCompareVectorElementsSimd(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::LessOrEqual, true, _out);
#else
                return false;
#endif
            }

            template <typename VectorType>
            bool TryAreVectorElementsGreaterSimd(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount,
                bool& _out)
            {
#if EGO_MATH_SIMD_SSE2
                return TryCompareVectorElementsSimd(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Greater, true, _out);
#else
                return false;
#endif
            }

            template <typename VectorType>
            bool TryAreVectorElementsGreaterOrEqualSimd(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount,
                bool& _out)
            {
#if EGO_MATH_SIMD_SSE2
                return TryCompareVectorElementsSimd(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::GreaterOrEqual, true, _out);
#else
                return false;
#endif
            }

            template <typename VectorType>
            bool TryIsAnyVectorElementLessSimd(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount,
                bool& _out)
            {
#if EGO_MATH_SIMD_SSE2
                return TryCompareVectorElementsSimd(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Less, false, _out);
#else
                return false;
#endif
            }

            template <typename VectorType>
            bool TryIsAnyVectorElementLessOrEqualSimd(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount,
                bool& _out)
            {
#if EGO_MATH_SIMD_SSE2
                return TryCompareVectorElementsSimd(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::LessOrEqual, false, _out);
#else
                return false;
#endif
            }

            template <typename VectorType>
            bool TryIsAnyVectorElementGreaterSimd(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount,
                bool& _out)
            {
#if EGO_MATH_SIMD_SSE2
                return TryCompareVectorElementsSimd(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Greater, false, _out);
#else
                return false;
#endif
            }

            template <typename VectorType>
            bool TryIsAnyVectorElementGreaterOrEqualSimd(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount,
                bool& _out)
            {
#if EGO_MATH_SIMD_SSE2
                return TryCompareVectorElementsSimd(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::GreaterOrEqual, false, _out);
#else
                return false;
#endif
            }

            template <typename VectorType>
            bool TryTransformMatrix2x2VectorSimd(
                const VectorType& _column0,
                const VectorType& _column1,
                const VectorType& _vector,
                VectorType& _out)
            {
#if EGO_MATH_SIMD_SSE2
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    const __m128 column0Values = LoadFloat32x4(_column0, 2);
                    const __m128 column1Values = LoadFloat32x4(_column1, 2);
                    const __m128 resultValues = _mm_add_ps(
                        _mm_mul_ps(column0Values, _mm_set1_ps(_vector.getElement(0))),
                        _mm_mul_ps(column1Values, _mm_set1_ps(_vector.getElement(1))));

                    StoreFloat32x4(_out, resultValues, 2);
                    return true;
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    const __m128d column0Values = LoadFloat64x2(_column0, 0);
                    const __m128d column1Values = LoadFloat64x2(_column1, 0);
                    const __m128d resultValues = _mm_add_pd(
                        _mm_mul_pd(column0Values, _mm_set1_pd(_vector.getElement(0))),
                        _mm_mul_pd(column1Values, _mm_set1_pd(_vector.getElement(1))));

                    StoreFloat64x2(_out, resultValues, 0);
                    return true;
                }
#endif

                return false;
            }

            template <typename VectorType>
            bool TryTransformMatrix3x3VectorSimd(
                const VectorType& _column0,
                const VectorType& _column1,
                const VectorType& _column2,
                const VectorType& _vector,
                VectorType& _out)
            {
#if EGO_MATH_SIMD_SSE2
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    const __m128 column0Values = LoadFloat32x4(_column0, 3);
                    const __m128 column1Values = LoadFloat32x4(_column1, 3);
                    const __m128 column2Values = LoadFloat32x4(_column2, 3);
                    const __m128 resultValues = _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(column0Values, _mm_set1_ps(_vector.getElement(0))),
                            _mm_mul_ps(column1Values, _mm_set1_ps(_vector.getElement(1)))),
                        _mm_mul_ps(column2Values, _mm_set1_ps(_vector.getElement(2))));

                    StoreFloat32x4(_out, resultValues, 3);
                    return true;
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    const __m128d column0Values = LoadFloat64x2(_column0, 0);
                    const __m128d column1Values = LoadFloat64x2(_column1, 0);
                    const __m128d column2Values = LoadFloat64x2(_column2, 0);
                    const __m128d resultValues = _mm_add_pd(
                        _mm_add_pd(
                            _mm_mul_pd(column0Values, _mm_set1_pd(_vector.getElement(0))),
                            _mm_mul_pd(column1Values, _mm_set1_pd(_vector.getElement(1)))),
                        _mm_mul_pd(column2Values, _mm_set1_pd(_vector.getElement(2))));

                    StoreFloat64x2(_out, resultValues, 0);
                    _out.setElement(
                        2,
                        _column0.getElement(2) * _vector.getElement(0) + _column1.getElement(2) * _vector.getElement(1) +
                            _column2.getElement(2) * _vector.getElement(2));
                    return true;
                }
#endif

                return false;
            }

            template <typename VectorType>
            bool TryTransformMatrix4x4VectorSimd(
                const VectorType& _column0,
                const VectorType& _column1,
                const VectorType& _column2,
                const VectorType& _column3,
                const VectorType& _vector,
                VectorType& _out)
            {
#if EGO_MATH_SIMD_SSE2
                if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                {
                    const __m128 column0Values = LoadFloat32x4(_column0, 4);
                    const __m128 column1Values = LoadFloat32x4(_column1, 4);
                    const __m128 column2Values = LoadFloat32x4(_column2, 4);
                    const __m128 column3Values = LoadFloat32x4(_column3, 4);

                    const __m128 resultValues = _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(column0Values, _mm_set1_ps(_vector.getElement(0))),
                            _mm_mul_ps(column1Values, _mm_set1_ps(_vector.getElement(1)))),
                        _mm_add_ps(
                            _mm_mul_ps(column2Values, _mm_set1_ps(_vector.getElement(2))),
                            _mm_mul_ps(column3Values, _mm_set1_ps(_vector.getElement(3)))));

                    StoreFloat32x4(_out, resultValues, 4);
                    return true;
                }
                else if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                {
                    const __m128d column0LowValues = LoadFloat64x2(_column0, 0);
                    const __m128d column1LowValues = LoadFloat64x2(_column1, 0);
                    const __m128d column2LowValues = LoadFloat64x2(_column2, 0);
                    const __m128d column3LowValues = LoadFloat64x2(_column3, 0);
                    const __m128d lowValues = _mm_add_pd(
                        _mm_add_pd(
                            _mm_mul_pd(column0LowValues, _mm_set1_pd(_vector.getElement(0))),
                            _mm_mul_pd(column1LowValues, _mm_set1_pd(_vector.getElement(1)))),
                        _mm_add_pd(
                            _mm_mul_pd(column2LowValues, _mm_set1_pd(_vector.getElement(2))),
                            _mm_mul_pd(column3LowValues, _mm_set1_pd(_vector.getElement(3)))));

                    const __m128d column0HighValues = LoadFloat64x2(_column0, 2);
                    const __m128d column1HighValues = LoadFloat64x2(_column1, 2);
                    const __m128d column2HighValues = LoadFloat64x2(_column2, 2);
                    const __m128d column3HighValues = LoadFloat64x2(_column3, 2);
                    const __m128d highValues = _mm_add_pd(
                        _mm_add_pd(
                            _mm_mul_pd(column0HighValues, _mm_set1_pd(_vector.getElement(0))),
                            _mm_mul_pd(column1HighValues, _mm_set1_pd(_vector.getElement(1)))),
                        _mm_add_pd(
                            _mm_mul_pd(column2HighValues, _mm_set1_pd(_vector.getElement(2))),
                            _mm_mul_pd(column3HighValues, _mm_set1_pd(_vector.getElement(3)))));

                    StoreFloat64x2(_out, lowValues, 0);
                    StoreFloat64x2(_out, highValues, 2);
                    return true;
                }
#endif

                return false;
            }

            template <typename MatrixType>
            bool TryDotDynamicMatrixRowColumnSimd(
                const MatrixType& _matrix1,
                const MatrixType& _matrix2,
                uint32_t _rowIndex,
                uint32_t _columnIndex,
                uint32_t _elementCount,
                typename MatrixType::ValueType& _out)
            {
#if EGO_MATH_SIMD_SSE2
                using ValueType = typename MatrixType::ValueType;
                uint32_t elementIndex = 0;

                if constexpr (std::is_same_v<ValueType, float>)
                {
                    __m128 values = _mm_setzero_ps();
                    for (; elementIndex + Float32x4ElementCount <= _elementCount; elementIndex += Float32x4ElementCount)
                    {
                        const __m128 rowValues = _mm_set_ps(
                            _matrix1.getElement(_rowIndex, elementIndex + 3),
                            _matrix1.getElement(_rowIndex, elementIndex + 2),
                            _matrix1.getElement(_rowIndex, elementIndex + 1),
                            _matrix1.getElement(_rowIndex, elementIndex));
                        const __m128 columnValues = _mm_set_ps(
                            _matrix2.getElement(elementIndex + 3, _columnIndex),
                            _matrix2.getElement(elementIndex + 2, _columnIndex),
                            _matrix2.getElement(elementIndex + 1, _columnIndex),
                            _matrix2.getElement(elementIndex, _columnIndex));

                        values = _mm_add_ps(values, _mm_mul_ps(rowValues, columnValues));
                    }

                    _out = SumFloat32x4(values, Float32x4ElementCount);
                }
                else if constexpr (std::is_same_v<ValueType, double>)
                {
                    __m128d values = _mm_setzero_pd();
                    for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                    {
                        const __m128d rowValues = _mm_set_pd(
                            _matrix1.getElement(_rowIndex, elementIndex + 1),
                            _matrix1.getElement(_rowIndex, elementIndex));
                        const __m128d columnValues = _mm_set_pd(
                            _matrix2.getElement(elementIndex + 1, _columnIndex),
                            _matrix2.getElement(elementIndex, _columnIndex));

                        values = _mm_add_pd(values, _mm_mul_pd(rowValues, columnValues));
                    }

                    _out = SumFloat64x2(values);
                }
                else
                {
                    return false;
                }

                for (; elementIndex < _elementCount; ++elementIndex)
                {
                    _out += _matrix1.getElement(_rowIndex, elementIndex) * _matrix2.getElement(elementIndex, _columnIndex);
                }

                return true;
#else
                return false;
#endif
            }

            template <typename MatrixType, typename VectorType>
            bool TryDotDynamicMatrixRowVectorSimd(
                const MatrixType& _matrix,
                const VectorType& _vector,
                uint32_t _rowIndex,
                uint32_t _elementCount,
                typename MatrixType::ValueType& _out)
            {
#if EGO_MATH_SIMD_SSE2
                using ValueType = typename MatrixType::ValueType;
                uint32_t elementIndex = 0;

                if constexpr (std::is_same_v<ValueType, float> && std::is_same_v<typename VectorType::ValueType, float>)
                {
                    __m128 values = _mm_setzero_ps();
                    for (; elementIndex + Float32x4ElementCount <= _elementCount; elementIndex += Float32x4ElementCount)
                    {
                        const __m128 rowValues = _mm_set_ps(
                            _matrix.getElement(_rowIndex, elementIndex + 3),
                            _matrix.getElement(_rowIndex, elementIndex + 2),
                            _matrix.getElement(_rowIndex, elementIndex + 1),
                            _matrix.getElement(_rowIndex, elementIndex));
                        const __m128 vectorValues = _mm_set_ps(
                            _vector.getElement(elementIndex + 3),
                            _vector.getElement(elementIndex + 2),
                            _vector.getElement(elementIndex + 1),
                            _vector.getElement(elementIndex));

                        values = _mm_add_ps(values, _mm_mul_ps(rowValues, vectorValues));
                    }

                    _out = SumFloat32x4(values, Float32x4ElementCount);
                }
                else if constexpr (std::is_same_v<ValueType, double> && std::is_same_v<typename VectorType::ValueType, double>)
                {
                    __m128d values = _mm_setzero_pd();
                    for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                    {
                        const __m128d rowValues = _mm_set_pd(
                            _matrix.getElement(_rowIndex, elementIndex + 1),
                            _matrix.getElement(_rowIndex, elementIndex));
                        const __m128d vectorValues = _mm_set_pd(_vector.getElement(elementIndex + 1), _vector.getElement(elementIndex));

                        values = _mm_add_pd(values, _mm_mul_pd(rowValues, vectorValues));
                    }

                    _out = SumFloat64x2(values);
                }
                else
                {
                    return false;
                }

                for (; elementIndex < _elementCount; ++elementIndex)
                {
                    _out += _matrix.getElement(_rowIndex, elementIndex) * _vector.getElement(elementIndex);
                }

                return true;
#else
                return false;
#endif
            }
        } // namespace simd
    } // namespace math
} // namespace ego
