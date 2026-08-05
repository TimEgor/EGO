#include "TestDemo.h"

#include "EgoCore/Math/ComputeQuaternion.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoResource/ResourceController.h"
#include "EgoResource/ResourceSubsystem.h"

#include "EgoEngine/EngineSession.h"
#include "EgoEngine/Graphic/SceneRender/Component/CameraComponent.h"
#include "EgoEngine/Graphic/SceneRender/Component/MeshRenderComponent.h"
#include "EgoEngine/Level/Components/TransformComponent.h"

namespace
{
    constexpr float MeshRotationSpeed = 1.0f;
    constexpr float FullRotation = 6.28318530718f;
    constexpr float TriangleScale = 0.65f;

    constexpr ego::TransformVector CameraPosition(ego::TransformValue(0.0), ego::TransformValue(0.0), ego::TransformValue(-2.0));
    constexpr ego::TransformVector FirstTrianglePosition(ego::TransformValue(-0.60), ego::TransformValue(0.0), ego::TransformValue(0.0));
    constexpr ego::TransformVector SecondTrianglePosition(ego::TransformValue(0.60), ego::TransformValue(0.0), ego::TransformValue(0.0));
} // namespace

ego::demo::TestDemo::~TestDemo()
{
    release();
}

bool ego::demo::TestDemo::init(const engine::EngineSessionWeakPointer& _engineSession)
{
    EGO_CHECK_INITIALIZATION(!_engineSession.isExpired());
    EGO_CHECK_INITIALIZATION(m_engineSession.isExpired());
    EGO_CHECK_INITIALIZATION(!m_resourceController);

    m_engineSession = _engineSession;

    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    EGO_CHECK_INITIALIZATION(engineSession);

    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    EGO_CHECK_INITIALIZATION(resourceSubsystem);

    m_resourceController = resourceSubsystem->getResourceControllerPointer();
    EGO_CHECK_INITIALIZATION(m_resourceController);

    m_triangleMesh = m_resourceController->load<render::MeshResource>("TestTriangle.mesh.xml");
    EGO_CHECK_INITIALIZATION(m_triangleMesh && m_triangleMesh->isLoaded());

    m_firstTriangleMaterial = m_resourceController->load<render::MaterialResource>("TestTriangle.material.xml");
    EGO_CHECK_INITIALIZATION(m_firstTriangleMaterial && m_firstTriangleMaterial->isLoaded());

    m_secondTriangleMaterial = m_resourceController->load<render::MaterialResource>("TestTriangleSecond.material.xml");
    EGO_CHECK_INITIALIZATION(m_secondTriangleMaterial && m_secondTriangleMaterial->isLoaded());

    m_level = MakePointer<Level>();
    EGO_CHECK_INITIALIZATION(m_level);
    EGO_CHECK_INITIALIZATION(engineSession->setActiveLevel(m_level));

    const ecs::Entity cameraNode = m_level->createNode();
    EGO_CHECK_INITIALIZATION(cameraNode);

    EGO_CHECK_INITIALIZATION(m_level->addOrReplaceComponent<render::CameraComponent>(cameraNode));
    TransformComponent* cameraTransformComponent = m_level->tryGetComponent<TransformComponent>(cameraNode);
    EGO_CHECK_INITIALIZATION(cameraTransformComponent);
    cameraTransformComponent->m_globalTransform.setOrigin(CameraPosition);
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

    setTriangleTransform(m_firstTriangleEntity, FirstTrianglePosition, m_triangleRotationAngle);
    setTriangleTransform(m_secondTriangleEntity, SecondTrianglePosition, -m_triangleRotationAngle);
}

void ego::demo::TestDemo::release()
{
    const engine::EngineSessionPointer engineSession = m_engineSession.lock();
    if (engineSession && m_level)
    {
        const LevelPointer activeLevel = engineSession->getActiveLevel();
        if (activeLevel == m_level)
        {
            engineSession->clearActiveLevel();
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

bool ego::demo::TestDemo::createTriangleEntity(ecs::Entity& _entity, const render::MaterialResourcePointer& _materialResource, const TransformVector& _position)
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

bool ego::demo::TestDemo::setTriangleTransform(ecs::Entity _entity, const TransformVector& _position, float _rotationAngle)
{
    TransformComponent* transformComponent = m_level->tryGetComponent<TransformComponent>(_entity);
    EGO_CHECK_RETURN_FALSE(transformComponent);

    Transform transform;
    const ComputeQuaternion rotation(ComputeVector3UnitZBase<ComputeValue>(), static_cast<ComputeValue>(_rotationAngle));
    transform.setRotationQuaternion(TransformQuaternion(rotation.getX(), rotation.getY(), rotation.getZ(), rotation.getW()));
    const ComputeValue triangleScale = static_cast<ComputeValue>(TriangleScale);
    const ComputeVector3 axisX = ComputeVector3(transform.getAxisX()) * triangleScale;
    const ComputeVector3 axisY = ComputeVector3(transform.getAxisY()) * triangleScale;
    const ComputeVector3 axisZ = ComputeVector3(transform.getAxisZ()) * triangleScale;
    transform.setAxisX(axisX.getVector<TransformValue>());
    transform.setAxisY(axisY.getVector<TransformValue>());
    transform.setAxisZ(axisZ.getVector<TransformValue>());
    transform.setOrigin(_position);
    transformComponent->m_globalTransform = transform;

    return true;
}
