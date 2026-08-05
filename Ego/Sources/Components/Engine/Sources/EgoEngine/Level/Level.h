#pragma once

#include <cstddef>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"

#include "EgoECS/Entity.h"
#include "EgoECS/World.h"

#include "Components/NameComponent.h"
#include "Components/TransformComponent.h"

namespace ego
{
    class Level;
    EGO_POINTER(Level);
    EGO_WEAK_POINTER(Level);

    class Level final : public NonCopyable
    {
    public:
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
            NodeChildRange(const Level* _level, ecs::Entity _firstNode);

            NodeChildIterator begin() const;
            NodeChildIterator end() const;

        private:
            const Level* m_level = nullptr;
            ecs::Entity m_firstNode;
        };

        Level();
        ~Level() override;

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

        template <typename TFunction>
        void forEachComponent(ecs::Entity _entity, TFunction&& _function);

        template <typename... TComponents, typename TFunction>
        void forEachComponent(TFunction&& _function);

        template <typename... TComponents, typename TFunction>
        void forEachComponent(TFunction&& _function) const;

        size_t getEntityCount() const;

        bool isNode(ecs::Entity _entity) const;

        ecs::Entity createNode();
        ecs::Entity createNode(ecs::Entity _parent);
        bool destroyNode(ecs::Entity _node);

        bool setNodeParent(ecs::Entity _node, ecs::Entity _parent);
        ecs::Entity getNodeParent(ecs::Entity _node) const;
        NodeChildRange getRootNodes() const;
        NodeChildRange getNodeChildren(ecs::Entity _node) const;
        ecs::Entity getFirstNodeChild(ecs::Entity _node) const;
        ecs::Entity getLastNodeChild(ecs::Entity _node) const;
        ecs::Entity getPreviousNodeSibling(ecs::Entity _node) const;
        ecs::Entity getNextNodeSibling(ecs::Entity _node) const;

        void clear();

    private:
        struct HierarchyComponent;

        void release();

        bool canSetNodeParent(ecs::Entity _node, ecs::Entity _parent) const;
        bool isNodeChildOf(ecs::Entity _node, ecs::Entity _possibleParent) const;
        void linkNodeToParentEnd(ecs::Entity _node, ecs::Entity _parent);
        void unlinkNode(ecs::Entity _node);

        ecs::WorldPointer m_world = nullptr;
        ecs::Entity m_firstRootNode;
        ecs::Entity m_lastRootNode;
    };
} // namespace ego

#include "Level.hpp"
