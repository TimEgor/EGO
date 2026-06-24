#include "SceneNodeComponent.h"

ego::SceneNodeComponent::SceneNodeComponent(ecs::Entity _parent)
    : m_parent(_parent)
{
}

ego::TransformComponent::TransformComponent(const Transform& _globalTransform)
    : m_globalTransform(_globalTransform)
{
}
