#pragma once

#include <cmath>
#include <cstdint>

#include "EgoCore/Assert/AssertCore.h"

#include "ComputeSimdSse2.h"

namespace ego
{
    namespace math
    {
        namespace simd
        {
            template <typename VectorType>
            VectorType& AddVectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
            {
                if (TryAddVectorElementsSimd(_target, _source, _elementCount))
                {
                    return _target;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, _target.getElement(elementIndex) + _source.getElement(elementIndex));
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& SubtractVectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
            {
                if (TrySubtractVectorElementsSimd(_target, _source, _elementCount))
                {
                    return _target;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, _target.getElement(elementIndex) - _source.getElement(elementIndex));
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& MultiplyVectorElements(VectorType& _target, typename VectorType::ValueType _value, uint32_t _elementCount)
            {
                if (TryMultiplyVectorElementsSimd(_target, _value, _elementCount))
                {
                    return _target;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, _target.getElement(elementIndex) * _value);
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& MultiplyVectorElements(VectorType& _target, const VectorType& _source, uint32_t _elementCount)
            {
                if (TryMultiplyVectorElementsSimd(_target, _source, _elementCount))
                {
                    return _target;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, _target.getElement(elementIndex) * _source.getElement(elementIndex));
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& DivideVectorElements(VectorType& _target, typename VectorType::ValueType _value, uint32_t _elementCount)
            {
                EGO_ASSERT(std::abs(_value) > TypedEpsilon<typename VectorType::ValueType>());

                if (TryDivideVectorElementsSimd(_target, _value, _elementCount))
                {
                    return _target;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, _target.getElement(elementIndex) / _value);
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& NegateVectorElements(VectorType& _target, uint32_t _elementCount)
            {
                if (TryNegateVectorElementsSimd(_target, _elementCount))
                {
                    return _target;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, -_target.getElement(elementIndex));
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& ReciprocalVectorElements(VectorType& _target, uint32_t _elementCount)
            {
                if (TryReciprocalVectorElementsSimd(_target, _elementCount))
                {
                    return _target;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    _target.setElement(elementIndex, static_cast<typename VectorType::ValueType>(1.0) / _target.getElement(elementIndex));
                }

                return _target;
            }

            template <typename VectorType>
            VectorType& AbsVectorElements(VectorType& _target, uint32_t _elementCount)
            {
                if (TryAbsVectorElementsSimd(_target, _elementCount))
                {
                    return _target;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    const typename VectorType::ValueType value = _target.getElement(elementIndex);
                    _target.setElement(elementIndex, value < static_cast<typename VectorType::ValueType>(0.0) ? -value : value);
                }

                return _target;
            }

            template <typename VectorType>
            typename VectorType::ValueType DotVectorElements(const VectorType& _vector1, const VectorType& _vector2, uint32_t _elementCount)
            {
                typename VectorType::ValueType value = static_cast<typename VectorType::ValueType>(0.0);

                if (TryDotVectorElementsSimd(_vector1, _vector2, _elementCount, value))
                {
                    return value;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    value += _vector1.getElement(elementIndex) * _vector2.getElement(elementIndex);
                }

                return value;
            }

            template <typename VectorType>
            void CrossVector3Elements(const VectorType& _vector1, const VectorType& _vector2, VectorType& _out)
            {
                if (TryCrossVector3ElementsSimd(_vector1, _vector2, _out))
                {
                    return;
                }

                _out = VectorType(
                    _vector1.getElement(1) * _vector2.getElement(2) - _vector1.getElement(2) * _vector2.getElement(1),
                    _vector1.getElement(2) * _vector2.getElement(0) - _vector1.getElement(0) * _vector2.getElement(2),
                    _vector1.getElement(0) * _vector2.getElement(1) - _vector1.getElement(1) * _vector2.getElement(0));
            }

            template <typename VectorType>
            bool AreVectorElementsEqual(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount)
            {
                EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));

                bool result = false;
                if (TryAreVectorElementsEqualSimd(_vector1, _vector2, _epsilon, _elementCount, result))
                {
                    return result;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    if (!ego::math::IsApproxEqual(_vector1.getElement(elementIndex), _vector2.getElement(elementIndex), _epsilon))
                    {
                        return false;
                    }
                }

                return true;
            }

            template <typename VectorType>
            bool AreVectorElementsLess(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount)
            {
                EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));

                bool result = false;
                if (TryAreVectorElementsLessSimd(_vector1, _vector2, _epsilon, _elementCount, result))
                {
                    return result;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    if (!ego::math::IsApproxLess(_vector1.getElement(elementIndex), _vector2.getElement(elementIndex), _epsilon))
                    {
                        return false;
                    }
                }

                return true;
            }

            template <typename VectorType>
            bool AreVectorElementsLessOrEqual(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount)
            {
                EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));

                bool result = false;
                if (TryAreVectorElementsLessOrEqualSimd(_vector1, _vector2, _epsilon, _elementCount, result))
                {
                    return result;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    if (!ego::math::IsApproxLessOrEqual(_vector1.getElement(elementIndex), _vector2.getElement(elementIndex), _epsilon))
                    {
                        return false;
                    }
                }

                return true;
            }

            template <typename VectorType>
            bool AreVectorElementsGreater(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount)
            {
                EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));

                bool result = false;
                if (TryAreVectorElementsGreaterSimd(_vector1, _vector2, _epsilon, _elementCount, result))
                {
                    return result;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    if (!ego::math::IsApproxGreater(_vector1.getElement(elementIndex), _vector2.getElement(elementIndex), _epsilon))
                    {
                        return false;
                    }
                }

                return true;
            }

            template <typename VectorType>
            bool AreVectorElementsGreaterOrEqual(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount)
            {
                EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));

                bool result = false;
                if (TryAreVectorElementsGreaterOrEqualSimd(_vector1, _vector2, _epsilon, _elementCount, result))
                {
                    return result;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    if (!ego::math::IsApproxGreaterOrEqual(_vector1.getElement(elementIndex), _vector2.getElement(elementIndex), _epsilon))
                    {
                        return false;
                    }
                }

                return true;
            }

            template <typename VectorType>
            bool IsAnyVectorElementLess(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount)
            {
                EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));

                bool result = false;
                if (TryIsAnyVectorElementLessSimd(_vector1, _vector2, _epsilon, _elementCount, result))
                {
                    return result;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    if (ego::math::IsApproxLess(_vector1.getElement(elementIndex), _vector2.getElement(elementIndex), _epsilon))
                    {
                        return true;
                    }
                }

                return false;
            }

            template <typename VectorType>
            bool IsAnyVectorElementLessOrEqual(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount)
            {
                EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));

                bool result = false;
                if (TryIsAnyVectorElementLessOrEqualSimd(_vector1, _vector2, _epsilon, _elementCount, result))
                {
                    return result;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    if (ego::math::IsApproxLessOrEqual(_vector1.getElement(elementIndex), _vector2.getElement(elementIndex), _epsilon))
                    {
                        return true;
                    }
                }

                return false;
            }

            template <typename VectorType>
            bool IsAnyVectorElementGreater(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount)
            {
                EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));

                bool result = false;
                if (TryIsAnyVectorElementGreaterSimd(_vector1, _vector2, _epsilon, _elementCount, result))
                {
                    return result;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    if (ego::math::IsApproxGreater(_vector1.getElement(elementIndex), _vector2.getElement(elementIndex), _epsilon))
                    {
                        return true;
                    }
                }

                return false;
            }

            template <typename VectorType>
            bool IsAnyVectorElementGreaterOrEqual(
                const VectorType& _vector1,
                const VectorType& _vector2,
                typename VectorType::ValueType _epsilon,
                uint32_t _elementCount)
            {
                EGO_ASSERT(_epsilon > static_cast<typename VectorType::ValueType>(0.0));

                bool result = false;
                if (TryIsAnyVectorElementGreaterOrEqualSimd(_vector1, _vector2, _epsilon, _elementCount, result))
                {
                    return result;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    if (ego::math::IsApproxGreaterOrEqual(_vector1.getElement(elementIndex), _vector2.getElement(elementIndex), _epsilon))
                    {
                        return true;
                    }
                }

                return false;
            }

            template <typename VectorType>
            void TransformMatrix2x2VectorElements(
                const VectorType& _column0,
                const VectorType& _column1,
                const VectorType& _vector,
                VectorType& _out);

            template <typename VectorType>
            void TransformMatrix3x3VectorElements(
                const VectorType& _column0,
                const VectorType& _column1,
                const VectorType& _column2,
                const VectorType& _vector,
                VectorType& _out);

            template <typename VectorType>
            void TransformMatrix4x4VectorElements(
                const VectorType& _column0,
                const VectorType& _column1,
                const VectorType& _column2,
                const VectorType& _column3,
                const VectorType& _vector,
                VectorType& _out);

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

            template <typename VectorType>
            void TransformMatrix2x2VectorElements(
                const VectorType& _column0,
                const VectorType& _column1,
                const VectorType& _vector,
                VectorType& _out)
            {
                if (TryTransformMatrix2x2VectorSimd(_column0, _column1, _vector, _out))
                {
                    return;
                }

                _out.setElement(0, _column0[0] * _vector[0] + _column1[0] * _vector[1]);
                _out.setElement(1, _column0[1] * _vector[0] + _column1[1] * _vector[1]);
            }

            template <typename VectorType>
            void TransformMatrix3x3VectorElements(
                const VectorType& _column0,
                const VectorType& _column1,
                const VectorType& _column2,
                const VectorType& _vector,
                VectorType& _out)
            {
                if (TryTransformMatrix3x3VectorSimd(_column0, _column1, _column2, _vector, _out))
                {
                    return;
                }

                _out.setElement(0, _column0[0] * _vector[0] + _column1[0] * _vector[1] + _column2[0] * _vector[2]);
                _out.setElement(1, _column0[1] * _vector[0] + _column1[1] * _vector[1] + _column2[1] * _vector[2]);
                _out.setElement(2, _column0[2] * _vector[0] + _column1[2] * _vector[1] + _column2[2] * _vector[2]);
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
                if (TryTransformMatrix4x4VectorSimd(_column0, _column1, _column2, _column3, _vector, _out))
                {
                    return;
                }

                _out.setElement(0, _column0[0] * _vector[0] + _column1[0] * _vector[1] + _column2[0] * _vector[2] + _column3[0] * _vector[3]);
                _out.setElement(1, _column0[1] * _vector[0] + _column1[1] * _vector[1] + _column2[1] * _vector[2] + _column3[1] * _vector[3]);
                _out.setElement(2, _column0[2] * _vector[0] + _column1[2] * _vector[1] + _column2[2] * _vector[2] + _column3[2] * _vector[3]);
                _out.setElement(3, _column0[3] * _vector[0] + _column1[3] * _vector[1] + _column2[3] * _vector[2] + _column3[3] * _vector[3]);
            }

            template <typename MatrixType>
            typename MatrixType::ValueType DotDynamicMatrixRowColumnElements(
                const MatrixType& _matrix1,
                const MatrixType& _matrix2,
                uint32_t _rowIndex,
                uint32_t _columnIndex,
                uint32_t _elementCount)
            {
                typename MatrixType::ValueType value = static_cast<typename MatrixType::ValueType>(0.0);

                if (TryDotDynamicMatrixRowColumnSimd(_matrix1, _matrix2, _rowIndex, _columnIndex, _elementCount, value))
                {
                    return value;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    value += _matrix1.getElement(_rowIndex, elementIndex) * _matrix2.getElement(elementIndex, _columnIndex);
                }

                return value;
            }

            template <typename MatrixType, typename VectorType>
            typename MatrixType::ValueType DotDynamicMatrixRowVectorElements(
                const MatrixType& _matrix,
                const VectorType& _vector,
                uint32_t _rowIndex,
                uint32_t _elementCount)
            {
                typename MatrixType::ValueType value = static_cast<typename MatrixType::ValueType>(0.0);

                if (TryDotDynamicMatrixRowVectorSimd(_matrix, _vector, _rowIndex, _elementCount, value))
                {
                    return value;
                }

                for (uint32_t elementIndex = 0; elementIndex < _elementCount; ++elementIndex)
                {
                    value += _matrix.getElement(_rowIndex, elementIndex) * _vector.getElement(elementIndex);
                }

                return value;
            }
        } // namespace simd
    } // namespace math
} // namespace ego
