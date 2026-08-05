#pragma once

#include "ComputeMath.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Vector.h"

namespace ego
{
    using TransformValue = ComputeValue;
    using TransformVector = Vector3Base<TransformValue>;
    using TransformColumn = Vector4Base<TransformValue>;
    using TransformLinearMatrix = Matrix3x3Base<TransformValue>;
    using TransformMatrix = Matrix4x4Base<TransformValue>;
    using TransformQuaternion = QuaternionBase<TransformValue>;

    inline constexpr TransformMatrix TransformMatrixIdentity = TransformMatrix(
        TransformColumn(TransformValue(1.0), TransformValue(0.0), TransformValue(0.0), TransformValue(0.0)),
        TransformColumn(TransformValue(0.0), TransformValue(1.0), TransformValue(0.0), TransformValue(0.0)),
        TransformColumn(TransformValue(0.0), TransformValue(0.0), TransformValue(1.0), TransformValue(0.0)),
        TransformColumn(TransformValue(0.0), TransformValue(0.0), TransformValue(0.0), TransformValue(1.0)));

    inline constexpr TransformQuaternion TransformQuaternionIdentity =
        TransformQuaternion(TransformValue(0.0), TransformValue(0.0), TransformValue(0.0), TransformValue(1.0));

    class Transform final
    {
    public:
        Transform() = default;
        explicit Transform(const TransformMatrix& _matrix);

        const TransformMatrix& getMatrix() const;
        void setMatrix(const TransformMatrix& _matrix);

        TransformVector getAxisX() const;
        void setAxisX(const TransformVector& _axis);
        TransformVector getAxisY() const;
        void setAxisY(const TransformVector& _axis);
        TransformVector getAxisZ() const;
        void setAxisZ(const TransformVector& _axis);
        TransformVector getOrigin() const;
        void setOrigin(const TransformVector& _origin);

        TransformLinearMatrix getLinearMatrix() const;
        void getLinearMatrix(TransformLinearMatrix& _linearMatrix) const;
        void setLinearMatrix(const TransformLinearMatrix& _linearMatrix);

        TransformLinearMatrix getRotationMatrix() const;
        void getRotationMatrix(TransformLinearMatrix& _rotation) const;
        void setRotationMatrix(const TransformLinearMatrix& _rotation);

        TransformQuaternion getRotationQuaternion() const;
        void getRotationQuaternion(TransformQuaternion& _rotation) const;
        bool tryGetRotationQuaternion(TransformQuaternion& _rotation) const;
        void setRotationQuaternion(const TransformQuaternion& _rotation);

    private:
        TransformMatrix m_matrix = TransformMatrixIdentity;
    };
} // namespace ego
