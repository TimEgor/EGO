#include "TestDemo.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Graphic/Render/Component/CameraComponent.h"
#include "EgoEngine/Graphic/Render/Component/MeshRenderComponent.h"
#include "EgoEngine/Platform/Platform.h"
#include "EgoEngine/Resources/Resource/ResourceController.h"
#include "EgoFramework/Framework.h"
#include "EgoFramework/Project.h"
#include "EgoFramework/ProjectReader.h"

namespace
{
    constexpr float MeshRotationSpeed = 1.0f;
    constexpr float FullRotation = 6.28318530718f;

    bool ReadAssetDirectoriesFromActiveProject(ego::framework::Project::DirectoryCollection& _directories)
    {
        const ego::framework::FrameworkPointer framework = ego::framework::FrameworkCore::GetInstance().getFramework();
        EGO_CHECK_RETURN_FALSE(framework);

        const ego::framework::ProjectPointer project = framework->getProject();
        EGO_CHECK_RETURN_FALSE(project);

        _directories = project->getAssetDirectories();
        return !_directories.empty();
    }

    bool ReadAssetDirectories(
        const ego::FileName& _projectPath,
        ego::framework::Project::DirectoryCollection& _directories
    )
    {
        if (ReadAssetDirectoriesFromActiveProject(_directories))
        {
            return true;
        }

        ego::framework::Project project;
        EGO_CHECK_RETURN_FALSE(ego::framework::ProjectReader::ReadFromFile(_projectPath, project));

        _directories = project.getAssetDirectories();
        return !_directories.empty();
    }
}

ego::demo::TestDemo::TestDemo(const FileName& _projectPath)
    : m_projectPath(_projectPath)
{}

bool ego::demo::TestDemo::init()
{
    framework::Project::DirectoryCollection assetDirectories;
    EGO_CHECK_RETURN_FALSE(ReadAssetDirectories(m_projectPath, assetDirectories));

    engine::Engine& engine = engine::GetEngine();
    ResourceController& resourceController = engine.getResourceController();

    for (const FileName& assetDirectory : assetDirectories)
    {
        RootedFileSystemPointer assetFileSystem =
            new RootedFileSystem(engine.getPlatform().getFileSystem(), assetDirectory);
        EGO_CHECK_INITIALIZATION(assetFileSystem && assetFileSystem->init());

        resourceController.addFileSystem(assetFileSystem);
        m_assetFileSystems.push_back(assetFileSystem);
    }

    m_triangleMesh = resourceController.load<MeshResource>("TestTriangle.mesh.xml");
    EGO_CHECK_INITIALIZATION(m_triangleMesh && m_triangleMesh->isLoaded());

    m_triangleMaterial = resourceController.load<MaterialResource>("TestTriangle.material.xml");
    EGO_CHECK_INITIALIZATION(m_triangleMaterial && m_triangleMaterial->isLoaded());

    m_level = engine.getLevelController().createLevel();
    EGO_CHECK_INITIALIZATION(m_level);
    EGO_CHECK_INITIALIZATION(engine.getLevelController().setActiveLevel(m_level->getID()));

    const ecs::Entity cameraNode = m_level->createNode();
    EGO_CHECK_INITIALIZATION(cameraNode);

    EGO_CHECK_INITIALIZATION(m_level->addOrReplaceComponent<CameraComponent>(cameraNode));
    engine.setRenderCameraEntity(cameraNode);

    m_meshEntity = m_level->createNode();
    EGO_CHECK_INITIALIZATION(m_meshEntity);

    EGO_CHECK_INITIALIZATION(m_level->addOrReplaceComponent<MeshRenderComponent>(
        m_meshEntity,
        CreateMeshHandle(m_triangleMesh),
        CreateMaterialHandle(m_triangleMaterial)
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

    for (const RootedFileSystemPointer& assetFileSystem : m_assetFileSystems)
    {
        if (assetFileSystem)
        {
            if (engine)
            {
                engine->getResourceController().removeFileSystem(assetFileSystem);
            }

            assetFileSystem->release();
        }
    }
    m_assetFileSystems.clear();
}
