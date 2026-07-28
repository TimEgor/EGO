#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Pointer/Pointer.h"

#include "EgoECS/Entity.h"
#include "EgoECS/World.h"

#include "Components/LevelComponent.h"
#include "Components/SceneNodeComponent.h"

namespace ego
{
    class LevelController;

    class Level;
    EGO_POINTER(Level);
    EGO_WEAK_POINTER(Level);

    struct LevelDeleter final
    {
        explicit LevelDeleter(const WeakPointer<LevelController>& _controller);

        void operator()(Level* _level) const;

    private:
        WeakPointer<LevelController> m_controller;
    };

    class Level final : public NonCopyable
    {
    public:
        class LevelAccessor final : public NonInstanceable
        {
            friend class LevelController;
            friend struct LevelDeleter;

            static void Destroy(Level* _level);
        };

        class NodeChildIterator final
        {
        public:
            NodeChildIterator() = default;
            NodeChildIterator(const Level* _level, ecs::Entity _node);

            ecs::Entity operator*() const;
            NodeChildIterator& operator++();
            NodeChildIterator operator++(int);

            bool operator==(const NodeChildIterator& _iterator) const;
            bool operator!=(const NodeChildIterator& _iterator) const;

        private:
            const Level* m_level = nullptr;
            ecs::Entity m_node;
            ecs::Entity m_nextNode;
        };

        class NodeChildRange final
        {
        public:
            NodeChildRange() = default;
            NodeChildRange(const Level* _level, ecs::Entity _parent);

            NodeChildIterator begin() const;
            NodeChildIterator end() const;

        private:
            const Level* m_level = nullptr;
            ecs::Entity m_parent;
        };

        explicit Level(LevelID _id);
        ~Level() override;

        LevelID getID() const;
        bool isValid() const;

        ecs::Entity createEntity();
        ecs::Entity createEntity(const char* _name);
        void destroyEntity(ecs::Entity _entity);
        bool ownsEntity(ecs::Entity _entity) const;

        template <typename TComponent, typename... Args>
        TComponent* addOrReplaceComponent(ecs::Entity _entity, Args&&... _args);

        template <typename TComponent>
        TComponent* tryGetComponent(ecs::Entity _entity);

        template <typename TComponent>
        const TComponent* tryGetComponent(ecs::Entity _entity) const;

        template <typename... TComponents, typename TFunction>
        void forEachComponent(TFunction&& _function);

        template <typename... TComponents, typename TFunction>
        void forEachComponent(TFunction&& _function) const;

        size_t getEntityCount() const;

        ecs::Entity getRootNode() const;
        bool isNode(ecs::Entity _entity) const;

        ecs::Entity createNode();
        ecs::Entity createNode(ecs::Entity _parent);
        bool destroyNode(ecs::Entity _node);

        bool setNodeParent(ecs::Entity _node, ecs::Entity _parent);
        ecs::Entity getNodeParent(ecs::Entity _node) const;
        NodeChildRange getNodeChildren(ecs::Entity _node) const;
        ecs::Entity getFirstNodeChild(ecs::Entity _node) const;
        ecs::Entity getLastNodeChild(ecs::Entity _node) const;
        ecs::Entity getPreviousNodeSibling(ecs::Entity _node) const;
        ecs::Entity getNextNodeSibling(ecs::Entity _node) const;

        void clear();

    private:
        void release();
        void attachEntity(ecs::Entity _entity);
        ecs::Entity createRootNode();

        void collectNodeHierarchy(ecs::Entity _node, std::vector<ecs::Entity>& _nodes) const;
        bool isNodeChildOf(ecs::Entity _node, ecs::Entity _possibleParent) const;
        bool attachNodeToParent(ecs::Entity _node, ecs::Entity _parent);
        void detachNodeFromParent(ecs::Entity _node);

        ecs::WorldPointer m_world = nullptr;
        ecs::Entity m_rootNode;
        LevelID m_id = InvalidLevelID;
    };
} // namespace ego

template <typename TComponent, typename... Args>
TComponent* ego::Level::addOrReplaceComponent(ego::ecs::Entity _entity, Args&&... _args)
{
    if (!m_world || !ownsEntity(_entity))
    {
        return nullptr;
    }

    return &m_world->addOrReplaceComponent<TComponent>(_entity, std::forward<Args>(_args)...);
}

template <typename TComponent>
TComponent* ego::Level::tryGetComponent(ego::ecs::Entity _entity)
{
    if (!m_world || !ownsEntity(_entity))
    {
        return nullptr;
    }

    return m_world->tryGetComponent<TComponent>(_entity);
}

template <typename TComponent>
const TComponent* ego::Level::tryGetComponent(ego::ecs::Entity _entity) const
{
    if (!m_world || !ownsEntity(_entity))
    {
        return nullptr;
    }

    return m_world->tryGetComponent<TComponent>(_entity);
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
