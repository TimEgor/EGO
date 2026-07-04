#pragma once

#include <cstddef>

#include "EgoECS/Entity.h"

#include <entt/entity/registry.hpp>

namespace ego::ecs::detail
{
    inline Entity ToEntity(entt::entity _entity)
    {
        return Entity(static_cast<EntityID>(entt::to_integral(_entity)));
    }

    inline entt::entity ToNativeEntity(Entity _entity)
    {
        return static_cast<entt::entity>(_entity.getID());
    }

    class WorldImplementation final
    {
    public:
        entt::registry m_registry;
        size_t m_entityCount = 0;
    };
} // namespace ego::ecs::detail
