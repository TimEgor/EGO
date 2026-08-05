#pragma once

#include "DynamicMatrix.h"

namespace ego
{
    void TransposeDynamicMatrix(const DynamicMatrix& _matrix, DynamicMatrix& _result);
    DynamicMatrix TransposeDynamicMatrix(const DynamicMatrix& _matrix);

    void MultiplyDynamicMatrix(const DynamicMatrix& _matrix1, const DynamicMatrix& _matrix2, DynamicMatrix& _result);
    DynamicMatrix MultiplyDynamicMatrix(const DynamicMatrix& _matrix1, const DynamicMatrix& _matrix2);

    void TransformDynamicVector(const DynamicMatrix& _matrix, const DynamicVector& _vector, DynamicVector& _result);
    DynamicVector TransformDynamicVector(const DynamicMatrix& _matrix, const DynamicVector& _vector);

    void DynamicMatrixAddDiagonal(DynamicMatrix& _matrix, ComputeValue _value);
} // namespace ego
