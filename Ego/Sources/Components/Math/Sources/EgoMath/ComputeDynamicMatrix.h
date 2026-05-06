#pragma once

#include "DynamicMatrix.h"

namespace ego
{
	void TransposeDynamicMatrix(const FloatDynamicMatrix& _matrix, FloatDynamicMatrix& _result);
	FloatDynamicMatrix TransposeDynamicMatrix(const FloatDynamicMatrix& _matrix);

	void MultiplyDynamicMatrix(const FloatDynamicMatrix& _matrix1, const FloatDynamicMatrix& _matrix2, FloatDynamicMatrix& _result);
	FloatDynamicMatrix MultiplyDynamicMatrix(const FloatDynamicMatrix& _matrix1, const FloatDynamicMatrix& _matrix2);

	void TransformDynamicVector(const FloatDynamicMatrix& _matrix, const FloatDynamicVector& _vector, FloatDynamicVector& _result);
	FloatDynamicVector TransformDynamicVector(const FloatDynamicMatrix& _matrix, const FloatDynamicVector& _vector);

	void DynamicMatrixAddDiagonal(FloatDynamicMatrix& _matrix, float _value);
}
