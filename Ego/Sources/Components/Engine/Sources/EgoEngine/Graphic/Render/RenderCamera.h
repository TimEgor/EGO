#pragma once

#include "EgoMath/Transform.h"

namespace ego
{
    class RenderCamera final
    {
    public:
        RenderCamera() = default;

        const Transform& getTransform() const;
        void setTransform(const Transform& _transform);

        const ComputeMatrix4x4& getProjectionMatrix() const;
        void setProjectionMatrix(const ComputeMatrix4x4& _projection);

        ComputeMatrix4x4 getViewMatrix() const;
        ComputeMatrix4x4 getViewProjectionMatrix() const;

    private:
        Transform m_transform;
        ComputeMatrix4x4 m_projection = ComputeMatrix4x4Identity;
    };
}
