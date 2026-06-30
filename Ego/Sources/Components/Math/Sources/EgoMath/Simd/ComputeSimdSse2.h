#pragma once

#include <cmath>
#include <cstdint>
#include <type_traits>

#include "ComputeSimdSse.h"

namespace ego
{
    namespace math
    {
        namespace simd
        {
            namespace sse2
            {
                using sse::AbsFloat32x4;
                using sse::LoadFloat32x4;
                using sse::LoadNativeFloat32x4;
                using sse::StoreFloat32x4;
                using sse::StoreNativeFloat32x4;
                using sse::SumFloat32x4;
                using sse::VectorCompareOperation;

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

                inline __m128d AbsFloat64x2(__m128d _values)
                {
                    return _mm_andnot_pd(_mm_set1_pd(-0.0), _values);
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

                inline bool CompareFloat64Element(double _value1, double _value2, double _epsilon, VectorCompareOperation _operation)
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
                bool CompareFloat64VectorElements(
                    const VectorType& _vector1,
                    const VectorType& _vector2,
                    double _epsilon,
                    uint32_t _elementCount,
                    VectorCompareOperation _operation,
                    bool _allElements)
                {
                    uint32_t elementIndex = 0;

                    for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                    {
                        const __m128d vector1Values = LoadFloat64x2(_vector1, elementIndex);
                        const __m128d vector2Values = LoadFloat64x2(_vector2, elementIndex);
                        const __m128d epsilonValues = _mm_set1_pd(_epsilon);
                        const uint32_t compareMask = static_cast<uint32_t>(_mm_movemask_pd(CompareFloat64x2(vector1Values, vector2Values, epsilonValues, _operation)));

                        if (_allElements)
                        {
                            if (compareMask != 0x3)
                            {
                                return false;
                            }
                        }
                        else if (compareMask != 0)
                        {
                            return true;
                        }
                    }

                    for (; elementIndex < _elementCount; ++elementIndex)
                    {
                        const bool isMatch = CompareFloat64Element(_vector1.getElement(elementIndex), _vector2.getElement(elementIndex), _epsilon, _operation);
                        if (_allElements)
                        {
                            if (!isMatch)
                            {
                                return false;
                            }
                        }
                        else if (isMatch)
                        {
                            return true;
                        }
                    }

                    return _allElements;
                }

                template <typename VectorType>
                VectorType& AddFloat64VectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
                {
                    uint32_t elementIndex = 0;

                    for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                    {
                        StoreFloat64x2(_target, _mm_add_pd(LoadFloat64x2(_target, elementIndex), LoadFloat64x2(_source, elementIndex)), elementIndex);
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
                        StoreFloat64x2(_target, _mm_sub_pd(LoadFloat64x2(_target, elementIndex), LoadFloat64x2(_source, elementIndex)), elementIndex);
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
                        StoreFloat64x2(_target, _mm_mul_pd(LoadFloat64x2(_target, elementIndex), _mm_set1_pd(_value)), elementIndex);
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
                        StoreFloat64x2(_target, _mm_mul_pd(LoadFloat64x2(_target, elementIndex), LoadFloat64x2(_source, elementIndex)), elementIndex);
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
                        StoreFloat64x2(_target, _mm_div_pd(LoadFloat64x2(_target, elementIndex), _mm_set1_pd(_value)), elementIndex);
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
                        StoreFloat64x2(_target, _mm_sub_pd(_mm_setzero_pd(), LoadFloat64x2(_target, elementIndex)), elementIndex);
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
                        StoreFloat64x2(_target, _mm_div_pd(_mm_set1_pd(1.0), LoadFloat64x2(_target, elementIndex)), elementIndex);
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
                        StoreFloat64x2(_target, AbsFloat64x2(LoadFloat64x2(_target, elementIndex)), elementIndex);
                    }

                    for (; elementIndex < _elementCount; ++elementIndex)
                    {
                        const double value = _target.getElement(elementIndex);
                        _target.setElement(elementIndex, value < 0.0 ? -value : value);
                    }

                    return _target;
                }

                template <typename VectorType>
                double DotFloat64VectorElements(const VectorType& _vector1, const VectorType& _vector2, uint32_t _elementCount)
                {
                    uint32_t elementIndex = 0;
                    __m128d values = _mm_setzero_pd();

                    for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                    {
                        values = _mm_add_pd(values, _mm_mul_pd(LoadFloat64x2(_vector1, elementIndex), LoadFloat64x2(_vector2, elementIndex)));
                    }

                    double result = SumFloat64x2(values);
                    for (; elementIndex < _elementCount; ++elementIndex)
                    {
                        result += _vector1.getElement(elementIndex) * _vector2.getElement(elementIndex);
                    }

                    return result;
                }

                template <typename VectorType>
                void CrossFloat64Vector3Elements(const VectorType& _vector1, const VectorType& _vector2, VectorType& _out)
                {
                    const __m128d firstValues = _mm_set_pd(_vector1.getElement(2), _vector1.getElement(1));
                    const __m128d secondValues = _mm_set_pd(_vector2.getElement(0), _vector2.getElement(2));
                    const __m128d thirdValues = _mm_set_pd(_vector1.getElement(0), _vector1.getElement(2));
                    const __m128d fourthValues = _mm_set_pd(_vector2.getElement(2), _vector2.getElement(1));

                    StoreFloat64x2(_out, _mm_sub_pd(_mm_mul_pd(firstValues, secondValues), _mm_mul_pd(thirdValues, fourthValues)), 0);
                    _out.setElement(2, _vector1.getElement(0) * _vector2.getElement(1) - _vector1.getElement(1) * _vector2.getElement(0));
                }

                template <typename VectorType>
                void TransformFloat64Matrix2x2VectorElements(const VectorType& _column0, const VectorType& _column1, const VectorType& _vector, VectorType& _out)
                {
                    const __m128d resultValues =
                        _mm_add_pd(_mm_mul_pd(LoadFloat64x2(_column0, 0), _mm_set1_pd(_vector.getElement(0))), _mm_mul_pd(LoadFloat64x2(_column1, 0), _mm_set1_pd(_vector.getElement(1))));

                    StoreFloat64x2(_out, resultValues, 0);
                }

                template <typename VectorType>
                void TransformFloat64Matrix3x3VectorElements(
                    const VectorType& _column0,
                    const VectorType& _column1,
                    const VectorType& _column2,
                    const VectorType& _vector,
                    VectorType& _out)
                {
                    const __m128d resultValues = _mm_add_pd(
                        _mm_add_pd(_mm_mul_pd(LoadFloat64x2(_column0, 0), _mm_set1_pd(_vector.getElement(0))), _mm_mul_pd(LoadFloat64x2(_column1, 0), _mm_set1_pd(_vector.getElement(1)))),
                        _mm_mul_pd(LoadFloat64x2(_column2, 0), _mm_set1_pd(_vector.getElement(2))));

                    StoreFloat64x2(_out, resultValues, 0);
                    _out.setElement(
                        2,
                        _column0.getElement(2) * _vector.getElement(0) + _column1.getElement(2) * _vector.getElement(1) + _column2.getElement(2) * _vector.getElement(2));
                }

                template <typename VectorType>
                void TransformFloat64Matrix4x4VectorElements(
                    const VectorType& _column0,
                    const VectorType& _column1,
                    const VectorType& _column2,
                    const VectorType& _column3,
                    const VectorType& _vector,
                    VectorType& _out)
                {
                    const __m128d lowValues = _mm_add_pd(
                        _mm_add_pd(_mm_mul_pd(LoadFloat64x2(_column0, 0), _mm_set1_pd(_vector.getElement(0))), _mm_mul_pd(LoadFloat64x2(_column1, 0), _mm_set1_pd(_vector.getElement(1)))),
                        _mm_add_pd(_mm_mul_pd(LoadFloat64x2(_column2, 0), _mm_set1_pd(_vector.getElement(2))), _mm_mul_pd(LoadFloat64x2(_column3, 0), _mm_set1_pd(_vector.getElement(3)))));
                    const __m128d highValues = _mm_add_pd(
                        _mm_add_pd(_mm_mul_pd(LoadFloat64x2(_column0, 2), _mm_set1_pd(_vector.getElement(0))), _mm_mul_pd(LoadFloat64x2(_column1, 2), _mm_set1_pd(_vector.getElement(1)))),
                        _mm_add_pd(_mm_mul_pd(LoadFloat64x2(_column2, 2), _mm_set1_pd(_vector.getElement(2))), _mm_mul_pd(LoadFloat64x2(_column3, 2), _mm_set1_pd(_vector.getElement(3)))));

                    StoreFloat64x2(_out, lowValues, 0);
                    StoreFloat64x2(_out, highValues, 2);
                }

                template <typename MatrixType>
                double DotFloat64DynamicMatrixRowColumnElements(
                    const MatrixType& _matrix1,
                    const MatrixType& _matrix2,
                    uint32_t _rowIndex,
                    uint32_t _columnIndex,
                    uint32_t _elementCount)
                {
                    uint32_t elementIndex = 0;
                    __m128d values = _mm_setzero_pd();

                    for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                    {
                        const __m128d rowValues = _mm_set_pd(_matrix1.getElement(_rowIndex, elementIndex + 1), _matrix1.getElement(_rowIndex, elementIndex));
                        const __m128d columnValues = _mm_set_pd(_matrix2.getElement(elementIndex + 1, _columnIndex), _matrix2.getElement(elementIndex, _columnIndex));

                        values = _mm_add_pd(values, _mm_mul_pd(rowValues, columnValues));
                    }

                    double result = SumFloat64x2(values);
                    for (; elementIndex < _elementCount; ++elementIndex)
                    {
                        result += _matrix1.getElement(_rowIndex, elementIndex) * _matrix2.getElement(elementIndex, _columnIndex);
                    }

                    return result;
                }

                template <typename MatrixType, typename VectorType>
                double DotFloat64DynamicMatrixRowVectorElements(const MatrixType& _matrix, const VectorType& _vector, uint32_t _rowIndex, uint32_t _elementCount)
                {
                    uint32_t elementIndex = 0;
                    __m128d values = _mm_setzero_pd();

                    for (; elementIndex + Float64x2ElementCount <= _elementCount; elementIndex += Float64x2ElementCount)
                    {
                        const __m128d rowValues = _mm_set_pd(_matrix.getElement(_rowIndex, elementIndex + 1), _matrix.getElement(_rowIndex, elementIndex));
                        const __m128d vectorValues = _mm_set_pd(_vector.getElement(elementIndex + 1), _vector.getElement(elementIndex));

                        values = _mm_add_pd(values, _mm_mul_pd(rowValues, vectorValues));
                    }

                    double result = SumFloat64x2(values);
                    for (; elementIndex < _elementCount; ++elementIndex)
                    {
                        result += _matrix.getElement(_rowIndex, elementIndex) * _vector.getElement(elementIndex);
                    }

                    return result;
                }

                template <typename VectorType>
                VectorType& AddVectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return AddFloat64VectorElements(_target, _source, _elementCount);
                    }

                    return sse::AddVectorElements(_target, _source, _elementCount);
                }

                template <typename VectorType>
                VectorType& SubtractVectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return SubtractFloat64VectorElements(_target, _source, _elementCount);
                    }

                    return sse::SubtractVectorElements(_target, _source, _elementCount);
                }

                template <typename VectorType>
                VectorType& MultiplyVectorElements(VectorType& _target, typename VectorType::ValueType _value, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return MultiplyFloat64VectorElements(_target, _value, _elementCount);
                    }

                    return sse::MultiplyVectorElements(_target, _value, _elementCount);
                }

                template <typename VectorType>
                VectorType& MultiplyVectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return MultiplyFloat64VectorElements(_target, _source, _elementCount);
                    }

                    return sse::MultiplyVectorElements(_target, _source, _elementCount);
                }

                template <typename VectorType>
                VectorType& DivideVectorElements(VectorType& _target, typename VectorType::ValueType _value, uint32_t _elementCount)
                {
                    EGO_ASSERT(std::abs(_value) > TypedEpsilon<typename VectorType::ValueType>());
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return DivideFloat64VectorElements(_target, _value, _elementCount);
                    }

                    return sse::DivideVectorElements(_target, _value, _elementCount);
                }

                template <typename VectorType>
                VectorType& NegateVectorElements(VectorType& _target, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return NegateFloat64VectorElements(_target, _elementCount);
                    }

                    return sse::NegateVectorElements(_target, _elementCount);
                }

                template <typename VectorType>
                VectorType& ReciprocalVectorElements(VectorType& _target, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return ReciprocalFloat64VectorElements(_target, _elementCount);
                    }

                    return sse::ReciprocalVectorElements(_target, _elementCount);
                }

                template <typename VectorType>
                VectorType& AbsVectorElements(VectorType& _target, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return AbsFloat64VectorElements(_target, _elementCount);
                    }

                    return sse::AbsVectorElements(_target, _elementCount);
                }

                template <typename VectorType>
                typename VectorType::ValueType DotVectorElements(const VectorType& _vector1, const VectorType& _vector2, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return DotFloat64VectorElements(_vector1, _vector2, _elementCount);
                    }

                    return sse::DotVectorElements(_vector1, _vector2, _elementCount);
                }

                template <typename VectorType>
                void CrossVector3Elements(const VectorType& _vector1, const VectorType& _vector2, VectorType& _out)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        CrossFloat64Vector3Elements(_vector1, _vector2, _out);
                        return;
                    }

                    sse::CrossVector3Elements(_vector1, _vector2, _out);
                }

                template <typename VectorType>
                bool AreVectorElementsEqual(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return CompareFloat64VectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Equal, true);
                    }

                    return sse::AreVectorElementsEqual(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool AreVectorElementsLess(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return CompareFloat64VectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Less, true);
                    }

                    return sse::AreVectorElementsLess(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool AreVectorElementsLessOrEqual(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return CompareFloat64VectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::LessOrEqual, true);
                    }

                    return sse::AreVectorElementsLessOrEqual(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool AreVectorElementsGreater(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return CompareFloat64VectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Greater, true);
                    }

                    return sse::AreVectorElementsGreater(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool AreVectorElementsGreaterOrEqual(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return CompareFloat64VectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::GreaterOrEqual, true);
                    }

                    return sse::AreVectorElementsGreaterOrEqual(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool IsAnyVectorElementLess(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return CompareFloat64VectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Less, false);
                    }

                    return sse::IsAnyVectorElementLess(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool IsAnyVectorElementLessOrEqual(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return CompareFloat64VectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::LessOrEqual, false);
                    }

                    return sse::IsAnyVectorElementLessOrEqual(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool IsAnyVectorElementGreater(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return CompareFloat64VectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::Greater, false);
                    }

                    return sse::IsAnyVectorElementGreater(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                bool IsAnyVectorElementGreaterOrEqual(const VectorType& _vector1, const VectorType& _vector2, typename VectorType::ValueType _epsilon, uint32_t _elementCount)
                {
                    EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return CompareFloat64VectorElements(_vector1, _vector2, _epsilon, _elementCount, VectorCompareOperation::GreaterOrEqual, false);
                    }

                    return sse::IsAnyVectorElementGreaterOrEqual(_vector1, _vector2, _epsilon, _elementCount);
                }

                template <typename VectorType>
                void TransformMatrix2x2VectorElements(const VectorType& _column0, const VectorType& _column1, const VectorType& _vector, VectorType& _out)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        TransformFloat64Matrix2x2VectorElements(_column0, _column1, _vector, _out);
                        return;
                    }

                    sse::TransformMatrix2x2VectorElements(_column0, _column1, _vector, _out);
                }

                template <typename VectorType>
                void TransformMatrix3x3VectorElements(
                    const VectorType& _column0,
                    const VectorType& _column1,
                    const VectorType& _column2,
                    const VectorType& _vector,
                    VectorType& _out)
                {
                    if constexpr (std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        TransformFloat64Matrix3x3VectorElements(_column0, _column1, _column2, _vector, _out);
                        return;
                    }

                    sse::TransformMatrix3x3VectorElements(_column0, _column1, _column2, _vector, _out);
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
                        TransformFloat64Matrix4x4VectorElements(_column0, _column1, _column2, _column3, _vector, _out);
                        return;
                    }

                    sse::TransformMatrix4x4VectorElements(_column0, _column1, _column2, _column3, _vector, _out);
                }

                template <typename MatrixType>
                typename MatrixType::ValueType DotDynamicMatrixRowColumnElements(
                    const MatrixType& _matrix1,
                    const MatrixType& _matrix2,
                    uint32_t _rowIndex,
                    uint32_t _columnIndex,
                    uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename MatrixType::ValueType, double>)
                    {
                        return DotFloat64DynamicMatrixRowColumnElements(_matrix1, _matrix2, _rowIndex, _columnIndex, _elementCount);
                    }

                    return sse::DotDynamicMatrixRowColumnElements(_matrix1, _matrix2, _rowIndex, _columnIndex, _elementCount);
                }

                template <typename MatrixType, typename VectorType>
                typename MatrixType::ValueType DotDynamicMatrixRowVectorElements(const MatrixType& _matrix, const VectorType& _vector, uint32_t _rowIndex, uint32_t _elementCount)
                {
                    if constexpr (std::is_same_v<typename MatrixType::ValueType, double> && std::is_same_v<typename VectorType::ValueType, double>)
                    {
                        return DotFloat64DynamicMatrixRowVectorElements(_matrix, _vector, _rowIndex, _elementCount);
                    }

                    return sse::DotDynamicMatrixRowVectorElements(_matrix, _vector, _rowIndex, _elementCount);
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
            } // namespace sse2
        } // namespace simd
    } // namespace math
} // namespace ego
