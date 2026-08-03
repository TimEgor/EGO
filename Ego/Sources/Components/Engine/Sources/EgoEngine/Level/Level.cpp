#include "Level.h"

#include "EgoCore/Assert/Assert.h"

struct ego::Level::HierarchyComponent final : public ecs::Component
{
    ecs::Entity m_parent;
    ecs::Entity m_firstChild;
    ecs::Entity m_lastChild;
    ecs::Entity m_previousSibling;
    ecs::Entity m_nextSibling;
};

ego::Level::Level()
    : m_world(MakePointer<ecs::World>())
{
}

ego::Level::~Level()
{
    release();
}

ego::Level::NodeChildIterator::NodeChildIterator(const Level* _level, ecs::Entity _node)
    : m_level(_level && _node ? _level : nullptr),
      m_node(_level ? _node : ecs::Entity()),
      m_nextNode(m_level ? m_level->getNextNodeSibling(m_node) : ecs::Entity())
{
}

ego::ecs::Entity ego::Level::NodeChildIterator::operator*() const
{
    return m_node;
}

ego::Level::NodeChildIterator& ego::Level::NodeChildIterator::operator++()
{
    if (m_level && m_node)
    {
        m_node = m_nextNode;
        m_nextNode = m_level && m_node ? m_level->getNextNodeSibling(m_node) : ecs::Entity();

        if (!m_node)
        {
            m_level = nullptr;
        }
    }

    return *this;
}

ego::Level::NodeChildIterator ego::Level::NodeChildIterator::operator++(int)
{
    NodeChildIterator iterator = *this;
    ++(*this);

    return iterator;
}

bool ego::Level::NodeChildIterator::operator==(const NodeChildIterator& _iterator) const
{
    return m_node == _iterator.m_node;
}

bool ego::Level::NodeChildIterator::operator!=(const NodeChildIterator& _iterator) const
{
    return !(*this == _iterator);
}

ego::Level::NodeChildRange::NodeChildRange(const Level* _level, ecs::Entity _firstNode)
    : m_level(_level),
      m_firstNode(_level ? _firstNode : ecs::Entity())
{
}

ego::Level::NodeChildIterator ego::Level::NodeChildRange::begin() const
{
    return m_level ? NodeChildIterator(m_level, m_firstNode) : NodeChildIterator();
}

ego::Level::NodeChildIterator ego::Level::NodeChildRange::end() const
{
    return NodeChildIterator();
}

bool ego::Level::isValid() const
{
    return static_cast<bool>(m_world);
}

ego::ecs::Entity ego::Level::createEntity()
{
    if (!m_world)
    {
        EGO_ASSERT_FAIL_MESSAGE("Level isn't valid.");

        return ecs::Entity();
    }

    const ecs::Entity entity = m_world->createEntity();

    return entity;
}

ego::ecs::Entity ego::Level::createEntity(const char* _name)
{
    const ecs::Entity entity = createEntity();
    if (entity && m_world && _name && _name[0] != '\0')
    {
        m_world->addOrReplaceComponent<NameComponent>(entity, NameComponent{_name});
    }

    return entity;
}

void ego::Level::destroyEntity(ecs::Entity _entity)
{
    if (!m_world || !ownsEntity(_entity))
    {
        return;
    }

    if (isNode(_entity))
    {
        destroyNode(_entity);

        return;
    }

    m_world->destroyEntity(_entity);
}

bool ego::Level::ownsEntity(ecs::Entity _entity) const
{
    return m_world && m_world->isEntityAlive(_entity);
}

size_t ego::Level::getEntityCount() const
{
    return m_world ? m_world->getEntityCount() : 0;
}

bool ego::Level::isNode(ecs::Entity _entity) const
{
    return m_world && ownsEntity(_entity) && m_world->hasComponent<HierarchyComponent>(_entity) && m_world->hasComponent<TransformComponent>(_entity);
}

ego::ecs::Entity ego::Level::createNode()
{
    return createNode(ecs::Entity());
}

ego::ecs::Entity ego::Level::createNode(ecs::Entity _parent)
{
    if (!m_world || (_parent && !isNode(_parent)))
    {
        return ecs::Entity();
    }

    const ecs::Entity node = createEntity();
    if (!node)
    {
        return ecs::Entity();
    }

    m_world->addOrReplaceComponent<HierarchyComponent>(node);
    m_world->addOrReplaceComponent<TransformComponent>(node);
    linkNodeToParentEnd(node, _parent);

    return node;
}

bool ego::Level::destroyNode(ecs::Entity _node)
{
    if (!m_world || !isNode(_node))
    {
        return false;
    }

    ecs::Entity currentNode = _node;
    while (currentNode)
    {
        const HierarchyComponent* hierarchyComponent = m_world->tryGetComponent<HierarchyComponent>(currentNode);
        EGO_ASSERT(hierarchyComponent);
        if (!hierarchyComponent)
        {
            return false;
        }

        if (hierarchyComponent->m_firstChild)
        {
            currentNode = hierarchyComponent->m_firstChild;

            continue;
        }

        const ecs::Entity parent = hierarchyComponent->m_parent;
        const ecs::Entity nextSibling = hierarchyComponent->m_nextSibling;
        const bool isSubtreeRoot = currentNode == _node;

        unlinkNode(currentNode);
        m_world->destroyEntity(currentNode);

        if (isSubtreeRoot)
        {
            break;
        }

        currentNode = nextSibling ? nextSibling : parent;
    }

    return true;
}

bool ego::Level::setNodeParent(ecs::Entity _node, ecs::Entity _parent)
{
    if (!canSetNodeParent(_node, _parent))
    {
        return false;
    }

    const ecs::Entity oldParent = getNodeParent(_node);
    if (oldParent == _parent)
    {
        return true;
    }

    unlinkNode(_node);
    linkNodeToParentEnd(_node, _parent);

    return true;
}

ego::ecs::Entity ego::Level::getNodeParent(ecs::Entity _node) const
{
    if (!m_world || !isNode(_node))
    {
        return ecs::Entity();
    }

    const HierarchyComponent* nodeComponent = m_world->tryGetComponent<HierarchyComponent>(_node);

    return nodeComponent ? nodeComponent->m_parent : ecs::Entity();
}

ego::Level::NodeChildRange ego::Level::getRootNodes() const
{
    return m_world ? NodeChildRange(this, m_firstRootNode) : NodeChildRange();
}

ego::Level::NodeChildRange ego::Level::getNodeChildren(ecs::Entity _node) const
{
    if (!m_world || !isNode(_node))
    {
        return NodeChildRange();
    }

    return NodeChildRange(this, getFirstNodeChild(_node));
}

ego::ecs::Entity ego::Level::getFirstNodeChild(ecs::Entity _node) const
{
    if (!m_world || !isNode(_node))
    {
        return ecs::Entity();
    }

    const HierarchyComponent* nodeComponent = m_world->tryGetComponent<HierarchyComponent>(_node);

    return nodeComponent ? nodeComponent->m_firstChild : ecs::Entity();
}

ego::ecs::Entity ego::Level::getLastNodeChild(ecs::Entity _node) const
{
    if (!m_world || !isNode(_node))
    {
        return ecs::Entity();
    }

    const HierarchyComponent* nodeComponent = m_world->tryGetComponent<HierarchyComponent>(_node);

    return nodeComponent ? nodeComponent->m_lastChild : ecs::Entity();
}

ego::ecs::Entity ego::Level::getPreviousNodeSibling(ecs::Entity _node) const
{
    if (!m_world || !isNode(_node))
    {
        return ecs::Entity();
    }

    const HierarchyComponent* nodeComponent = m_world->tryGetComponent<HierarchyComponent>(_node);

    return nodeComponent ? nodeComponent->m_previousSibling : ecs::Entity();
}

ego::ecs::Entity ego::Level::getNextNodeSibling(ecs::Entity _node) const
{
    if (!m_world || !isNode(_node))
    {
        return ecs::Entity();
    }

    const HierarchyComponent* nodeComponent = m_world->tryGetComponent<HierarchyComponent>(_node);

    return nodeComponent ? nodeComponent->m_nextSibling : ecs::Entity();
}

void ego::Level::clear()
{
    if (!m_world)
    {
        return;
    }

    m_world->clear();
    m_firstRootNode = ecs::Entity();
    m_lastRootNode = ecs::Entity();
}

void ego::Level::release()
{
    if (m_world)
    {
        m_world->clear();
    }

    m_firstRootNode = ecs::Entity();
    m_lastRootNode = ecs::Entity();
    m_world = nullptr;
}

bool ego::Level::canSetNodeParent(ecs::Entity _node, ecs::Entity _parent) const
{
    return m_world && isNode(_node) && (!_parent || isNode(_parent)) && _node != _parent && (!_parent || !isNodeChildOf(_parent, _node));
}

bool ego::Level::isNodeChildOf(ecs::Entity _node, ecs::Entity _possibleParent) const
{
    if (!m_world || !isNode(_node) || !isNode(_possibleParent))
    {
        return false;
    }

    ecs::Entity parent = getNodeParent(_node);
    while (parent)
    {
        if (parent == _possibleParent)
        {
            return true;
        }

        parent = getNodeParent(parent);
    }

    return false;
}

void ego::Level::linkNodeToParentEnd(ecs::Entity _node, ecs::Entity _parent)
{
    EGO_ASSERT(m_world && isNode(_node) && (!_parent || isNode(_parent)) && _node != _parent);

    HierarchyComponent* nodeComponent = m_world->tryGetComponent<HierarchyComponent>(_node);
    EGO_ASSERT(nodeComponent);
    EGO_ASSERT(!nodeComponent->m_parent && !nodeComponent->m_previousSibling && !nodeComponent->m_nextSibling);
    EGO_ASSERT(m_firstRootNode != _node && m_lastRootNode != _node);

    if (_parent)
    {
        HierarchyComponent* parentComponent = m_world->tryGetComponent<HierarchyComponent>(_parent);
        EGO_ASSERT(parentComponent);

        if (parentComponent->m_lastChild)
        {
            HierarchyComponent* lastChildComponent = m_world->tryGetComponent<HierarchyComponent>(parentComponent->m_lastChild);
            EGO_ASSERT(lastChildComponent && !lastChildComponent->m_nextSibling);

            lastChildComponent->m_nextSibling = _node;
            nodeComponent->m_previousSibling = parentComponent->m_lastChild;
        }
        else
        {
            EGO_ASSERT(!parentComponent->m_firstChild);
            parentComponent->m_firstChild = _node;
        }

        parentComponent->m_lastChild = _node;
        nodeComponent->m_parent = _parent;
    }
    else
    {
        if (m_lastRootNode)
        {
            HierarchyComponent* lastRootComponent = m_world->tryGetComponent<HierarchyComponent>(m_lastRootNode);
            EGO_ASSERT(lastRootComponent && !lastRootComponent->m_nextSibling);

            lastRootComponent->m_nextSibling = _node;
            nodeComponent->m_previousSibling = m_lastRootNode;
        }
        else
        {
            EGO_ASSERT(!m_firstRootNode);
            m_firstRootNode = _node;
        }

        m_lastRootNode = _node;
    }
}

void ego::Level::unlinkNode(ecs::Entity _node)
{
    EGO_ASSERT(m_world && isNode(_node));

    HierarchyComponent* nodeComponent = m_world->tryGetComponent<HierarchyComponent>(_node);
    EGO_ASSERT(nodeComponent);

    if (nodeComponent->m_parent)
    {
        HierarchyComponent* parentComponent = m_world->tryGetComponent<HierarchyComponent>(nodeComponent->m_parent);
        EGO_ASSERT(parentComponent);
        EGO_ASSERT(nodeComponent->m_previousSibling || parentComponent->m_firstChild == _node);
        EGO_ASSERT(nodeComponent->m_nextSibling || parentComponent->m_lastChild == _node);

        if (parentComponent->m_firstChild == _node)
        {
            parentComponent->m_firstChild = nodeComponent->m_nextSibling;
        }

        if (parentComponent->m_lastChild == _node)
        {
            parentComponent->m_lastChild = nodeComponent->m_previousSibling;
        }
    }
    else
    {
        EGO_ASSERT(nodeComponent->m_previousSibling || m_firstRootNode == _node);
        EGO_ASSERT(nodeComponent->m_nextSibling || m_lastRootNode == _node);

        if (m_firstRootNode == _node)
        {
            m_firstRootNode = nodeComponent->m_nextSibling;
        }

        if (m_lastRootNode == _node)
        {
            m_lastRootNode = nodeComponent->m_previousSibling;
        }
    }

    if (nodeComponent->m_previousSibling)
    {
        HierarchyComponent* previousSiblingComponent = m_world->tryGetComponent<HierarchyComponent>(nodeComponent->m_previousSibling);
        EGO_ASSERT(previousSiblingComponent);
        previousSiblingComponent->m_nextSibling = nodeComponent->m_nextSibling;
    }

    if (nodeComponent->m_nextSibling)
    {
        HierarchyComponent* nextSiblingComponent = m_world->tryGetComponent<HierarchyComponent>(nodeComponent->m_nextSibling);
        EGO_ASSERT(nextSiblingComponent);
        nextSiblingComponent->m_previousSibling = nodeComponent->m_previousSibling;
    }

    nodeComponent->m_parent = ecs::Entity();
    nodeComponent->m_previousSibling = ecs::Entity();
    nodeComponent->m_nextSibling = ecs::Entity();
}
