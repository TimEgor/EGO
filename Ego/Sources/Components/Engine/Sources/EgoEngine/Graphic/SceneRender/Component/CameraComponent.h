#pragma once

#include "EgoCore/RTTI/RTTI.h"

#include "EgoECS/Component.h"

namespace ego::render
{
    struct CameraComponent final : public ecs::Component
    {
        CameraComponent() = default;
        explicit CameraComponent(float _verticalFieldOfView, float _nearPlane, float _farPlane);

        float m_verticalFieldOfView = 60.0f;
        float m_nearPlane = 0.1f;
        float m_farPlane = 1000.0f;

        EGO_RTTI_VIRTUAL(CameraComponent, ecs::Component);
        EGO_RTTI_PROPERTIES(
            EGO_RTTI_PROPERTY_OPTIONS(m_verticalFieldOfView, 0.1f, 0.1f, 179.9f),
            EGO_RTTI_PROPERTY_OPTIONS(m_nearPlane, 0.01f, 0.001f),
            EGO_RTTI_PROPERTY_OPTIONS(m_farPlane, 1.0f, 0.001f));
    };
} // namespace ego::render
