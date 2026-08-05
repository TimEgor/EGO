#pragma once

#include "ComputeQuaternion.h"
#include "Matrix.h"
#include "Vector.h"

namespace ego
{
    using TransformMatrix = FloatMatrix4x4;

    struct Transform final
    {
        TransformMatrix m_matrix = FloatMatrix4x4Identity;

        Transform() = default;
        Transform(const TransformMatrix& _matrix);

        FloatVector3 getAxisX() const;
        void setAxisX(const FloatVector3& _axis);
        FloatVector3 getAxisY() const;
        void setAxisY(const FloatVector3& _axis);
        FloatVector3 getAxisZ() const;
        void setAxisZ(const FloatVector3& _axis);
        FloatVector3 getOrigin() const;
        void setOrigin(const FloatVector3& _origin);

        FloatMatrix3x3 getRotationMatrix() const;
        void getRotationMatrix(FloatMatrix3x3& _rotation) const;
        void setRotationMatrix(const FloatMatrix3x3& _rotation);

        ComputeQuaternion getRotationQuaternion() const;
        void getRotationQuaternion(ComputeQuaternion& _rotation) const;
        void setRotationQuaternion(const ComputeQuaternion& _rotation);
    };
} // namespace ego
