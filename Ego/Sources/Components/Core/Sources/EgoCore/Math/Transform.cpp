#include "Transform.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "EgoCore/Assert/Assert.h"

#include "ComputeQuaternion.h"

namespace
{
    template <typename T, uint32_t Size>
    bool TryNormalizeFinite(ego::ComputeVectorBase<T, Size>& _vector)
    {
        T maxAbsoluteValue = T(0.0);
        for (uint32_t elementIndex = 0; elementIndex < Size; ++elementIndex)
        {
            const T value = _vector.getElement(elementIndex);
            if (!std::isfinite(value))
            {
                return false;
            }

            maxAbsoluteValue = (std::max)(maxAbsoluteValue, std::abs(value));
        }

        if (maxAbsoluteValue <= T(0.0))
        {
            return false;
        }

        for (uint32_t elementIndex = 0; elementIndex < Size; ++elementIndex)
        {
            _vector.setElement(elementIndex, _vector.getElement(elementIndex) / maxAbsoluteValue);
        }

        const T scaledLength = _vector.getLength();
        if (!std::isfinite(scaledLength) || scaledLength <= T(0.0) || maxAbsoluteValue <= ego::math::TypedEpsilon<T>() / scaledLength)
        {
            return false;
        }

        _vector /= scaledLength;
        for (uint32_t elementIndex = 0; elementIndex < Size; ++elementIndex)
        {
            if (!std::isfinite(_vector.getElement(elementIndex)))
            {
                return false;
            }
        }

        return true;
    }
} // namespace

ego::Transform::Transform(const TransformMatrix& _matrix)
    : m_matrix(_matrix)
{
}

const ego::TransformMatrix& ego::Transform::getMatrix() const
{
    return m_matrix;
}

void ego::Transform::setMatrix(const TransformMatrix& _matrix)
{
    m_matrix = _matrix;
}

ego::TransformVector ego::Transform::getAxisX() const
{
    return TransformVector(m_matrix.getElement(0, 0), m_matrix.getElement(1, 0), m_matrix.getElement(2, 0));
}

void ego::Transform::setAxisX(const TransformVector& _axis)
{
    m_matrix.setElement(0, 0, _axis.getX());
    m_matrix.setElement(1, 0, _axis.getY());
    m_matrix.setElement(2, 0, _axis.getZ());
}

ego::TransformVector ego::Transform::getAxisY() const
{
    return TransformVector(m_matrix.getElement(0, 1), m_matrix.getElement(1, 1), m_matrix.getElement(2, 1));
}

void ego::Transform::setAxisY(const TransformVector& _axis)
{
    m_matrix.setElement(0, 1, _axis.getX());
    m_matrix.setElement(1, 1, _axis.getY());
    m_matrix.setElement(2, 1, _axis.getZ());
}

ego::TransformVector ego::Transform::getAxisZ() const
{
    return TransformVector(m_matrix.getElement(0, 2), m_matrix.getElement(1, 2), m_matrix.getElement(2, 2));
}

void ego::Transform::setAxisZ(const TransformVector& _axis)
{
    m_matrix.setElement(0, 2, _axis.getX());
    m_matrix.setElement(1, 2, _axis.getY());
    m_matrix.setElement(2, 2, _axis.getZ());
}

ego::TransformVector ego::Transform::getOrigin() const
{
    return TransformVector(m_matrix.getElement(0, 3), m_matrix.getElement(1, 3), m_matrix.getElement(2, 3));
}

void ego::Transform::setOrigin(const TransformVector& _origin)
{
    m_matrix.setElement(0, 3, _origin.getX());
    m_matrix.setElement(1, 3, _origin.getY());
    m_matrix.setElement(2, 3, _origin.getZ());
}

ego::TransformLinearMatrix ego::Transform::getLinearMatrix() const
{
    TransformLinearMatrix linearMatrix;
    getLinearMatrix(linearMatrix);

    return linearMatrix;
}

void ego::Transform::getLinearMatrix(TransformLinearMatrix& _linearMatrix) const
{
    for (size_t columnIndex = 0; columnIndex < 3; ++columnIndex)
    {
        for (size_t rowIndex = 0; rowIndex < 3; ++rowIndex)
        {
            _linearMatrix.setElement(rowIndex, columnIndex, m_matrix.getElement(rowIndex, columnIndex));
        }
    }
}

void ego::Transform::setLinearMatrix(const TransformLinearMatrix& _linearMatrix)
{
    for (size_t columnIndex = 0; columnIndex < 3; ++columnIndex)
    {
        for (size_t rowIndex = 0; rowIndex < 3; ++rowIndex)
        {
            m_matrix.setElement(rowIndex, columnIndex, _linearMatrix.getElement(rowIndex, columnIndex));
        }
    }
}

ego::TransformLinearMatrix ego::Transform::getRotationMatrix() const
{
    return getLinearMatrix();
}

void ego::Transform::getRotationMatrix(TransformLinearMatrix& _rotation) const
{
    getLinearMatrix(_rotation);
}

void ego::Transform::setRotationMatrix(const TransformLinearMatrix& _rotation)
{
    setLinearMatrix(_rotation);
}

ego::TransformQuaternion ego::Transform::getRotationQuaternion() const
{
    TransformQuaternion quaternion;
    getRotationQuaternion(quaternion);

    return quaternion;
}

void ego::Transform::getRotationQuaternion(TransformQuaternion& _rotation) const
{
    const bool hasRotation = tryGetRotationQuaternion(_rotation);
    EGO_ASSERT(hasRotation);

    if (!hasRotation)
    {
        _rotation = TransformQuaternionIdentity;
    }
}

bool ego::Transform::tryGetRotationQuaternion(TransformQuaternion& _rotation) const
{
    ComputeVector3 axisX(getAxisX());
    ComputeVector3 axisY(getAxisY());
    ComputeVector3 sourceAxisZ(getAxisZ());

    if (!TryNormalizeFinite(axisX) || !TryNormalizeFinite(axisY) || !TryNormalizeFinite(sourceAxisZ))
    {
        return false;
    }

    axisY -= axisX * axisX.dot(axisY);
    if (!TryNormalizeFinite(axisY))
    {
        return false;
    }

    ComputeVector3 axisZ = CrossComputeVector3(axisX, axisY);
    if (!TryNormalizeFinite(axisZ) || axisZ.dot(sourceAxisZ) <= ComputeValue(0.0))
    {
        return false;
    }

    ComputeQuaternion rotation(ComputeMatrix3x3(axisX, axisY, axisZ));
    if (!TryNormalizeFinite(rotation.getVector()))
    {
        return false;
    }

    _rotation = TransformQuaternion(rotation.getX(), rotation.getY(), rotation.getZ(), rotation.getW());

    return true;
}

void ego::Transform::setRotationQuaternion(const TransformQuaternion& _rotation)
{
    ComputeQuaternion rotation(_rotation);
    const bool hasRotation = TryNormalizeFinite(rotation.getVector());
    EGO_ASSERT(hasRotation);

    if (!hasRotation)
    {
        return;
    }

    setLinearMatrix(rotation.getRotationMatrix3x3().getMatrix<TransformValue>());
}
