#pragma once

namespace ego
{
    template <typename T>
    T DotComputeVector2(const ComputeVector2Base<T>& _vector1, const ComputeVector2Base<T>& _vector2)
    {
        return _vector1.dot(_vector2);
    }
} // namespace ego
