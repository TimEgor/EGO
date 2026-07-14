#include "TestDemo.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoMath/ComputeQuaternion.h"

#include "EgoResource/ResourceController.h"

#include "EgoEngine/EngineSession.h"
#include "EgoEngine/Graphic/Render/Component/CameraComponent.h"
#include "EgoEngine/Graphic/Render/Component/MeshRenderComponent.h"

namespace
{
    constexpr float MeshRotationSpeed = 1.0f;
    constexpr float FullRotation = 6.28318530718f;
    constexpr float TriangleScale = 0.65f;

    const ego::ComputeVector3 FirstTrianglePosition(-0.60f, 0.0f, 0.0f);
    const ego::ComputeVector3 SecondTrianglePosition(0.60f, 0.0f, 0.0f);
} // namespace

bool ego::demo::TestDemo::init(const InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(!_initData.m_engineSession.isExpired());
    EGO_CHECK_INITIALIZATION(_initData.m_resourceController);
    EGO_CHECK_INITIALIZATION(m_engineSession.isExpired());
    EGO_CHECK_INITIALIZATION(!m_resourceController);

    m_engineSession = _initData.m_engineSession;
    m_resourceController = _initData.m_resourceController;

    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    EGO_CHECK_INITIALIZATION(engineSession);

    m_triangleMesh = m_resourceController->load<render::MeshResource>("TestTriangle.mesh.xml");
    EGO_CHECK_INITIALIZATION(m_triangleMesh && m_triangleMesh->isLoaded());

    m_firstTriangleMaterial = m_resourceController->load<render::MaterialResource>("TestTriangle.material.xml");
    EGO_CHECK_INITIALIZATION(m_firstTriangleMaterial && m_firstTriangleMaterial->isLoaded());

    m_secondTriangleMaterial = m_resourceController->load<render::MaterialResource>("TestTriangleSecond.material.xml");
    EGO_CHECK_INITIALIZATION(m_secondTriangleMaterial && m_secondTriangleMaterial->isLoaded());

    m_level = engineSession->getLevelController().createLevel();
    EGO_CHECK_INITIALIZATION(m_level);
    EGO_CHECK_INITIALIZATION(engineSession->getLevelController().setActiveLevel(m_level->getID()));

    const ecs::Entity cameraNode = m_level->createNode();
    EGO_CHECK_INITIALIZATION(cameraNode);

    EGO_CHECK_INITIALIZATION(m_level->addOrReplaceComponent<render::CameraComponent>(cameraNode));
    engineSession->setRenderCameraEntity(cameraNode);

    EGO_CHECK_INITIALIZATION(createTriangleEntity(m_firstTriangleEntity, m_firstTriangleMaterial, FirstTrianglePosition));
    EGO_CHECK_INITIALIZATION(createTriangleEntity(m_secondTriangleEntity, m_secondTriangleMaterial, SecondTrianglePosition));

    return true;
}

void ego::demo::TestDemo::update(float _deltaTime)
{
    if (!m_level || !m_firstTriangleEntity || !m_secondTriangleEntity)
    {
        return;
    }

    m_triangleRotationAngle += MeshRotationSpeed * _deltaTime;
    while (m_triangleRotationAngle >= FullRotation)
    {
        m_triangleRotationAngle -= FullRotation;
    }

    if (!setTriangleTransform(m_firstTriangleEntity, FirstTrianglePosition, m_triangleRotationAngle))
    {
        return;
    }

    setTriangleTransform(m_secondTriangleEntity, SecondTrianglePosition, -m_triangleRotationAngle);
}

void ego::demo::TestDemo::release()
{
    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    if (engineSession && m_level)
    {
        const LevelPointer activeLevel = engineSession->getLevelController().getActiveLevel();
        if (activeLevel && activeLevel->getID() == m_level->getID())
        {
            engineSession->getLevelController().clearActiveLevel();
        }

        engineSession->clearRenderCameraEntity();
    }

    m_level = nullptr;
    m_firstTriangleEntity = ecs::Entity();
    m_secondTriangleEntity = ecs::Entity();
    m_triangleRotationAngle = 0.0f;
    m_secondTriangleMaterial = nullptr;
    m_firstTriangleMaterial = nullptr;
    m_triangleMesh = nullptr;
    m_resourceController = nullptr;
    m_engineSession.reset();
}

bool ego::demo::TestDemo::createTriangleEntity(ecs::Entity& _entity, const render::MaterialResourcePointer& _materialResource, const ComputeVector3& _position)
{
    _entity = m_level->createNode();
    EGO_CHECK_RETURN_FALSE(_entity);

    const render::RenderMesh triangleMesh = render::CreateMeshHandler(m_triangleMesh);
    const render::RenderMaterial triangleMaterial = render::CreateMaterialHandler(_materialResource);
    EGO_CHECK_RETURN_FALSE(triangleMesh && triangleMaterial);

    EGO_CHECK_RETURN_FALSE(m_level->addOrReplaceComponent<render::MeshRenderComponent>(_entity, triangleMesh, triangleMaterial));
    EGO_CHECK_RETURN_FALSE(setTriangleTransform(_entity, _position, 0.0f));

    return true;
}

bool ego::demo::TestDemo::setTriangleTransform(ecs::Entity _entity, const ComputeVector3& _position, float _rotationAngle)
{
    TransformComponent* transformComponent = m_level->tryGetComponent<TransformComponent>(_entity);
    EGO_CHECK_RETURN_FALSE(transformComponent);

    Transform transform;
    transform.setRotationQuaternion(ComputeQuaternion(ComputeVector3UnitZ, _rotationAngle));
    const ComputeVector3 axisX = transform.getAxisX() * TriangleScale;
    const ComputeVector3 axisY = transform.getAxisY() * TriangleScale;
    const ComputeVector3 axisZ = transform.getAxisZ() * TriangleScale;
    transform.setAxisX(axisX);
    transform.setAxisY(axisY);
    transform.setAxisZ(axisZ);
    transform.setOrigin(_position);
    transformComponent->m_globalTransform = transform;

    return true;
}
