#pragma once

namespace ego
{
    template <typename T>
    T DotComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2)
    {
        return _vector1.dot(_vector2);
    }

    template <typename T>
    ComputeVector3Base<T> CrossComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2)
    {
        return _vector1.cross(_vector2);
    }

    template <typename T>
    ComputeVector3Base<T> NegateComputeVector3(const ComputeVector3Base<T>& _vector)
    {
        ComputeVector3Base<T> result(_vector);
        return result.negate();
    }

    template <typename T>
    ComputeVector3Base<T> ComputeReciprocalComputeVector3(const ComputeVector3Base<T>& _vector)
    {
        ComputeVector3Base<T> result(_vector);
        return result.computeReciprocal();
    }

    template <typename T>
    ComputeVector3Base<T> NormalizeComputeVector3(const ComputeVector3Base<T>& _vector)
    {
        ComputeVector3Base<T> result(_vector);
        return result.normalize();
    }

    template <typename T>
    T LengthComputeVector3(const ComputeVector3Base<T>& _vector)
    {
        return _vector.getLength();
    }

    template <typename T>
    T LengthSqrComputeVector3(const ComputeVector3Base<T>& _vector)
    {
        return _vector.getLengthSqr();
    }

    template <typename T>
    ComputeVector3Base<T> MultiplyComputeVector3Elements(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2)
    {
        ComputeVector3Base<T> result(_vector1);
        return result.multiplyElements(_vector2);
    }

    template <typename T>
    ComputeVector3Base<T> MinComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2)
    {
        return ComputeVector3Base<T>(std::min(_vector1.getX(), _vector2.getX()), std::min(_vector1.getY(), _vector2.getY()), std::min(_vector1.getZ(), _vector2.getZ()));
    }

    template <typename T>
    ComputeVector3Base<T> MaxComputeVector3(const ComputeVector3Base<T>& _vector1, const ComputeVector3Base<T>& _vector2)
    {
        return ComputeVector3Base<T>(std::max(_vector1.getX(), _vector2.getX()), std::max(_vector1.getY(), _vector2.getY()), std::max(_vector1.getZ(), _vector2.getZ()));
    }
} // namespace ego
