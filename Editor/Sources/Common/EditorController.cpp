#include "EditorController.h"

#include <string>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/Platform/FileSystem/RootedFileSystem.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoResource/ResourceController.h"
#include "EgoResource/ResourceSubsystem.h"

#include "EgoApplication/ApplicationSubsystem.h"
#include "EgoApplication/Presentation/PresenterProvider.h"

#include "EgoEngine/EngineSubsystem.h"

ego::editor::EditorController::~EditorController()
{
    release();
}

bool ego::editor::EditorController::init(const XmlDocument& _config)
{
    release();

    const application::ApplicationSubsystemPointer applicationSubsystem = application::GetApplicationSubsystemPointer();
    EGO_CHECK_INITIALIZATION(applicationSubsystem && applicationSubsystem->getApplicationPointer());

    const engine::EngineSubsystemPointer engineSubsystem = engine::GetEngineSubsystemPointer();
    EGO_CHECK_INITIALIZATION(engineSubsystem && engineSubsystem->getEnginePointer());

    EGO_CHECK_INITIALIZATION(initEditorAssets(_config));
    EGO_CHECK_INITIALIZATION(initContext(_config));
    EGO_CHECK_INITIALIZATION(m_projectController.init(_config));

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

    m_editorAssetsFileSystem->release();
    m_editorAssetsFileSystem = nullptr;
}

bool ego::editor::EditorController::initContext(const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(!m_editorContext.m_engineSession && !m_editorContext.m_mainSurface);

    const application::ApplicationSubsystemPointer applicationSubsystem = application::GetApplicationSubsystemPointer();
    const application::ApplicationPointer application = applicationSubsystem ? applicationSubsystem->getApplicationPointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(application);

    const engine::EngineSubsystemPointer engineSubsystem = engine::GetEngineSubsystemPointer();
    const engine::EnginePointer engine = engineSubsystem ? engineSubsystem->getEnginePointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(engine);

    const XmlNode rootNode = _config.getRootNode();
    EGO_CHECK_RETURN_FALSE(rootNode && rootNode.getNameView() == "Editor");

    const XmlNode engineNode = rootNode.getChild("Engine");
    EGO_CHECK_RETURN_FALSE(engineNode);

    const application::PresenterProviderPointer presenterProvider = application->getPresenterProviderPointer();
    EGO_CHECK_RETURN_FALSE(presenterProvider);

    application::PresentationDesc presentationDesc;
    presentationDesc.m_name = "EgoEditor";
    presentationDesc.m_size = SurfaceSize(1100, 700);
    presentationDesc.m_hasFrame = false;
    const application::Presentation presentation = presenterProvider->createPresentation(presentationDesc);
    if (!presentation.m_surface || !presentation.m_graphicPresenter || !presentation.m_surface->isValid())
    {
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
        presenterProvider->destroyPresentation(presentation.m_surface);

        return false;
    }

    m_editorContext.m_engineSession = engineSession;
    m_editorContext.m_mainSurface = presentation.m_surface;

    return m_guiController.init(_config);
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

    const application::ApplicationSubsystemPointer applicationSubsystem = application::GetApplicationSubsystemPointer();
    const application::ApplicationPointer application = applicationSubsystem ? applicationSubsystem->getApplicationPointer() : nullptr;
    const application::PresenterProviderPointer presenterProvider = application ? application->getPresenterProviderPointer() : nullptr;
    if (presenterProvider && m_editorContext.m_mainSurface)
    {
        presenterProvider->destroyPresentation(m_editorContext.m_mainSurface);
    }
    m_editorContext.m_mainSurface = nullptr;
}
