#include "World.h"

#include <atomic>

#include "Detail/WorldImplementation.h"

ego::ecs::World::World()
    : m_id(AllocateWorldID()),
      m_implementation(std::make_unique<detail::WorldImplementation>())
{
}

ego::ecs::World::~World() = default;

ego::ecs::Entity ego::ecs::World::createEntity()
{
    const Entity entity = detail::ToEntity(m_id, m_implementation->m_registry.create());
    ++m_implementation->m_entityCount;

    return entity;
}

void ego::ecs::World::destroyEntity(Entity _entity)
{
    if (!isEntityAlive(_entity))
    {
        return;
    }

    m_implementation->m_registry.destroy(detail::ToNativeEntity(_entity));
    --m_implementation->m_entityCount;
}

bool ego::ecs::World::isEntityAlive(Entity _entity) const
{
    return _entity.getWorldID() == m_id && _entity.isValid() && m_implementation->m_registry.valid(detail::ToNativeEntity(_entity));
}

void ego::ecs::World::clear()
{
    m_implementation->m_registry.clear();
    m_implementation->m_entityCount = 0;
}

size_t ego::ecs::World::getEntityCount() const
{
    return m_implementation->m_entityCount;
}

bool ego::ecs::World::removeComponent(Entity _entity, ComponentTypeID _componentTypeID)
{
    if (!isEntityAlive(_entity) || _componentTypeID == InvalidComponentTypeID)
    {
        return false;
    }

    const entt::entity nativeEntity = detail::ToNativeEntity(_entity);
    auto* componentStorage = m_implementation->m_registry.storage(_componentTypeID);

    return componentStorage && componentStorage->remove(nativeEntity);
}

ego::ecs::WorldID ego::ecs::World::AllocateWorldID()
{
    static std::atomic<WorldID> nextWorldID = FirstWorldID;

    WorldID worldID = InvalidWorldID;
    while (worldID == InvalidWorldID)
    {
        worldID = nextWorldID.fetch_add(1, std::memory_order_relaxed);
    }

    return worldID;
}
