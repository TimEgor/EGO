#pragma once

#include "EgoCore/Math/ComputeMatrix44.h"

#include "EgoECS/Component.h"

namespace ego::render
{
    struct CameraComponent final : public ecs::Component
    {
        CameraComponent() = default;
        explicit CameraComponent(const ComputeMatrix4x4& _projection);

        ComputeMatrix4x4 m_projection = ComputeMatrix4x4Identity;
    };
} // namespace ego::render
