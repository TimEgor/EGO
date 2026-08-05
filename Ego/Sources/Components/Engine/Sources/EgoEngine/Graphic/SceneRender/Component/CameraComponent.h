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
        EGO_RTTI_PROPERTIES(EGO_RTTI_PROPERTY(m_verticalFieldOfView), EGO_RTTI_PROPERTY(m_nearPlane), EGO_RTTI_PROPERTY(m_farPlane));
    };
} // namespace ego::render
