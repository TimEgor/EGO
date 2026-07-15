#include "ComputeDynamicMatrix.h"

#include "EgoCore/Assert/Assert.h"

#include "ComputeMath.h"

namespace
{
    float DotDynamicMatrixRowColumn(
        const ego::FloatDynamicMatrix& _matrix1,
        const ego::FloatDynamicMatrix& _matrix2,
        uint32_t _rowIndex,
        uint32_t _columnIndex,
        uint32_t _elementCount)
    {
        return ego::math::simd::DotDynamicMatrixRowColumnElements(_matrix1, _matrix2, _rowIndex, _columnIndex, _elementCount);
    }

    float DotDynamicMatrixRowVector(
        const ego::FloatDynamicMatrix& _matrix,
        const ego::FloatDynamicVector& _vector,
        uint32_t _rowIndex,
        uint32_t _elementCount)
    {
        return ego::math::simd::DotDynamicMatrixRowVectorElements(_matrix, _vector, _rowIndex, _elementCount);
    }
} // namespace

void ego::TransposeDynamicMatrix(const FloatDynamicMatrix& _matrix, FloatDynamicMatrix& _result)
{
    _result = TransposeDynamicMatrix(_matrix);
}

ego::FloatDynamicMatrix ego::TransposeDynamicMatrix(const FloatDynamicMatrix& _matrix)
{
    const uint32_t rowCount = _matrix.getRowCount();
    const uint32_t columnCount = _matrix.getColumnCount();

    FloatDynamicMatrix result(columnCount, rowCount);

    for (uint32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex)
    {
        for (uint32_t columnIndex = 0; columnIndex < columnCount; ++columnIndex)
        {
            result.setElement(columnIndex, rowIndex, _matrix.getElement(rowIndex, columnIndex));
        }
    }

    return result;
}

void ego::MultiplyDynamicMatrix(const FloatDynamicMatrix& _matrix1, const FloatDynamicMatrix& _matrix2, FloatDynamicMatrix& _result)
{
    _result = MultiplyDynamicMatrix(_matrix1, _matrix2);
}

ego::FloatDynamicMatrix ego::MultiplyDynamicMatrix(const FloatDynamicMatrix& _matrix1, const FloatDynamicMatrix& _matrix2)
{
    const uint32_t rowCount1 = _matrix1.getRowCount();
    const uint32_t columnCount1 = _matrix1.getColumnCount();

    const uint32_t rowCount2 = _matrix2.getRowCount();
    const uint32_t columnCount2 = _matrix2.getColumnCount();

    EGO_ASSERT(columnCount1 == rowCount2);

    FloatDynamicMatrix result(rowCount1, columnCount2);

    for (uint32_t rowIndex = 0; rowIndex < rowCount1; ++rowIndex)
    {
        for (uint32_t columnIndex = 0; columnIndex < columnCount2; ++columnIndex)
        {
            const float value = DotDynamicMatrixRowColumn(_matrix1, _matrix2, rowIndex, columnIndex, columnCount1);

            result.setElement(rowIndex, columnIndex, value);
        }
    }

    return result;
}

void ego::TransformDynamicVector(const FloatDynamicMatrix& _matrix, const FloatDynamicVector& _vector, FloatDynamicVector& _result)
{
    _result = TransformDynamicVector(_matrix, _vector);
}

ego::FloatDynamicVector ego::TransformDynamicVector(const FloatDynamicMatrix& _matrix, const FloatDynamicVector& _vector)
{
    const uint32_t rowCount = _matrix.getRowCount();
    const uint32_t columnCount = _matrix.getColumnCount();

    EGO_ASSERT(_vector.getElementCount() == columnCount);

    FloatDynamicVector result(rowCount);

    for (uint32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex)
    {
        const float value = DotDynamicMatrixRowVector(_matrix, _vector, rowIndex, columnCount);

        result.setElement(rowIndex, value);
    }

    return result;
}

void ego::DynamicMatrixAddDiagonal(FloatDynamicMatrix& _matrix, float _value)
{
    const uint32_t rowCount = _matrix.getRowCount();
    const uint32_t columnCount = _matrix.getColumnCount();

    EGO_ASSERT(rowCount == columnCount);

    for (uint32_t elementIndex = 0; elementIndex < rowCount; ++elementIndex)
    {
        _matrix[elementIndex][elementIndex] += _value;
    }
}
