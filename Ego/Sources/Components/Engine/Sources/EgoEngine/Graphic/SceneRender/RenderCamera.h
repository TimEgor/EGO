#pragma once

#include "EgoCore/Math/Transform.h"

namespace ego::render
{
    class RenderCamera final
    {
    public:
        RenderCamera() = default;

        bool update(const Transform& _transform, float _verticalFieldOfView, float _aspectRatio, float _nearPlane, float _farPlane);
        void reset();

        const Transform& getTransform() const;
        const FloatMatrix4x4& getProjectionMatrix() const;
        const FloatMatrix4x4& getViewMatrix() const;
        const FloatMatrix4x4& getViewProjectionMatrix() const;
        const FloatMatrix4x4& getInverseViewProjectionMatrix() const;

    private:
        static FloatMatrix4x4 CreatePerspectiveProjectionMatrix(float _verticalFieldOfView, float _aspectRatio, float _nearPlane, float _farPlane);

        Transform m_transform;
        FloatMatrix4x4 m_projection = FloatMatrix4x4Identity;
        FloatMatrix4x4 m_view = FloatMatrix4x4Identity;
        FloatMatrix4x4 m_viewProjection = FloatMatrix4x4Identity;
        FloatMatrix4x4 m_inverseViewProjection = FloatMatrix4x4Identity;
    };
} // namespace ego::render
