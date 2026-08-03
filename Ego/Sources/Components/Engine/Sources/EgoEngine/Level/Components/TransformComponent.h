#pragma once

#include "EgoCore/Math/Transform.h"

#include "EgoECS/Component.h"

namespace ego
{
    struct TransformComponent final : public ecs::Component
    {
        TransformComponent() = default;
        explicit TransformComponent(const Transform& _globalTransform);

        Transform m_globalTransform;
    };
} // namespace ego
