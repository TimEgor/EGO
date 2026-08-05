#pragma once

#include <utility>

template <typename TComponent, typename... Args>
TComponent* ego::Level::addOrReplaceComponent(ecs::Entity _entity, Args&&... _args)
{
    if (!m_world || !ownsEntity(_entity))
    {
        return nullptr;
    }

    return &m_world->addOrReplaceComponent<TComponent>(_entity, std::forward<Args>(_args)...);
}

template <typename TComponent>
TComponent* ego::Level::tryGetComponent(ecs::Entity _entity)
{
    if (!m_world || !ownsEntity(_entity))
    {
        return nullptr;
    }

    return m_world->tryGetComponent<TComponent>(_entity);
}

template <typename TComponent>
const TComponent* ego::Level::tryGetComponent(ecs::Entity _entity) const
{
    if (!m_world || !ownsEntity(_entity))
    {
        return nullptr;
    }

    return m_world->tryGetComponent<TComponent>(_entity);
}

template <typename TFunction>
void ego::Level::forEachComponent(ecs::Entity _entity, TFunction&& _function)
{
    if (!m_world || !ownsEntity(_entity))
    {
        return;
    }

    m_world->forEachComponent(_entity, std::forward<TFunction>(_function));
}

template <typename... TComponents, typename TFunction>
void ego::Level::forEachComponent(TFunction&& _function)
{
    if (!m_world)
    {
        return;
    }

    m_world->forEach<TComponents...>(std::forward<TFunction>(_function));
}

template <typename... TComponents, typename TFunction>
void ego::Level::forEachComponent(TFunction&& _function) const
{
    if (!m_world)
    {
        return;
    }

    m_world->forEach<TComponents...>(std::forward<TFunction>(_function));
}
