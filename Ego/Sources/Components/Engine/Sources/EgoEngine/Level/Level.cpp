#include "Level.h"

#include "EgoCore/Assert/AssertCore.h"

#include "EgoEngine/Engine.h"
#include "LevelController.h"

ego::Level::Level(LevelID _id)
    : m_world(new ecs::World())
    , m_id(_id)
{
    createRootNode();
}

ego::Level::~Level()
{
    release();
}

void ego::LevelDeleter::operator()(Level* _level) const
{
    if (_level)
    {
        LevelController& levelController = engine::GetEngine().getLevelController();
        LevelController::LevelControllerAccessor::RemoveLevel(levelController, _level->getID());
    }

    Level::LevelAccessor::Destroy(_level);
}

void ego::Level::LevelAccessor::Destroy(Level* _level)
{
    if (!_level)
    {
        return;
    }

    delete _level;
}

ego::Level::NodeChildIterator::NodeChildIterator(const Level* _level, ecs::Entity _node)
    : m_level(_level && _node ? _level : nullptr)
    , m_node(_level ? _node : ecs::Entity())
    , m_nextNode(m_level ? m_level->getNextNodeSibling(m_node) : ecs::Entity())
{}

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

ego::Level::NodeChildRange::NodeChildRange(const Level* _level, ecs::Entity _parent)
    : m_level(_level && _parent ? _level : nullptr)
    , m_parent(_level ? _parent : ecs::Entity())
{}

ego::Level::NodeChildIterator ego::Level::NodeChildRange::begin() const
{
    return m_level
        ? NodeChildIterator(m_level, m_level->getFirstNodeChild(m_parent))
        : NodeChildIterator();
}

ego::Level::NodeChildIterator ego::Level::NodeChildRange::end() const
{
    return NodeChildIterator();
}

ego::LevelID ego::Level::getID() const
{
    return m_id;
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
    attachEntity(entity);

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
    if (!m_world || !m_world->isEntityAlive(_entity))
    {
        return false;
    }

    const LevelComponent* levelComponent = m_world->tryGetComponent<LevelComponent>(_entity);
    return levelComponent && levelComponent->m_levelID == m_id;
}

size_t ego::Level::getEntityCount() const
{
    return m_world ? m_world->getEntityCount() : 0;
}

ego::ecs::Entity ego::Level::getRootNode() const
{
    if (!m_world || !isNode(m_rootNode))
    {
        return ecs::Entity();
    }

    const SceneNodeComponent* rootComponent = m_world->tryGetComponent<SceneNodeComponent>(m_rootNode);
    return rootComponent && !rootComponent->m_parent ? m_rootNode : ecs::Entity();
}

bool ego::Level::isNode(ecs::Entity _entity) const
{
    return m_world &&
        ownsEntity(_entity) &&
        m_world->hasComponent<SceneNodeComponent>(_entity) &&
        m_world->hasComponent<TransformComponent>(_entity);
}

ego::ecs::Entity ego::Level::createNode()
{
    return createNode(getRootNode());
}

ego::ecs::Entity ego::Level::createNode(ecs::Entity _parent)
{
    if (!m_world || !isNode(_parent))
    {
        return ecs::Entity();
    }

    const ecs::Entity node = createEntity();
    if (!node)
    {
        return ecs::Entity();
    }

    m_world->addOrReplaceComponent<SceneNodeComponent>(node);
    m_world->addOrReplaceComponent<TransformComponent>(node);

    if (!attachNodeToParent(node, _parent))
    {
        m_world->destroyEntity(node);
        return ecs::Entity();
    }

    return node;
}

bool ego::Level::destroyNode(ecs::Entity _node)
{
    if (!m_world || !isNode(_node) || _node == m_rootNode)
    {
        return false;
    }

    std::vector<ecs::Entity> nodes;
    collectNodeHierarchy(_node, nodes);
    detachNodeFromParent(_node);

    for (auto nodeIt = nodes.rbegin(); nodeIt != nodes.rend(); ++nodeIt)
    {
        if (m_world->isEntityAlive(*nodeIt))
        {
            m_world->destroyEntity(*nodeIt);
        }
    }

    return true;
}

bool ego::Level::setNodeParent(ecs::Entity _node, ecs::Entity _parent)
{
    if (!m_world ||
        !isNode(_node) ||
        !isNode(_parent) ||
        _node == _parent ||
        _node == m_rootNode ||
        isNodeChildOf(_parent, _node))
    {
        return false;
    }

    const ecs::Entity oldParent = getNodeParent(_node);
    if (oldParent == _parent)
    {
        return true;
    }

    detachNodeFromParent(_node);

    if (!attachNodeToParent(_node, _parent))
    {
        attachNodeToParent(_node, oldParent);
        return false;
    }

    return true;
}

ego::ecs::Entity ego::Level::getNodeParent(ecs::Entity _node) const
{
    if (!m_world || !isNode(_node))
    {
        return ecs::Entity();
    }

    const SceneNodeComponent* nodeComponent = m_world->tryGetComponent<SceneNodeComponent>(_node);
    return nodeComponent ? nodeComponent->m_parent : ecs::Entity();
}

ego::Level::NodeChildRange ego::Level::getNodeChildren(ecs::Entity _node) const
{
    if (!m_world || !isNode(_node))
    {
        return NodeChildRange();
    }

    return NodeChildRange(this, _node);
}

ego::ecs::Entity ego::Level::getFirstNodeChild(ecs::Entity _node) const
{
    if (!m_world || !isNode(_node))
    {
        return ecs::Entity();
    }

    const SceneNodeComponent* nodeComponent = m_world->tryGetComponent<SceneNodeComponent>(_node);
    return nodeComponent ? nodeComponent->m_firstChild : ecs::Entity();
}

ego::ecs::Entity ego::Level::getLastNodeChild(ecs::Entity _node) const
{
    if (!m_world || !isNode(_node))
    {
        return ecs::Entity();
    }

    const SceneNodeComponent* nodeComponent = m_world->tryGetComponent<SceneNodeComponent>(_node);
    return nodeComponent ? nodeComponent->m_lastChild : ecs::Entity();
}

ego::ecs::Entity ego::Level::getPreviousNodeSibling(ecs::Entity _node) const
{
    if (!m_world || !isNode(_node))
    {
        return ecs::Entity();
    }

    const SceneNodeComponent* nodeComponent = m_world->tryGetComponent<SceneNodeComponent>(_node);
    return nodeComponent ? nodeComponent->m_previousSibling : ecs::Entity();
}

ego::ecs::Entity ego::Level::getNextNodeSibling(ecs::Entity _node) const
{
    if (!m_world || !isNode(_node))
    {
        return ecs::Entity();
    }

    const SceneNodeComponent* nodeComponent = m_world->tryGetComponent<SceneNodeComponent>(_node);
    return nodeComponent ? nodeComponent->m_nextSibling : ecs::Entity();
}

void ego::Level::clear()
{
    if (!m_world)
    {
        return;
    }

    m_world->clear();
    m_rootNode = ecs::Entity();
    createRootNode();
}

void ego::Level::release()
{
    if (m_world)
    {
        m_world->clear();
    }

    m_rootNode = ecs::Entity();
    m_world = nullptr;
}

void ego::Level::attachEntity(ecs::Entity _entity)
{
    EGO_ASSERT(m_world);
    EGO_ASSERT(m_world->isEntityAlive(_entity));

    m_world->addOrReplaceComponent<LevelComponent>(_entity, LevelComponent{m_id});
}

ego::ecs::Entity ego::Level::createRootNode()
{
    if (!m_world)
    {
        return ecs::Entity();
    }

    m_rootNode = ecs::Entity();

    const ecs::Entity rootNode = createEntity();
    if (!rootNode)
    {
        return ecs::Entity();
    }

    m_world->addOrReplaceComponent<SceneNodeComponent>(rootNode);
    m_world->addOrReplaceComponent<TransformComponent>(rootNode);

    m_rootNode = rootNode;
    return m_rootNode;
}

void ego::Level::collectNodeHierarchy(ecs::Entity _node, std::vector<ecs::Entity>& _nodes) const
{
    if (!m_world || !isNode(_node))
    {
        return;
    }

    _nodes.push_back(_node);

    const SceneNodeComponent* nodeComponent = m_world->tryGetComponent<SceneNodeComponent>(_node);
    if (!nodeComponent)
    {
        return;
    }

    for (ecs::Entity child : getNodeChildren(_node))
    {
        collectNodeHierarchy(child, _nodes);
    }
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

bool ego::Level::attachNodeToParent(ecs::Entity _node, ecs::Entity _parent)
{
    if (!m_world || !isNode(_node) || !isNode(_parent) || _node == _parent || _node == m_rootNode)
    {
        return false;
    }

    SceneNodeComponent* nodeComponent = m_world->tryGetComponent<SceneNodeComponent>(_node);
    SceneNodeComponent* parentComponent = m_world->tryGetComponent<SceneNodeComponent>(_parent);
    if (!nodeComponent || !parentComponent || nodeComponent->m_parent)
    {
        return false;
    }

    if (parentComponent->m_lastChild)
    {
        SceneNodeComponent* lastChildComponent = m_world->tryGetComponent<SceneNodeComponent>(parentComponent->m_lastChild);
        if (!lastChildComponent)
        {
            return false;
        }

        lastChildComponent->m_nextSibling = _node;
        nodeComponent->m_previousSibling = parentComponent->m_lastChild;
        parentComponent->m_lastChild = _node;
    }
    else
    {
        parentComponent->m_firstChild = _node;
        parentComponent->m_lastChild = _node;
    }

    nodeComponent->m_parent = _parent;
    return true;
}

void ego::Level::detachNodeFromParent(ecs::Entity _node)
{
    SceneNodeComponent* nodeComponent = m_world && isNode(_node)
        ? m_world->tryGetComponent<SceneNodeComponent>(_node)
        : nullptr;
    if (!nodeComponent || !nodeComponent->m_parent)
    {
        return;
    }

    SceneNodeComponent* parentComponent = m_world->tryGetComponent<SceneNodeComponent>(nodeComponent->m_parent);
    if (parentComponent)
    {
        if (parentComponent->m_firstChild == _node)
        {
            parentComponent->m_firstChild = nodeComponent->m_nextSibling;
        }

        if (parentComponent->m_lastChild == _node)
        {
            parentComponent->m_lastChild = nodeComponent->m_previousSibling;
        }
    }

    if (nodeComponent->m_previousSibling)
    {
        SceneNodeComponent* previousSiblingComponent = m_world->tryGetComponent<SceneNodeComponent>(nodeComponent->m_previousSibling);
        if (previousSiblingComponent)
        {
            previousSiblingComponent->m_nextSibling = nodeComponent->m_nextSibling;
        }
    }

    if (nodeComponent->m_nextSibling)
    {
        SceneNodeComponent* nextSiblingComponent = m_world->tryGetComponent<SceneNodeComponent>(nodeComponent->m_nextSibling);
        if (nextSiblingComponent)
        {
            nextSiblingComponent->m_previousSibling = nodeComponent->m_previousSibling;
        }
    }

    nodeComponent->m_parent = ecs::Entity();
    nodeComponent->m_previousSibling = ecs::Entity();
    nodeComponent->m_nextSibling = ecs::Entity();
}

