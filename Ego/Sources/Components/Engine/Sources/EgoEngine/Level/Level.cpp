#include "Level.h"

#include "EgoCore/Assert/AssertCore.h"

#include "EgoEngine/Engine.h"
#include "LevelController.h"

#include <algorithm>

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

const std::vector<ego::ecs::Entity>* ego::Level::getNodeChildren(ecs::Entity _node) const
{
    if (!m_world || !isNode(_node))
    {
        return nullptr;
    }

    const SceneNodeComponent* nodeComponent = m_world->tryGetComponent<SceneNodeComponent>(_node);
    return nodeComponent ? &nodeComponent->m_children : nullptr;
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

    for (ecs::Entity child : nodeComponent->m_children)
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

    const auto childIt = std::find(
        parentComponent->m_children.begin(),
        parentComponent->m_children.end(),
        _node
    );
    if (childIt == parentComponent->m_children.end())
    {
        parentComponent->m_children.push_back(_node);
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
        parentComponent->m_children.erase(
            std::remove(
                parentComponent->m_children.begin(),
                parentComponent->m_children.end(),
                _node
            ),
            parentComponent->m_children.end()
        );
    }

    nodeComponent->m_parent = ecs::Entity();
}

