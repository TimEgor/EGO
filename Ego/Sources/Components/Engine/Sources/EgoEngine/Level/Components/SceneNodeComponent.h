#pragma once

#include "EgoECS/Component.h"
#include "EgoECS/Entity.h"

#include "EgoMath/Transform.h"

#include <vector>

namespace ego
{
    struct SceneNodeComponent final : public ecs::Component
    {
        SceneNodeComponent() = default;
        explicit SceneNodeComponent(ecs::Entity _parent);

        ecs::Entity m_parent;
        std::vector<ecs::Entity> m_children;
    };

    struct TransformComponent final : public ecs::Component
    {
        TransformComponent() = default;
        explicit TransformComponent(const Transform& _globalTransform);

        Transform m_globalTransform;
    };
}
