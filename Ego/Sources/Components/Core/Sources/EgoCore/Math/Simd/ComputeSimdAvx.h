#pragma once

#include "ComputeSimdSse42.h"

namespace ego
{
    namespace math
    {
        namespace simd
        {
            namespace avx
            {
                using namespace sse42;

                inline __m256d LoadNativeFloat64x4(const NativeComputeVectorStorage<double>& _values)
                {
                    return _mm256_set_m128d(_values.m_highValue, _values.m_lowValue);
                }

                inline void StoreNativeFloat64x4(NativeComputeVectorStorage<double>& _target, __m256d _values)
                {
                    _target.m_lowValue = _mm256_castpd256_pd128(_values);
                    _target.m_highValue = _mm256_extractf128_pd(_values, 1);
                }

                template <typename VectorType>
                __m256d LoadFloat64x4(const VectorType& _vector)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double> && requires { _vector.getNativeValues(); })
                    {
                        return LoadNativeFloat64x4(_vector.getNativeValues());
                    }

                    return _mm256_set_pd(_vector.getElement(3), _vector.getElement(2), _vector.getElement(1), _vector.getElement(0));
                }

                template <typename VectorType>
                void StoreFloat64x4(VectorType& _vector, __m256d _values)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double> && requires { _vector.getNativeValues(); })
                    {
                        StoreNativeFloat64x4(_vector.getNativeValues(), _values);
                        return;
                    }

                    alignas(32) double values[4];
                    _mm256_store_pd(values, _values);

                    for (uint32_t elementIndex = 0; elementIndex < 4; ++elementIndex)
                    {
                        _vector.setElement(elementIndex, values[elementIndex]);
                    }
                }

                inline __m256d AbsFloat64x4(__m256d _values)
                {
                    return _mm256_andnot_pd(_mm256_set1_pd(-0.0), _values);
                }

                template <typename VectorType>
                VectorType& AddVectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        if (_elementCount == 4)
                        {
                            StoreFloat64x4(_target, _mm256_add_pd(LoadFloat64x4(_target), LoadFloat64x4(_source)));
                            return _target;
                        }
                    }

                    return sse42::AddVectorElements(_target, _source, _elementCount);
                }

                template <typename VectorType>
                VectorType& SubtractVectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        if (_elementCount == 4)
                        {
                            StoreFloat64x4(_target, _mm256_sub_pd(LoadFloat64x4(_target), LoadFloat64x4(_source)));
                            return _target;
                        }
                    }

                    return sse42::SubtractVectorElements(_target, _source, _elementCount);
                }

                template <typename VectorType>
                VectorType& MultiplyVectorElements(VectorType& _target, typename VectorType::ValueType _value, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        if (_elementCount == 4)
                        {
                            StoreFloat64x4(_target, _mm256_mul_pd(LoadFloat64x4(_target), _mm256_set1_pd(_value)));
                            return _target;
                        }
                    }

                    return sse42::MultiplyVectorElements(_target, _value, _elementCount);
                }

                template <typename VectorType>
                VectorType& MultiplyVectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        if (_elementCount == 4)
                        {
                            StoreFloat64x4(_target, _mm256_mul_pd(LoadFloat64x4(_target), LoadFloat64x4(_source)));
                            return _target;
                        }
                    }

                    return sse42::MultiplyVectorElements(_target, _source, _elementCount);
                }

                template <typename VectorType>
                VectorType& DivideVectorElements(VectorType& _target, typename VectorType::ValueType _value, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        EGO_ASSERT(std::abs(_value) > TypedEpsilon<typename VectorType::ValueType>());
                        if (_elementCount == 4)
                        {
                            StoreFloat64x4(_target, _mm256_div_pd(LoadFloat64x4(_target), _mm256_set1_pd(_value)));
                            return _target;
                        }
                    }

                    return sse42::DivideVectorElements(_target, _value, _elementCount);
                }

                template <typename VectorType>
                VectorType& NegateVectorElements(VectorType& _target, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        if (_elementCount == 4)
                        {
                            StoreFloat64x4(_target, _mm256_sub_pd(_mm256_setzero_pd(), LoadFloat64x4(_target)));
                            return _target;
                        }
                    }

                    return sse42::NegateVectorElements(_target, _elementCount);
                }

                template <typename VectorType>
                VectorType& ReciprocalVectorElements(VectorType& _target, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        if (_elementCount == 4)
                        {
                            StoreFloat64x4(_target, _mm256_div_pd(_mm256_set1_pd(1.0), LoadFloat64x4(_target)));
                            return _target;
                        }
                    }

                    return sse42::ReciprocalVectorElements(_target, _elementCount);
                }

                template <typename VectorType>
                VectorType& AbsVectorElements(VectorType& _target, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        if (_elementCount == 4)
                        {
                            StoreFloat64x4(_target, AbsFloat64x4(LoadFloat64x4(_target)));
                            return _target;
                        }
                    }

                    return sse42::AbsVectorElements(_target, _elementCount);
                }

                template <typename VectorType>
                typename VectorType::ValueType DotVectorElements(const VectorType& _vector1, const VectorType& _vector2, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        if (_elementCount == 4)
                        {
                            const __m256d multipliedValues = _mm256_mul_pd(LoadFloat64x4(_vector1), LoadFloat64x4(_vector2));
                            const __m128d lowValues = _mm256_castpd256_pd128(multipliedValues);
                            const __m128d highValues = _mm256_extractf128_pd(multipliedValues, 1);

                            return sse2::SumFloat64x2(_mm_add_pd(lowValues, highValues));
                        }
                    }

                    return sse42::DotVectorElements(_vector1, _vector2, _elementCount);
                }

                template <typename VectorType>
                void TransformMatrix4x4VectorElements(
                    const VectorType& _column0,
                    const VectorType& _column1,
                    const VectorType& _column2,
                    const VectorType& _column3,
                    const VectorType& _vector,
                    VectorType& _out)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        const __m256d resultValues = _mm256_add_pd(
                            _mm256_add_pd(
                                _mm256_mul_pd(LoadFloat64x4(_column0), _mm256_set1_pd(_vector.getElement(0))),
                                _mm256_mul_pd(LoadFloat64x4(_column1), _mm256_set1_pd(_vector.getElement(1)))),
                            _mm256_add_pd(
                                _mm256_mul_pd(LoadFloat64x4(_column2), _mm256_set1_pd(_vector.getElement(2))),
                                _mm256_mul_pd(LoadFloat64x4(_column3), _mm256_set1_pd(_vector.getElement(3)))));

                        StoreFloat64x4(_out, resultValues);
                        return;
                    }

                    sse42::TransformMatrix4x4VectorElements(_column0, _column1, _column2, _column3, _vector, _out);
                }

                template <typename MatrixType>
                MatrixType& AddMatrixElements(MatrixType& _target, const MatrixType& _source, uint32_t _elementCount)
                {
                    for (uint32_t columnIndex = 0; columnIndex < _elementCount; ++columnIndex)
                    {
                        AddVectorElements(_target.getColumn(columnIndex), _source.getColumn(columnIndex), _elementCount);
                    }

                    return _target;
                }

                template <typename MatrixType>
                MatrixType& SubtractMatrixElements(MatrixType& _target, const MatrixType& _source, uint32_t _elementCount)
                {
                    for (uint32_t columnIndex = 0; columnIndex < _elementCount; ++columnIndex)
                    {
                        SubtractVectorElements(_target.getColumn(columnIndex), _source.getColumn(columnIndex), _elementCount);
                    }

                    return _target;
                }

                template <typename MatrixType>
                MatrixType& MultiplyMatrixElements(MatrixType& _target, typename MatrixType::ValueType _value, uint32_t _elementCount)
                {
                    for (uint32_t columnIndex = 0; columnIndex < _elementCount; ++columnIndex)
                    {
                        MultiplyVectorElements(_target.getColumn(columnIndex), _value, _elementCount);
                    }

                    return _target;
                }

                template <typename MatrixType>
                MatrixType& DivideMatrixElements(MatrixType& _target, typename MatrixType::ValueType _value, uint32_t _elementCount)
                {
                    for (uint32_t columnIndex = 0; columnIndex < _elementCount; ++columnIndex)
                    {
                        DivideVectorElements(_target.getColumn(columnIndex), _value, _elementCount);
                    }

                    return _target;
                }

                template <typename MatrixType>
                MatrixType& NegateMatrixElements(MatrixType& _target, uint32_t _elementCount)
                {
                    for (uint32_t columnIndex = 0; columnIndex < _elementCount; ++columnIndex)
                    {
                        NegateVectorElements(_target.getColumn(columnIndex), _elementCount);
                    }

                    return _target;
                }

                template <typename MatrixType>
                MatrixType& MultiplyMatrixElements(MatrixType& _target, const MatrixType& _source, uint32_t _elementCount)
                {
                    MatrixType result;

                    for (uint32_t columnIndex = 0; columnIndex < _elementCount; ++columnIndex)
                    {
                        if (_elementCount == 2)
                        {
                            TransformMatrix2x2VectorElements(_target.getColumn(0), _target.getColumn(1), _source.getColumn(columnIndex), result.getColumn(columnIndex));
                        }
                        else if (_elementCount == 3)
                        {
                            TransformMatrix3x3VectorElements(
                                _target.getColumn(0),
                                _target.getColumn(1),
                                _target.getColumn(2),
                                _source.getColumn(columnIndex),
                                result.getColumn(columnIndex));
                        }
                        else
                        {
                            TransformMatrix4x4VectorElements(
                                _target.getColumn(0),
                                _target.getColumn(1),
                                _target.getColumn(2),
                                _target.getColumn(3),
                                _source.getColumn(columnIndex),
                                result.getColumn(columnIndex));
                        }
                    }

                    _target = result;
                    return _target;
                }
            } // namespace avx
        } // namespace simd
    } // namespace math
} // namespace ego
