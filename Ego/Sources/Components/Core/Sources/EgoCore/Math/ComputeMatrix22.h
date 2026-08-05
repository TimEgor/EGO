#pragma once

#include "ComputeMatrix.h"

namespace ego
{
    template <typename T>
    using ComputeMatrix2x2Base = ComputeMatrixBase<T, 2>;

    template <typename T>
    ComputeMatrix2x2Base<T> InvertComputeMatrix2x2(const ComputeMatrix2x2Base<T>& _matrix);
    template <typename T>
    ComputeMatrix2x2Base<T> TransposeComputeMatrix2x2(const ComputeMatrix2x2Base<T>& _matrix);

    template <typename T>
    ComputeMatrix2x2Base<T> ComputeMatrix2x2ZeroBase()
    {
        return ComputeMatrix2x2Base<T>(T(0.0), T(0.0), T(0.0), T(0.0));
    }

    template <typename T>
    ComputeMatrix2x2Base<T> ComputeMatrix2x2IdentityBase()
    {
        return ComputeMatrix2x2Base<T>(T(1.0), T(0.0), T(0.0), T(1.0));
    }

    using ComputeMatrix2x2 = ComputeMatrix2x2Base<ComputeValueType>;
    using FloatComputeMatrix2x2 = ComputeMatrix2x2Base<float>;
} // namespace ego

#include "ComputeMatrix22.hpp"
