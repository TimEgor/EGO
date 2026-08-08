#pragma once

#include <cstddef>
#include <memory>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"

#include "Component.h"
#include "Entity.h"

namespace ego::ecs::detail
{
    class WorldImplementation;
}

namespace ego::ecs
{
    class World final : public NonCopyable
    {
    public:
        World();
        ~World() override;

        Entity createEntity();
        void destroyEntity(Entity _entity);
        bool isEntityAlive(Entity _entity) const;

        void clear();
        size_t getEntityCount() const;

        template <typename TComponent, typename... Args>
        TComponent& addComponent(Entity _entity, Args&&... _args);

        template <typename TComponent, typename... Args>
        TComponent& addOrReplaceComponent(Entity _entity, Args&&... _args);

        template <typename TComponent>
        bool hasComponent(Entity _entity) const;

        template <typename TComponent>
        TComponent* tryGetComponent(Entity _entity);

        template <typename TComponent>
        const TComponent* tryGetComponent(Entity _entity) const;

        template <typename TComponent>
        TComponent& getComponent(Entity _entity);

        template <typename TComponent>
        const TComponent& getComponent(Entity _entity) const;

        bool removeComponent(Entity _entity, ComponentTypeID _componentTypeID);

        template <typename TComponent>
        bool removeComponent(Entity _entity);

        template <typename TFunction>
        void forEachComponent(Entity _entity, TFunction&& _function);

        template <typename... TComponents, typename TFunction>
        void forEach(TFunction&& _function);

        template <typename... TComponents, typename TFunction>
        void forEach(TFunction&& _function) const;

    private:
        static WorldID AllocateWorldID();

        WorldID m_id = InvalidWorldID;
        std::unique_ptr<detail::WorldImplementation> m_implementation;
    };

    EGO_POINTER(World);
    EGO_WEAK_POINTER(World);
} // namespace ego::ecs

#include "World.hpp"
