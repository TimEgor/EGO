#include "World.h"

#include "Detail/WorldImplementation.h"

ego::ecs::World::World()
    : m_implementation(std::make_unique<detail::WorldImplementation>())
{
}

ego::ecs::World::~World() = default;

ego::ecs::Entity ego::ecs::World::createEntity()
{
    const Entity entity = detail::ToEntity(m_implementation->m_registry.create());
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
    return _entity.isValid() && m_implementation->m_registry.valid(detail::ToNativeEntity(_entity));
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
