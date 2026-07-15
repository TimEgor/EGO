#pragma once

#include <cmath>
#include <cstdint>
#include <type_traits>

#include "ComputeNativeStorage.h"
#include "ComputeScalar.h"

namespace ego
{
    namespace math
    {
        namespace simd
        {
            namespace sse
            {
                inline __m128 LoadNativeFloat32x4(const NativeComputeVectorStorage<float>& _values)
                {
                    return _values.m_value;
                }

                inline void StoreNativeFloat32x4(NativeComputeVectorStorage<float>& _target, __m128 _values)
                {
                    _target.m_value = _values;
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

                template <typename VectorType>
                bool CompareVectorElements(
                    const VectorType& _vector1,
                    const VectorType& _vector2,
                    typename VectorType::ValueType _epsilon,
                    uint32_t _elementCount,
                    VectorCompareOperation _operation,
                    bool _allElements)
                {
                    const __m128 vector1Values = LoadFloat32x4(_vector1, _elementCount);
                    const __m128 vector2Values = LoadFloat32x4(_vector2, _elementCount);
                    const __m128 epsilonValues = _mm_set1_ps(_epsilon);
                    const uint32_t activeMask = MakeElementMask(_elementCount);
                    const uint32_t compareMask = static_cast<uint32_t>(_mm_movemask_ps(CompareFloat32x4(vector1Values, vector2Values, epsilonValues, _operation)));

                    return _allElements ? (compareMask & activeMask) == activeMask : (compareMask & activeMask) != 0;
                }

                template <typename VectorType>
                VectorType& AddVectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        StoreFloat32x4(_target, _mm_add_ps(LoadFloat32x4(_target, _elementCount), LoadFloat32x4(_source, _elementCount)), _elementCount);
                        return _target;
                    }

                    return scalar::AddVectorElements(_target, _source, _elementCount);
                }

                template <typename VectorType>
                VectorType& SubtractVectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        StoreFloat32x4(_target, _mm_sub_ps(LoadFloat32x4(_target, _elementCount), LoadFloat32x4(_source, _elementCount)), _elementCount);
                        return _target;
                    }

                    return scalar::SubtractVectorElements(_target, _source, _elementCount);
                }

                template <typename VectorType>
                VectorType& MultiplyVectorElements(VectorType& _target, typename VectorType::ValueType _value, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        StoreFloat32x4(_target, _mm_mul_ps(LoadFloat32x4(_target, _elementCount), _mm_set1_ps(_value)), _elementCount);
                        return _target;
                    }

                    return scalar::MultiplyVectorElements(_target, _value, _elementCount);
                }

                template <typename VectorType>
                VectorType& MultiplyVectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        StoreFloat32x4(_target, _mm_mul_ps(LoadFloat32x4(_target, _elementCount), LoadFloat32x4(_source, _elementCount)), _elementCount);
                        return _target;
                    }

                    return scalar::MultiplyVectorElements(_target, _source, _elementCount);
                }

                template <typename VectorType>
                VectorType& DivideVectorElements(VectorType& _target, typename VectorType::ValueType _value, uint32_t _elementCount)
                {
                    EGO_ASSERT(std::abs(_value) > TypedEpsilon<typename VectorType::ValueType>());
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        StoreFloat32x4(_target, _mm_div_ps(LoadFloat32x4(_target, _elementCount), _mm_set1_ps(_value)), _elementCount);
                        return _target;
                    }

                    return scalar::DivideVectorElements(_target, _value, _elementCount);
                }

                template <typename VectorType>
                VectorType& NegateVectorElements(VectorType& _target, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        StoreFloat32x4(_target, _mm_sub_ps(_mm_setzero_ps(), LoadFloat32x4(_target, _elementCount)), _elementCount);
                        return _target;
                    }

                    return scalar::NegateVectorElements(_target, _elementCount);
                }

                template <typename VectorType>
                VectorType& ReciprocalVectorElements(VectorType& _target, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        StoreFloat32x4(_target, _mm_div_ps(_mm_set1_ps(1.0f), LoadFloat32x4(_target, _elementCount)), _elementCount);
                        return _target;
                    }

                    return scalar::ReciprocalVectorElements(_target, _elementCount);
                }

                template <typename VectorType>
                VectorType& AbsVectorElements(VectorType& _target, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        StoreFloat32x4(_target, AbsFloat32x4(LoadFloat32x4(_target, _elementCount)), _elementCount);
                        return _target;
                    }

                    return scalar::AbsVectorElements(_target, _elementCount);
                }

                template <typename VectorType>
                typename VectorType::ValueType DotVectorElements(const VectorType& _vector1, const VectorType& _vector2, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        return SumFloat32x4(_mm_mul_ps(LoadFloat32x4(_vector1, _elementCount), LoadFloat32x4(_vector2, _elementCount)), _elementCount);
                    }

                    return scalar::DotVectorElements(_vector1, _vector2, _elementCount);
                }

                template <typename VectorType>
                void CrossVector3Elements(const VectorType& _vector1, const VectorType& _vector2, VectorType& _out)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        const __m128 vector1Values = LoadFloat32x4(_vector1, 3);
                        const __m128 vector2Values = LoadFloat32x4(_vector2, 3);
                        const __m128 vector1YzxValues = _mm_shuffle_ps(vector1Values, vector1Values, _MM_SHUFFLE(3, 0, 2, 1));
                        const __m128 vector1ZxyValues = _mm_shuffle_ps(vector1Values, vector1Values, _MM_SHUFFLE(3, 1, 0, 2));
                        const __m128 vector2YzxValues = _mm_shuffle_ps(vector2Values, vector2Values, _MM_SHUFFLE(3, 0, 2, 1));
                        const __m128 vector2ZxyValues = _mm_shuffle_ps(vector2Values, vector2Values, _MM_SHUFFLE(3, 1, 0, 2));

                        StoreFloat32x4(_out, _mm_sub_ps(_mm_mul_ps(vector1YzxValues, vector2ZxyValues), _mm_mul_ps(vector1ZxyValues, vector2YzxValues)), 3);
                        return;
                    }

                    scalar::CrossVector3Elements(_vector1, _vector2, _out);
                }

                template <typename VectorType>
                bool AreVectorElementsEqual(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        return CompareVectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Equal, true);
                    }

                    return scalar::AreVectorElementsEqual(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool AreVectorElementsLess(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        return CompareVectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Less, true);
                    }

                    return scalar::AreVectorElementsLess(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool AreVectorElementsLessOrEqual(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        return CompareVectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::LessOrEqual, true);
                    }

                    return scalar::AreVectorElementsLessOrEqual(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool AreVectorElementsGreater(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        return CompareVectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Greater, true);
                    }

                    return scalar::AreVectorElementsGreater(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool AreVectorElementsGreaterOrEqual(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        return CompareVectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::GreaterOrEqual, true);
                    }

                    return scalar::AreVectorElementsGreaterOrEqual(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool IsAnyVectorElementLess(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        return CompareVectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Less, false);
                    }

                    return scalar::IsAnyVectorElementLess(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool IsAnyVectorElementLessOrEqual(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        return CompareVectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::LessOrEqual, false);
                    }

                    return scalar::IsAnyVectorElementLessOrEqual(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool IsAnyVectorElementGreater(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        return CompareVectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Greater, false);
                    }

                    return scalar::IsAnyVectorElementGreater(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool IsAnyVectorElementGreaterOrEqual(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        return CompareVectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::GreaterOrEqual, false);
                    }

                    return scalar::IsAnyVectorElementGreaterOrEqual(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                void TransformMatrix2x2VectorElements(const VectorType& _column0, const VectorType& _column1, const VectorType& _vector, VectorType& _out)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        const __m128 resultValues = _mm_add_ps(
                            _mm_mul_ps(LoadFloat32x4(_column0, 2), _mm_set1_ps(_vector.getElement(0))),
                            _mm_mul_ps(LoadFloat32x4(_column1, 2), _mm_set1_ps(_vector.getElement(1))));

                        StoreFloat32x4(_out, resultValues, 2);
                        return;
                    }

                    scalar::TransformMatrix2x2VectorElements(_column0, _column1, _vector, _out);
                }

                template <typename VectorType>
                void TransformMatrix3x3VectorElements(
                    const VectorType& _column0,
                    const VectorType& _column1,
                    const VectorType& _column2,
                    const VectorType& _vector,
                    VectorType& _out)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        const __m128 resultValues = _mm_add_ps(
                            _mm_add_ps(_mm_mul_ps(LoadFloat32x4(_column0, 3), _mm_set1_ps(_vector.getElement(0))), _mm_mul_ps(LoadFloat32x4(_column1, 3), _mm_set1_ps(_vector.getElement(1)))),
                            _mm_mul_ps(LoadFloat32x4(_column2, 3), _mm_set1_ps(_vector.getElement(2))));

                        StoreFloat32x4(_out, resultValues, 3);
                        return;
                    }

                    scalar::TransformMatrix3x3VectorElements(_column0, _column1, _column2, _vector, _out);
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
                    if constexpr (std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        const __m128 resultValues = _mm_add_ps(
                            _mm_add_ps(_mm_mul_ps(LoadFloat32x4(_column0, 4), _mm_set1_ps(_vector.getElement(0))), _mm_mul_ps(LoadFloat32x4(_column1, 4), _mm_set1_ps(_vector.getElement(1)))),
                            _mm_add_ps(_mm_mul_ps(LoadFloat32x4(_column2, 4), _mm_set1_ps(_vector.getElement(2))), _mm_mul_ps(LoadFloat32x4(_column3, 4), _mm_set1_ps(_vector.getElement(3)))));

                        StoreFloat32x4(_out, resultValues, 4);
                        return;
                    }

                    scalar::TransformMatrix4x4VectorElements(_column0, _column1, _column2, _column3, _vector, _out);
                }

                template <typename MatrixType>
                typename MatrixType::ValueType DotDynamicMatrixRowColumnElements(
                    const MatrixType& _matrix1,
                    const MatrixType& _matrix2,
                    uint32_t _rowIndex,
                    uint32_t _columnIndex,
                    uint32_t _elementCount)
                {
                    using ValueType = typename MatrixType::ValueType;
                    if constexpr (std::is_same_v<ValueType, float>)
                    {
                        uint32_t elementIndex = 0;
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

                        ValueType result = SumFloat32x4(values, Float32x4ElementCount);
                        for (; elementIndex < _elementCount; ++elementIndex)
                        {
                            result += _matrix1.getElement(_rowIndex, elementIndex) * _matrix2.getElement(elementIndex, _columnIndex);
                        }

                        return result;
                    }

                    return scalar::DotDynamicMatrixRowColumnElements(_matrix1, _matrix2, _rowIndex, _columnIndex, _elementCount);
                }

                template <typename MatrixType, typename VectorType>
                typename MatrixType::ValueType DotDynamicMatrixRowVectorElements(const MatrixType& _matrix, const VectorType& _vector, uint32_t _rowIndex, uint32_t _elementCount)
                {
                    using ValueType = typename MatrixType::ValueType;
                    if constexpr (std::is_same_v<ValueType, float> && std::is_same_v<typename VectorType::ValueType, float>)
                    {
                        uint32_t elementIndex = 0;
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

                        ValueType result = SumFloat32x4(values, Float32x4ElementCount);
                        for (; elementIndex < _elementCount; ++elementIndex)
                        {
                            result += _matrix.getElement(_rowIndex, elementIndex) * _vector.getElement(elementIndex);
                        }

                        return result;
                    }

                    return scalar::DotDynamicMatrixRowVectorElements(_matrix, _vector, _rowIndex, _elementCount);
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

                template <typename MatrixType>
                bool AreMatrixElementsEqual(const MatrixType& _matrix1, const MatrixType& _matrix2, typename MatrixType::ValueType _epsilon, uint32_t _elementCount)
                {
                    for (uint32_t columnIndex = 0; columnIndex < _elementCount; ++columnIndex)
                    {
                        if (!AreVectorElementsEqual(_matrix1.getColumn(columnIndex), _matrix2.getColumn(columnIndex), _epsilon, _elementCount))
                        {
                            return false;
                        }
                    }

                    return true;
                }
            } // namespace sse
        } // namespace simd
    } // namespace math
} // namespace ego
