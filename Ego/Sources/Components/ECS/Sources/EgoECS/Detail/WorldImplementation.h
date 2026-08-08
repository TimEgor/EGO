#pragma once

#include <cstddef>
#include <unordered_map>

#include "EgoECS/Component.h"
#include "EgoECS/Entity.h"

#include <entt/entity/registry.hpp>

namespace ego::ecs::detail
{
    inline Entity ToEntity(WorldID _worldID, entt::entity _entity)
    {
        return Entity(_worldID, static_cast<EntityID>(entt::to_integral(_entity)));
    }

    inline entt::entity ToNativeEntity(Entity _entity)
    {
        return static_cast<entt::entity>(_entity.getID());
    }

    class WorldImplementation final
    {
    public:
        using ComponentResolver = Component* (*)(entt::registry & _registry, entt::entity _entity);
        using ComponentResolverCollection = std::unordered_map<ComponentTypeID, ComponentResolver>;

        template <typename TComponent>
        void registerComponent()
        {
            m_componentResolvers.try_emplace(
                GetComponentTypeID<TComponent>(),
                [](entt::registry& _registry, entt::entity _entity) -> Component*
                {
                    return _registry.try_get<TComponent>(_entity);
                });
        }

        entt::registry m_registry;
        ComponentResolverCollection m_componentResolvers;
        size_t m_entityCount = 0;
    };
} // namespace ego::ecs::detail
