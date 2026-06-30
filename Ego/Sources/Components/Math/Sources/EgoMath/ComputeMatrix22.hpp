#pragma once

namespace ego
{
    template <typename T>
    ComputeMatrix2x2Base<T> InvertComputeMatrix2x2(const ComputeMatrix2x2Base<T>& _matrix)
    {
        ComputeMatrix2x2Base<T> result(_matrix);
        return result.invert();
    }

    template <typename T>
    ComputeMatrix2x2Base<T> TransposeComputeMatrix2x2(const ComputeMatrix2x2Base<T>& _matrix)
    {
        ComputeMatrix2x2Base<T> result(_matrix);
        return result.transpose();
    }
} // namespace ego
