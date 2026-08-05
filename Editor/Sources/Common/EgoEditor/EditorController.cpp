#include "EgoEditor/EditorController.h"

#include <string>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/Platform/FileSystem/RootedFileSystem.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoResource/ResourceController.h"
#include "EgoResource/ResourceSubsystem.h"

#include "EgoRuntime/RuntimeSubsystem.h"
#include "EgoRuntime/Presentation/PresenterProvider.h"

#include "EgoEngine/EngineSubsystem.h"

ego::editor::EditorController::~EditorController()
{
    release();
}

bool ego::editor::EditorController::init(const XmlDocument& _config)
{
    release();

    const runtime::RuntimeSubsystemPointer runtimeSubsystem = runtime::GetRuntimeSubsystemPointer();
    EGO_CHECK_INITIALIZATION_ASSERT(runtimeSubsystem && runtimeSubsystem->getRuntimePointer());

    const engine::EngineSubsystemPointer engineSubsystem = engine::GetEngineSubsystemPointer();
    EGO_CHECK_INITIALIZATION_ASSERT(engineSubsystem && engineSubsystem->getEnginePointer());

    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initEditorAssets(_config), "Failed to initialize editor assets.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initContext(_config), "Failed to initialize the editor context.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(m_projectController.init(_config), "Failed to initialize the editor project controller.");

    m_editorContext.m_mainSurface->show();

    return true;
}

void ego::editor::EditorController::release()
{
    m_projectController.release();
    releaseContext();
    releaseEditorAssets();
}

bool ego::editor::EditorController::isMainSurfaceValid() const
{
    return m_editorContext.m_mainSurface && m_editorContext.m_mainSurface->isValid();
}

ego::engine::EngineSessionPointer ego::editor::EditorController::getEditorEngineSessionPointer() const
{
    return m_editorContext.m_engineSession;
}

ego::editor::GuiController& ego::editor::EditorController::getGuiController()
{
    return m_guiController;
}

const ego::editor::GuiController& ego::editor::EditorController::getGuiController() const
{
    return m_guiController;
}

ego::editor::ProjectController& ego::editor::EditorController::getProjectController()
{
    return m_projectController;
}

const ego::editor::ProjectController& ego::editor::EditorController::getProjectController() const
{
    return m_projectController;
}

ego::PlatformSurfacePointer ego::editor::EditorController::getMainSurfacePointer() const
{
    return m_editorContext.m_mainSurface;
}

bool ego::editor::EditorController::initEditorAssets(const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(!m_editorAssetsFileSystem);

    const XmlNode rootNode = _config.getRootNode();
    EGO_CHECK_RETURN_FALSE(rootNode && rootNode.getNameView() == "Editor");

    const XmlNode engineNode = rootNode.getChild("Engine");
    EGO_CHECK_RETURN_FALSE(engineNode);

    const std::string assetsDirectory = engineNode.getChildValueOr<std::string>("AssetsDir", std::string());
    EGO_CHECK_RETURN_FALSE(!assetsDirectory.empty());

    const PlatformPointer platform = GetPlatformPointer();
    const FileSystemPointer sourceFileSystem = platform ? platform->getFileSystem() : nullptr;
    EGO_CHECK_RETURN_FALSE(sourceFileSystem);

    RootedFileSystemPointer assetsFileSystem = MakePointer<RootedFileSystem>(sourceFileSystem, FileName(assetsDirectory));
    EGO_CHECK_RETURN_FALSE(assetsFileSystem && assetsFileSystem->init());

    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    const ResourceControllerPointer resourceController = resourceSubsystem ? resourceSubsystem->getResourceControllerPointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(resourceController);

    resourceController->addFileSystem(assetsFileSystem);
    m_editorAssetsFileSystem = assetsFileSystem;

    return true;
}

void ego::editor::EditorController::releaseEditorAssets()
{
    EGO_CHECK_RETURN(m_editorAssetsFileSystem);

    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    const ResourceControllerPointer resourceController = resourceSubsystem ? resourceSubsystem->getResourceControllerPointer() : nullptr;
    if (resourceController)
    {
        resourceController->removeFileSystem(m_editorAssetsFileSystem);
    }

    m_editorAssetsFileSystem = nullptr;
}

bool ego::editor::EditorController::initContext(const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(!m_editorContext.m_engineSession && !m_editorContext.m_mainSurface);

    const runtime::RuntimeSubsystemPointer runtimeSubsystem = runtime::GetRuntimeSubsystemPointer();
    const runtime::RuntimePointer runtime = runtimeSubsystem ? runtimeSubsystem->getRuntimePointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(runtime);

    const engine::EngineSubsystemPointer engineSubsystem = engine::GetEngineSubsystemPointer();
    const engine::EnginePointer engine = engineSubsystem ? engineSubsystem->getEnginePointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(engine);

    const XmlNode rootNode = _config.getRootNode();
    EGO_CHECK_RETURN_FALSE(rootNode && rootNode.getNameView() == "Editor");

    const XmlNode engineNode = rootNode.getChild("Engine");
    EGO_CHECK_RETURN_FALSE(engineNode);

    const runtime::PresenterProviderPointer presenterProvider = runtime->getPresenterProviderPointer();
    EGO_CHECK_RETURN_FALSE(presenterProvider);

    runtime::PresentationDesc presentationDesc;
    presentationDesc.m_name = "EgoEditor";
    presentationDesc.m_size = SurfaceSize(1100, 700);
    presentationDesc.m_hasFrame = false;
    const runtime::Presentation presentation = presenterProvider->createPresentation(presentationDesc);
    if (!presentation.m_surface || !presentation.m_graphicPresenter || !presentation.m_surface->isValid())
    {
        EGO_ASSERT_FAIL_MESSAGE("Failed to create the editor presentation.");
        if (presentation.m_surface)
        {
            presenterProvider->destroyPresentation(presentation.m_surface);
        }

        return false;
    }

    engine::EngineSession::InitData sessionInitData;
    sessionInitData.m_mainPresentation = presentation;
    sessionInitData.m_sceneRender.m_isEnabled = false;
    sessionInitData.m_gui.m_isEnabled = true;
    sessionInitData.m_gui.m_pluginModuleName = FileName(engineNode.getChildValueOr<std::string>("EditorRenderPlugin", std::string()));

    const engine::EngineSessionPointer engineSession = engine->createSession(sessionInitData);
    if (!engineSession)
    {
        EGO_ASSERT_FAIL_MESSAGE("Failed to create the editor engine session.");
        presenterProvider->destroyPresentation(presentation.m_surface);

        return false;
    }

    m_editorContext.m_engineSession = engineSession;
    m_editorContext.m_mainSurface = presentation.m_surface;

    if (!m_guiController.init(_config))
    {
        EGO_ASSERT_FAIL_MESSAGE("Failed to initialize the editor GUI controller.");

        return false;
    }

    return true;
}

void ego::editor::EditorController::releaseContext()
{
    m_guiController.release();

    const engine::EngineSubsystemPointer engineSubsystem = engine::GetEngineSubsystemPointer();
    const engine::EnginePointer engine = engineSubsystem ? engineSubsystem->getEnginePointer() : nullptr;
    if (engine && m_editorContext.m_engineSession)
    {
        engine->destroySession(m_editorContext.m_engineSession->getID());
    }
    m_editorContext.m_engineSession = nullptr;

    const runtime::RuntimeSubsystemPointer runtimeSubsystem = runtime::GetRuntimeSubsystemPointer();
    const runtime::RuntimePointer runtime = runtimeSubsystem ? runtimeSubsystem->getRuntimePointer() : nullptr;
    const runtime::PresenterProviderPointer presenterProvider = runtime ? runtime->getPresenterProviderPointer() : nullptr;
    if (presenterProvider && m_editorContext.m_mainSurface)
    {
        presenterProvider->destroyPresentation(m_editorContext.m_mainSurface);
    }
    m_editorContext.m_mainSurface = nullptr;
}
