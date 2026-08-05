#include "ComputeDynamicMatrix.h"

#include <utility>

#include "EgoCore/Assert/Assert.h"

void ego::TransposeDynamicMatrix(const DynamicMatrix& _matrix, DynamicMatrix& _result)
{
    DynamicMatrix result = TransposeDynamicMatrix(_matrix);
    _result = std::move(result);
}

ego::DynamicMatrix ego::TransposeDynamicMatrix(const DynamicMatrix& _matrix)
{
    const uint32_t rowCount = _matrix.getRowCount();
    const uint32_t columnCount = _matrix.getColumnCount();
    if (rowCount == 0 || columnCount == 0)
    {
        EGO_ASSERT(rowCount > 0 && columnCount > 0);

        return DynamicMatrix();
    }

    DynamicMatrix result(columnCount, rowCount);

    for (uint32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex)
    {
        for (uint32_t columnIndex = 0; columnIndex < columnCount; ++columnIndex)
        {
            result.setElement(columnIndex, rowIndex, _matrix.getElement(rowIndex, columnIndex));
        }
    }

    return result;
}

void ego::MultiplyDynamicMatrix(const DynamicMatrix& _matrix1, const DynamicMatrix& _matrix2, DynamicMatrix& _result)
{
    DynamicMatrix result = MultiplyDynamicMatrix(_matrix1, _matrix2);
    _result = std::move(result);
}

ego::DynamicMatrix ego::MultiplyDynamicMatrix(const DynamicMatrix& _matrix1, const DynamicMatrix& _matrix2)
{
    const uint32_t rowCount1 = _matrix1.getRowCount();
    const uint32_t columnCount1 = _matrix1.getColumnCount();

    const uint32_t rowCount2 = _matrix2.getRowCount();
    const uint32_t columnCount2 = _matrix2.getColumnCount();

    if (rowCount1 == 0 || columnCount1 == 0 || rowCount2 == 0 || columnCount2 == 0 || columnCount1 != rowCount2)
    {
        EGO_ASSERT(rowCount1 > 0 && columnCount1 > 0 && rowCount2 > 0 && columnCount2 > 0 && columnCount1 == rowCount2);

        return DynamicMatrix();
    }

    DynamicMatrix result(rowCount1, columnCount2);
    if (result.getRowCount() != rowCount1 || result.getColumnCount() != columnCount2)
    {
        return DynamicMatrix();
    }

    for (uint32_t rowIndex = 0; rowIndex < rowCount1; ++rowIndex)
    {
        for (uint32_t columnIndex = 0; columnIndex < columnCount2; ++columnIndex)
        {
            ComputeValue value = DynamicMatrix::DefaultValue;
            for (uint32_t elementIndex = 0; elementIndex < columnCount1; ++elementIndex)
            {
                value += _matrix1.getElement(rowIndex, elementIndex) * _matrix2.getElement(elementIndex, columnIndex);
            }

            result.setElement(rowIndex, columnIndex, value);
        }
    }

    return result;
}

void ego::TransformDynamicVector(const DynamicMatrix& _matrix, const DynamicVector& _vector, DynamicVector& _result)
{
    DynamicVector result = TransformDynamicVector(_matrix, _vector);
    _result = std::move(result);
}

ego::DynamicVector ego::TransformDynamicVector(const DynamicMatrix& _matrix, const DynamicVector& _vector)
{
    const uint32_t rowCount = _matrix.getRowCount();
    const uint32_t columnCount = _matrix.getColumnCount();

    if (rowCount == 0 || columnCount == 0 || _vector.getElementCount() != columnCount)
    {
        EGO_ASSERT(rowCount > 0 && columnCount > 0 && _vector.getElementCount() == columnCount);

        return DynamicVector();
    }

    DynamicVector result(rowCount);

    for (uint32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex)
    {
        ComputeValue value = DynamicVector::DefaultValue;
        for (uint32_t elementIndex = 0; elementIndex < columnCount; ++elementIndex)
        {
            value += _matrix.getElement(rowIndex, elementIndex) * _vector.getElement(elementIndex);
        }

        result.setElement(rowIndex, value);
    }

    return result;
}

void ego::DynamicMatrixAddDiagonal(DynamicMatrix& _matrix, ComputeValue _value)
{
    const uint32_t rowCount = _matrix.getRowCount();
    const uint32_t columnCount = _matrix.getColumnCount();

    if (rowCount == 0 || columnCount == 0 || rowCount != columnCount)
    {
        EGO_ASSERT(rowCount > 0 && columnCount > 0 && rowCount == columnCount);

        return;
    }

    for (uint32_t elementIndex = 0; elementIndex < rowCount; ++elementIndex)
    {
        _matrix.getElement(elementIndex, elementIndex) += _value;
    }
}
