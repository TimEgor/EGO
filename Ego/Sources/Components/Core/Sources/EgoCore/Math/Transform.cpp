#include "Transform.h"

ego::Transform::Transform(const TransformMatrix& _matrix)
    : m_matrix(_matrix)
{
}

ego::FloatVector3 ego::Transform::getAxisX() const
{
    return m_matrix.m_row1.m_xyz;
}

void ego::Transform::setAxisX(const FloatVector3& _axis)
{
    m_matrix.m_row1.m_xyz = _axis;
}

ego::FloatVector3 ego::Transform::getAxisY() const
{
    return m_matrix.m_row2.m_xyz;
}

void ego::Transform::setAxisY(const FloatVector3& _axis)
{
    m_matrix.m_row2.m_xyz = _axis;
}

ego::FloatVector3 ego::Transform::getAxisZ() const
{
    return m_matrix.m_row3.m_xyz;
}

void ego::Transform::setAxisZ(const FloatVector3& _axis)
{
    m_matrix.m_row3.m_xyz = _axis;
}

ego::FloatVector3 ego::Transform::getOrigin() const
{
    return m_matrix.m_row4.m_xyz;
}

void ego::Transform::setOrigin(const FloatVector3& _origin)
{
    m_matrix.m_row4.m_xyz = _origin;
}

ego::FloatMatrix3x3 ego::Transform::getRotationMatrix() const
{
    FloatMatrix3x3 rotation;
    getRotationMatrix(rotation);

    return rotation;
}

void ego::Transform::getRotationMatrix(FloatMatrix3x3& _rotation) const
{
    _rotation.m_row1 = getAxisX();
    _rotation.m_row2 = getAxisY();
    _rotation.m_row3 = getAxisZ();
}

void ego::Transform::setRotationMatrix(const FloatMatrix3x3& _rotation)
{
    setAxisX(_rotation.m_row1);
    setAxisY(_rotation.m_row2);
    setAxisZ(_rotation.m_row3);
}

ego::ComputeQuaternion ego::Transform::getRotationQuaternion() const
{
    ComputeQuaternion quaternion;
    getRotationQuaternion(quaternion);

    return quaternion;
}

void ego::Transform::getRotationQuaternion(ComputeQuaternion& _rotation) const
{
    const ComputeMatrix3x3 rotationMatrix(getRotationMatrix());
    _rotation.setupFromRotationMatrix3x3(rotationMatrix);
}

void ego::Transform::setRotationQuaternion(const ComputeQuaternion& _rotation)
{
    const ComputeMatrix3x3 rotationMatrix = _rotation.getRotationMatrix3x3();
    setRotationMatrix(rotationMatrix.getMatrix<float>());
}
