#pragma once

#include <functional>
#include <type_traits>
#include <utility>

#include "EgoCore/Assert/Assert.h"

#include "Detail/WorldImplementation.h"

namespace ego::ecs
{
    namespace detail
    {
        template <typename TComponent>
        inline constexpr bool IsComponentType =
            std::is_base_of_v<Component, std::remove_cv_t<TComponent>> && !std::is_same_v<Component, std::remove_cv_t<TComponent>>;
    }

    template <typename TComponent, typename... Args>
    TComponent& World::addComponent(Entity _entity, Args&&... _args)
    {
        static_assert(std::is_object_v<TComponent>);
        static_assert(!std::is_const_v<TComponent>);
        static_assert(detail::IsComponentType<TComponent>, "ECS component must inherit from ego::ecs::Component.");

        EGO_ASSERT(isEntityAlive(_entity));

        m_implementation->registerComponent<TComponent>();

        return m_implementation->m_registry.emplace<TComponent>(detail::ToNativeEntity(_entity), std::forward<Args>(_args)...);
    }

    template <typename TComponent, typename... Args>
    TComponent& World::addOrReplaceComponent(Entity _entity, Args&&... _args)
    {
        static_assert(std::is_object_v<TComponent>);
        static_assert(!std::is_const_v<TComponent>);
        static_assert(detail::IsComponentType<TComponent>, "ECS component must inherit from ego::ecs::Component.");

        EGO_ASSERT(isEntityAlive(_entity));

        m_implementation->registerComponent<TComponent>();

        return m_implementation->m_registry.emplace_or_replace<TComponent>(detail::ToNativeEntity(_entity), std::forward<Args>(_args)...);
    }

    template <typename TComponent>
    bool World::hasComponent(Entity _entity) const
    {
        static_assert(std::is_object_v<TComponent>);
        static_assert(detail::IsComponentType<TComponent>, "ECS component must inherit from ego::ecs::Component.");

        if (!isEntityAlive(_entity))
        {
            return false;
        }

        return m_implementation->m_registry.all_of<TComponent>(detail::ToNativeEntity(_entity));
    }

    template <typename TComponent>
    TComponent* World::tryGetComponent(Entity _entity)
    {
        static_assert(std::is_object_v<TComponent>);
        static_assert(!std::is_const_v<TComponent>);
        static_assert(detail::IsComponentType<TComponent>, "ECS component must inherit from ego::ecs::Component.");

        if (!isEntityAlive(_entity))
        {
            return nullptr;
        }

        return m_implementation->m_registry.try_get<TComponent>(detail::ToNativeEntity(_entity));
    }

    template <typename TComponent>
    const TComponent* World::tryGetComponent(Entity _entity) const
    {
        static_assert(std::is_object_v<TComponent>);
        static_assert(detail::IsComponentType<TComponent>, "ECS component must inherit from ego::ecs::Component.");

        if (!isEntityAlive(_entity))
        {
            return nullptr;
        }

        return m_implementation->m_registry.try_get<TComponent>(detail::ToNativeEntity(_entity));
    }

    template <typename TComponent>
    TComponent& World::getComponent(Entity _entity)
    {
        TComponent* component = tryGetComponent<TComponent>(_entity);
        EGO_ASSERT(component);

        return *component;
    }

    template <typename TComponent>
    const TComponent& World::getComponent(Entity _entity) const
    {
        const TComponent* component = tryGetComponent<TComponent>(_entity);
        EGO_ASSERT(component);

        return *component;
    }

    template <typename TComponent>
    bool World::removeComponent(Entity _entity)
    {
        static_assert(std::is_object_v<TComponent>);
        static_assert(!std::is_const_v<TComponent>);
        static_assert(detail::IsComponentType<TComponent>, "ECS component must inherit from ego::ecs::Component.");

        if (!isEntityAlive(_entity))
        {
            return false;
        }

        return m_implementation->m_registry.remove<TComponent>(detail::ToNativeEntity(_entity)) > 0;
    }

    template <typename TFunction>
    void World::forEachComponent(Entity _entity, TFunction&& _function)
    {
        if (!isEntityAlive(_entity))
        {
            return;
        }

        const entt::entity nativeEntity = detail::ToNativeEntity(_entity);
        for (const auto& [componentTypeID, componentResolver] : m_implementation->m_componentResolvers)
        {
            Component* component = componentResolver(m_implementation->m_registry, nativeEntity);
            if (!component)
            {
                continue;
            }

            std::invoke(_function, componentTypeID, *component);
        }
    }

    template <typename... TComponents, typename TFunction>
    void World::forEach(TFunction&& _function)
    {
        static_assert((detail::IsComponentType<TComponents> && ...), "Each ECS component must inherit from ego::ecs::Component.");

        auto view = m_implementation->m_registry.view<TComponents...>();
        auto&& function = _function;
        const WorldID worldID = m_id;

        view.each(
            [&function, worldID](entt::entity _nativeEntity, TComponents&... _components)
            {
                std::invoke(function, detail::ToEntity(worldID, _nativeEntity), _components...);
            });
    }

    template <typename... TComponents, typename TFunction>
    void World::forEach(TFunction&& _function) const
    {
        static_assert((detail::IsComponentType<std::remove_const_t<TComponents>> && ...), "Each ECS component must inherit from ego::ecs::Component.");

        auto view = m_implementation->m_registry.view<const std::remove_const_t<TComponents>...>();
        auto&& function = _function;
        const WorldID worldID = m_id;

        view.each(
            [&function, worldID](entt::entity _nativeEntity, const std::remove_const_t<TComponents>&... _components)
            {
                std::invoke(function, detail::ToEntity(worldID, _nativeEntity), _components...);
            });
    }
} // namespace ego::ecs
