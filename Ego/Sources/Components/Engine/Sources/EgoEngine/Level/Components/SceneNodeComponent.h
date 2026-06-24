#pragma once

#include "EgoECS/Component.h"
#include "EgoECS/Entity.h"

#include "EgoMath/Transform.h"

namespace ego
{
    struct SceneNodeComponent final : public ecs::Component
    {
        SceneNodeComponent() = default;
        explicit SceneNodeComponent(ecs::Entity _parent);

        ecs::Entity m_parent;
        ecs::Entity m_firstChild;
        ecs::Entity m_lastChild;
        ecs::Entity m_previousSibling;
        ecs::Entity m_nextSibling;
    };

    struct TransformComponent final : public ecs::Component
    {
        TransformComponent() = default;
        explicit TransformComponent(const Transform& _globalTransform);

        Transform m_globalTransform;
    };
} // namespace ego
