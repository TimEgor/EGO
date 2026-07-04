#pragma once

#include "EgoECS/Component.h"

#include "EgoMath/ComputeMatrix44.h"

namespace ego::render
{
    struct CameraComponent final : public ecs::Component
    {
        CameraComponent() = default;
        explicit CameraComponent(const ComputeMatrix4x4& _projection);

        ComputeMatrix4x4 m_projection = ComputeMatrix4x4Identity;
    };
} // namespace ego::render
