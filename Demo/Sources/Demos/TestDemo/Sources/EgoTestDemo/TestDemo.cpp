#include "TestDemo.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Platform/Platform.h"
#include "EgoEngine/Resources/Resource/ResourceController.h"
#include "EgoFramework/Framework.h"
#include "EgoFramework/Project.h"
#include "EgoFramework/ProjectReader.h"

namespace
{
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

    m_textResource = resourceController.load<TextResource>("TestDemoText.txt");
    EGO_CHECK_INITIALIZATION(m_textResource && m_textResource->isLoaded());

    m_loadedText = m_textResource->getText();

    m_triangleMesh = resourceController.load<MeshResource>("TestTriangle.mesh.xml");
    EGO_CHECK_INITIALIZATION(m_triangleMesh && m_triangleMesh->isLoaded());

    m_triangleMaterial = resourceController.load<MaterialResource>("TestTriangle.material.xml");
    EGO_CHECK_INITIALIZATION(m_triangleMaterial && m_triangleMaterial->isLoaded());

    engine.getRender().addRenderItem(m_triangleMesh->getMesh(), m_triangleMaterial->getMaterial());
    return true;
}

void ego::demo::TestDemo::release()
{
    const engine::EnginePointer engine = engine::EngineCore::GetInstance().getEngine();
    if (engine)
    {
        engine->getRender().clearRenderItems();
    }

    m_triangleMaterial = nullptr;
    m_triangleMesh = nullptr;

    m_loadedText.clear();
    m_textResource = nullptr;

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

const std::string& ego::demo::TestDemo::getLoadedText() const
{
    return m_loadedText;
}
