#include "DefaultRenderScene.h"

#include "EgoEngine/Graphic/Render/Component/MeshRenderComponent.h"
#include "EgoEngine/Level/Level.h"

void ego::render::DefaultRenderScene::collect(Level& _level)
{
    m_items.clear();

    _level.forEachComponent<MeshRenderComponent>(
        [this, &_level](ecs::Entity _entity, const MeshRenderComponent& _meshRenderComponent)
        {
            if (!_meshRenderComponent.m_mesh || !_meshRenderComponent.m_material)
            {
                return;
            }

            DefaultRenderItem item;
            item.m_mesh = _meshRenderComponent.m_mesh;
            item.m_material = _meshRenderComponent.m_material;
            item.m_objectIndex = static_cast<uint32_t>(m_items.size());

            const TransformComponent* transformComponent = _level.tryGetComponent<TransformComponent>(_entity);
            if (transformComponent)
            {
                item.m_globalTransform = transformComponent->m_globalTransform;
            }

            m_items.push_back(item);
        });
}

void ego::render::DefaultRenderScene::clear()
{
    m_items.clear();
}

ego::render::DefaultRenderScene::ItemCollection& ego::render::DefaultRenderScene::getItems()
{
    return m_items;
}

const ego::render::DefaultRenderScene::ItemCollection& ego::render::DefaultRenderScene::getItems() const
{
    return m_items;
}

bool ego::render::DefaultRenderScene::isEmpty() const
{
    return m_items.empty();
}
