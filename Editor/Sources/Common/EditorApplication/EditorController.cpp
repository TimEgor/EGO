#include "EditorController.h"

#include <string>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Math/Color.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/Platform/FileSystem/RootedFileSystem.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoECS/Entity.h"

#include "EgoResource/ResourceController.h"
#include "EgoResource/ResourceSubsystem.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoEngine/Graphic/SceneRender/Component/CameraComponent.h"
#include "EgoEngine/Graphic/SceneRender/Render.h"

#include "EgoApplication/Presentation/PresenterProvider.h"

namespace
{
    constexpr ego::gpu::Texture2DSize SimulationTextureSize(900, 600);
} // namespace

ego::editor::EditorController::~EditorController()
{
    release();
}

bool ego::editor::EditorController::init(const application::ApplicationPointer& _application, const engine::EnginePointer& _engine, const XmlDocument& _config)
{
    release();

    EGO_CHECK_RETURN_FALSE(_application && _engine);

    m_application = _application;
    m_engine = _engine;

    if (!initEditorAssets(_config) || !initProjectContext(_config) || !initEditorContext(_config))
    {
        release();
        return false;
    }

    m_editorContext.m_surface->show();
    return true;
}

void ego::editor::EditorController::release()
{
    releaseEditorContext();
    releaseProjectContext();
    releaseEditorAssets();

    m_engine = nullptr;
    m_application = nullptr;
}

void ego::editor::EditorController::update()
{
    drawProjectContext();
}

bool ego::editor::EditorController::isInitialized() const
{
    return m_application && m_engine && m_editorContext.m_engineSession && m_editorContext.m_surface && m_projectContext.m_simulationSession &&
           m_projectContext.m_simulationGraphicPresenter && m_projectContext.m_simulationLevel && m_editorGuiController.isInitialized();
}

bool ego::editor::EditorController::isSurfaceValid() const
{
    return m_editorContext.m_surface && m_editorContext.m_surface->isValid();
}

ego::gui::GuiControllerPointer ego::editor::EditorController::getGuiControllerPointer() const
{
    return m_editorContext.m_engineSession ? m_editorContext.m_engineSession->getGuiControllerPointer() : nullptr;
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
    if (!m_editorAssetsFileSystem)
    {
        return;
    }

    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    const ResourceControllerPointer resourceController = resourceSubsystem ? resourceSubsystem->getResourceControllerPointer() : nullptr;
    if (resourceController)
    {
        resourceController->removeFileSystem(m_editorAssetsFileSystem);
    }

    m_editorAssetsFileSystem->release();
    m_editorAssetsFileSystem = nullptr;
}

bool ego::editor::EditorController::initEditorContext(const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(m_application && m_engine);
    EGO_CHECK_RETURN_FALSE(!m_editorContext.m_engineSession && !m_editorContext.m_surface);
    EGO_CHECK_RETURN_FALSE(m_projectContext.m_simulationGraphicPresenter);

    const XmlNode rootNode = _config.getRootNode();
    EGO_CHECK_RETURN_FALSE(rootNode && rootNode.getNameView() == "Editor");

    const XmlNode engineNode = rootNode.getChild("Engine");
    EGO_CHECK_RETURN_FALSE(engineNode);

    const application::PresenterProviderPointer presenterProvider = m_application->getPresenterProviderPointer();
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
    sessionInitData.m_gui.m_pluginModuleName = FileName(engineNode.getChildValueOr<std::string>("GuiRenderPlugin", std::string()));

    const engine::EngineSessionPointer engineSession = m_engine->createSession(sessionInitData);
    if (!engineSession)
    {
        presenterProvider->destroyPresentation(presentation.m_surface);
        return false;
    }

    m_editorContext.m_engineSession = engineSession;
    m_editorContext.m_surface = presentation.m_surface;

    return m_editorGuiController.init(_config, m_editorContext.m_surface, m_projectContext.m_simulationGraphicPresenter);
}

void ego::editor::EditorController::releaseEditorContext()
{
    m_editorGuiController.release();

    if (m_engine && m_editorContext.m_engineSession)
    {
        m_engine->destroySession(m_editorContext.m_engineSession->getID());
    }
    m_editorContext.m_engineSession = nullptr;

    const application::PresenterProviderPointer presenterProvider = m_application ? m_application->getPresenterProviderPointer() : nullptr;
    if (presenterProvider && m_editorContext.m_surface)
    {
        presenterProvider->destroyPresentation(m_editorContext.m_surface);
    }
    m_editorContext.m_surface = nullptr;
}

bool ego::editor::EditorController::initProjectContext(const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(m_engine);
    EGO_CHECK_RETURN_FALSE(!m_projectContext.m_simulationSession && !m_projectContext.m_simulationGraphicPresenter && !m_projectContext.m_simulationLevel);

    const XmlNode rootNode = _config.getRootNode();
    EGO_CHECK_RETURN_FALSE(rootNode && rootNode.getNameView() == "Editor");

    const XmlNode engineNode = rootNode.getChild("Engine");
    EGO_CHECK_RETURN_FALSE(engineNode);

    TextureGraphicPresenterPointer graphicPresenter = new TextureGraphicPresenter();
    if (!graphicPresenter || !graphicPresenter->init(gpu::GetGraphicDevice(), SimulationTextureSize, gpu::GraphicResourceFormat::R8G8B8A8UNorm))
    {
        EGO_SAFE_RESET_POINTER_WITH_RELEASING(graphicPresenter);
        return false;
    }

    engine::EngineSession::InitData sessionInitData;
    sessionInitData.m_mainPresentation.m_graphicPresenter = graphicPresenter;
    sessionInitData.m_sceneRender.m_pluginModuleName = FileName(engineNode.getChildValueOr<std::string>("RenderPlugin", std::string()));

    const engine::EngineSessionPointer engineSession = m_engine->createSession(sessionInitData);
    if (!engineSession)
    {
        graphicPresenter->release();
        graphicPresenter = nullptr;
        return false;
    }

    m_projectContext.m_simulationSession = engineSession;
    m_projectContext.m_simulationGraphicPresenter = graphicPresenter;

    LevelController& levelController = engineSession->getLevelController();
    m_projectContext.m_simulationLevel = levelController.createLevel();
    EGO_CHECK_RETURN_FALSE(m_projectContext.m_simulationLevel);
    EGO_CHECK_RETURN_FALSE(levelController.setActiveLevel(m_projectContext.m_simulationLevel->getID()));

    const ecs::Entity cameraEntity = m_projectContext.m_simulationLevel->createNode();
    EGO_CHECK_RETURN_FALSE(cameraEntity);
    EGO_CHECK_RETURN_FALSE(m_projectContext.m_simulationLevel->addOrReplaceComponent<render::CameraComponent>(cameraEntity));

    engineSession->setRenderCameraEntity(cameraEntity);
    return true;
}

void ego::editor::EditorController::releaseProjectContext()
{
    if (m_projectContext.m_simulationSession && m_projectContext.m_simulationLevel)
    {
        LevelController& levelController = m_projectContext.m_simulationSession->getLevelController();
        const LevelPointer activeLevel = levelController.getActiveLevel();
        if (activeLevel && activeLevel->getID() == m_projectContext.m_simulationLevel->getID())
        {
            levelController.clearActiveLevel();
        }

        m_projectContext.m_simulationSession->clearRenderCameraEntity();
    }
    m_projectContext.m_simulationLevel = nullptr;

    if (m_engine && m_projectContext.m_simulationSession)
    {
        m_engine->destroySession(m_projectContext.m_simulationSession->getID());
    }
    m_projectContext.m_simulationSession = nullptr;

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_projectContext.m_simulationGraphicPresenter);
}

void ego::editor::EditorController::drawProjectContext()
{
    if (!m_projectContext.m_simulationSession || !m_projectContext.m_simulationLevel)
    {
        return;
    }

    render::Render& render = m_projectContext.m_simulationSession->getRender();

    const FloatVector3 top(0.0f, 0.6f, 0.0f);
    const FloatVector3 bottomLeft(-0.6f, -0.5f, 0.0f);
    const FloatVector3 bottomRight(0.6f, -0.5f, 0.0f);

    render.drawLine(top, bottomLeft, NormalizedColorRed);
    render.drawLine(bottomLeft, bottomRight, NormalizedColorGreen);
    render.drawLine(bottomRight, top, NormalizedColorBlue);

    render.drawPoint(top, NormalizedColorWhite);
    render.drawPoint(bottomLeft, NormalizedColorWhite);
    render.drawPoint(bottomRight, NormalizedColorWhite);
}
