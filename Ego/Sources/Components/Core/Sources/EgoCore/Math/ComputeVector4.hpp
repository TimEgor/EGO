#pragma once

namespace ego
{
    template <typename T>
    ComputeVector4Base<T> ComputeVector4FromPoint(const ComputeVector3Base<T>& _vector)
    {
        return ComputeVector4Base<T>(_vector, T(1.0));
    }

    template <typename T>
    T DotComputeVector4(const ComputeVector4Base<T>& _vector1, const ComputeVector4Base<T>& _vector2)
    {
        return _vector1.dot(_vector2);
    }
} // namespace ego
