#include "Transform.h"

ego::ComputeMatrix3x3 ego::Transform::getRotationMatrix() const
{
    ComputeMatrix3x3 rotation;
    getRotationMatrix(rotation);

    return rotation;
}

void ego::Transform::getRotationMatrix(ComputeMatrix3x3& _rotation) const
{
    _rotation.setColumn(0, getAxisX());
    _rotation.setColumn(1, getAxisY());
    _rotation.setColumn(2, getAxisZ());
}

void ego::Transform::setRotationMatrix(const ComputeMatrix3x3& _rotation)
{
    m_matrix.getColumn(0).setXYZ(_rotation.getColumn(0));
    m_matrix.getColumn(1).setXYZ(_rotation.getColumn(1));
    m_matrix.getColumn(2).setXYZ(_rotation.getColumn(2));
}

ego::ComputeQuaternion ego::Transform::getRotationQuaternion() const
{
    ComputeQuaternion quaternion;
    getRotationQuaternion(quaternion);

    return quaternion;
}

void ego::Transform::getRotationQuaternion(ComputeQuaternion& _rotation) const
{
    ComputeMatrix3x3 rotationMatrix;
    getRotationMatrix(rotationMatrix);
    _rotation.setupFromRotationMatrix3x3(rotationMatrix);
}

void ego::Transform::setRotationQuaternion(const ComputeQuaternion& _rotation)
{
    setRotationMatrix(_rotation.getRotationMatrix3x3());
}