#include "TestDemo.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Graphic/Render/Component/CameraComponent.h"
#include "EgoEngine/Graphic/Render/Component/MeshRenderComponent.h"
#include "EgoEngine/Resources/Resource/ResourceController.h"

namespace
{
    constexpr float MeshRotationSpeed = 1.0f;
    constexpr float FullRotation = 6.28318530718f;
}

bool ego::demo::TestDemo::init()
{
    engine::Engine& engine = engine::GetEngine();
    ResourceController& resourceController = engine.getResourceController();

    m_triangleMesh = resourceController.load<render::MeshResource>("TestTriangle.mesh.xml");
    EGO_CHECK_INITIALIZATION(m_triangleMesh && m_triangleMesh->isLoaded());

    m_triangleMaterial = resourceController.load<render::MaterialResource>("TestTriangle.material.xml");
    EGO_CHECK_INITIALIZATION(m_triangleMaterial && m_triangleMaterial->isLoaded());

    m_level = engine.getLevelController().createLevel();
    EGO_CHECK_INITIALIZATION(m_level);
    EGO_CHECK_INITIALIZATION(engine.getLevelController().setActiveLevel(m_level->getID()));

    const ecs::Entity cameraNode = m_level->createNode();
    EGO_CHECK_INITIALIZATION(cameraNode);

    EGO_CHECK_INITIALIZATION(m_level->addOrReplaceComponent<render::CameraComponent>(cameraNode));
    engine.setRenderCameraEntity(cameraNode);

    m_meshEntity = m_level->createNode();
    EGO_CHECK_INITIALIZATION(m_meshEntity);

    const render::RenderMesh triangleMesh = render::CreateMeshHandler(m_triangleMesh);
    const render::RenderMaterial triangleMaterial = render::CreateMaterialHandler(m_triangleMaterial);
    EGO_CHECK_INITIALIZATION(triangleMesh && triangleMaterial);

    EGO_CHECK_INITIALIZATION(m_level->addOrReplaceComponent<render::MeshRenderComponent>(
        m_meshEntity,
        triangleMesh,
        triangleMaterial
    ));

    return true;
}

void ego::demo::TestDemo::update(float _deltaTime)
{
    if (!m_level || !m_meshEntity)
    {
        return;
    }

    TransformComponent* transformComponent = m_level->tryGetComponent<TransformComponent>(m_meshEntity);
    if (!transformComponent)
    {
        return;
    }

    m_meshRotationAngle += MeshRotationSpeed * _deltaTime;
    while (m_meshRotationAngle >= FullRotation)
    {
        m_meshRotationAngle -= FullRotation;
    }

    Transform transform;
    transform.setRotationQuaternion(ComputeQuaternion(ComputeVector3UnitZ, m_meshRotationAngle));
    transformComponent->m_globalTransform = transform;
}

void ego::demo::TestDemo::release()
{
    const engine::EnginePointer engine = engine::EngineCore::GetInstance().getEngine();
    if (engine && m_level)
    {
        const LevelPointer activeLevel = engine->getLevelController().getActiveLevel();
        if (activeLevel && activeLevel->getID() == m_level->getID())
        {
            engine->getLevelController().clearActiveLevel();
        }

        engine->clearRenderCameraEntity();
    }

    m_level = nullptr;
    m_meshEntity = ecs::Entity();
    m_meshRotationAngle = 0.0f;
    m_triangleMaterial = nullptr;
    m_triangleMesh = nullptr;
}
