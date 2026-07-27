#include "EditorApplication.h"

#include "EgoCore/Math/Color.h"
#include "EgoCore/Parsers/ArgParser/Parser.h"
#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoEngine/Graphic/SceneRender/Component/CameraComponent.h"
#include "EgoEngine/Graphic/SceneRender/Render.h"

#include "EgoApplication/Presentation/PresenterProvider.h"

namespace
{
    constexpr const char* DefaultGuiFontPath = "C:/Windows/Fonts/segoeui.ttf";
    constexpr ego::gpu::Texture2DSize SceneTextureSize(900, 600);
} // namespace

ego::editor::EditorApplication::~EditorApplication()
{
    release();
}

bool ego::editor::EditorApplication::init(void* _nativeInstanceHandle, int _argCount, char** _argValues)
{
    release();

    CommandLineOptions options;
    ParseCommandLine(_argCount, _argValues, options);

    if (!initApplication(_nativeInstanceHandle, options))
    {
        release();
        return false;
    }

    if (!initEngine(options))
    {
        release();
        return false;
    }

    return true;
}

void ego::editor::EditorApplication::release()
{
    releaseEngine();
    releaseApplication();

    m_renderPluginModuleName.clear();
    m_guiRenderPluginModuleName.clear();
}

int ego::editor::EditorApplication::run()
{
    if (!m_application || !m_engine || !m_editorSession.m_engineSession || !m_sceneSession.m_engineSession)
    {
        return InitializationFailedExitCode;
    }

    const int exitCode = runMainLoop() ? SuccessExitCode : RuntimeFailedExitCode;
    release();

    return exitCode;
}

bool ego::editor::EditorApplication::initApplication(void* _nativeInstanceHandle, const CommandLineOptions& _options)
{
    EGO_CHECK_RETURN_FALSE(!m_application);

    m_application = new application::Application();
    EGO_CHECK_RETURN_FALSE(m_application);

    application::Application::InitData initData;
    initData.m_nativeInstanceHandle = _nativeInstanceHandle;
    initData.m_pluginDirectory = FileName(_options.m_pluginDirectoryPath);
    initData.m_profilerPluginModuleName = FileName(_options.m_profilerPluginName);
    initData.m_graphicHardwarePluginModuleName = FileName(_options.m_graphicHardwarePluginModuleName);
    initData.m_enableGraphicHardware = true;
    EGO_CHECK_RETURN_FALSE(m_application->init(initData));

    return true;
}

void ego::editor::EditorApplication::releaseApplication()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_application);
}

bool ego::editor::EditorApplication::initEngine(const CommandLineOptions& _options)
{
    EGO_CHECK_RETURN_FALSE(m_application);
    EGO_CHECK_RETURN_FALSE(!m_engine);
    EGO_CHECK_RETURN_FALSE(!m_editorSession.m_engineSession && !m_sceneSession.m_engineSession);

    m_renderPluginModuleName = FileName(_options.m_renderPluginModuleName);
    m_guiRenderPluginModuleName = FileName(_options.m_guiRenderPluginModuleName);

    m_engine = new engine::Engine();
    EGO_CHECK_RETURN_FALSE(m_engine && m_engine->init());

    engine::EngineSession::InitData sceneInitData;
    sceneInitData.m_sceneRender.m_pluginModuleName = m_renderPluginModuleName;
    EGO_CHECK_RETURN_FALSE(createSceneSession(SceneTextureSize, sceneInitData, m_sceneSession));

    engine::EngineSession::InitData editorInitData;
    editorInitData.m_sceneRender.m_isEnabled = false;
    editorInitData.m_gui.m_isEnabled = true;
    editorInitData.m_gui.m_pluginModuleName = m_guiRenderPluginModuleName;
    EGO_CHECK_RETURN_FALSE(loadDefaultGuiFont(editorInitData.m_gui.m_fontAtlasDesc));
    EGO_CHECK_RETURN_FALSE(createPresentedSession("EgoEditor", SurfaceSize(1100, 700), editorInitData, m_editorSession));

    EGO_CHECK_RETURN_FALSE(initScene());
    EGO_CHECK_RETURN_FALSE(initEditorUi());

    m_editorSession.m_surface->show();
    return true;
}

void ego::editor::EditorApplication::releaseEngine()
{
    releaseEditorUi();
    releaseScene();
    releasePresentedSession(m_editorSession);
    releaseSceneSession(m_sceneSession);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_engine);
}

bool ego::editor::EditorApplication::createPresentedSession(
    const std::string& _name,
    const SurfaceSize& _size,
    engine::EngineSession::InitData& _sessionInitData,
    PresentedSession& _session)
{
    EGO_CHECK_RETURN_FALSE(m_application && m_engine);
    EGO_CHECK_RETURN_FALSE(!_session.m_engineSession && !_session.m_surface);

    const application::PresenterProviderPointer presenterProvider = m_application->getPresenterProviderPointer();
    EGO_CHECK_RETURN_FALSE(presenterProvider);

    application::PresentationDesc presentationDesc;
    presentationDesc.m_name = _name;
    presentationDesc.m_size = _size;
    const application::Presentation presentation = presenterProvider->createPresentation(presentationDesc);
    if (!presentation.m_surface || !presentation.m_graphicPresenter || !IsSurfaceValid(presentation.m_surface))
    {
        if (presentation.m_surface)
        {
            presenterProvider->destroyPresentation(presentation.m_surface);
        }

        return false;
    }

    _sessionInitData.m_mainPresentation = presentation;
    const engine::EngineSessionPointer engineSession = m_engine->createSession(_sessionInitData);
    if (!engineSession)
    {
        presenterProvider->destroyPresentation(presentation.m_surface);
        return false;
    }

    _session.m_engineSession = engineSession;
    _session.m_surface = presentation.m_surface;
    return true;
}

void ego::editor::EditorApplication::releasePresentedSession(PresentedSession& _session)
{
    if (m_engine && _session.m_engineSession)
    {
        m_engine->destroySession(_session.m_engineSession->getID());
    }
    _session.m_engineSession = nullptr;

    const application::PresenterProviderPointer presenterProvider = m_application ? m_application->getPresenterProviderPointer() : nullptr;
    if (presenterProvider && _session.m_surface)
    {
        presenterProvider->destroyPresentation(_session.m_surface);
    }
    _session.m_surface = nullptr;
}

bool ego::editor::EditorApplication::createSceneSession(
    const gpu::Texture2DSize& _size,
    engine::EngineSession::InitData& _sessionInitData,
    SceneSession& _session)
{
    EGO_CHECK_RETURN_FALSE(m_engine);
    EGO_CHECK_RETURN_FALSE(!_session.m_engineSession && !_session.m_graphicPresenter);
    EGO_CHECK_RETURN_FALSE(_size.m_x > 0 && _size.m_y > 0);

    TextureGraphicPresenterPointer graphicPresenter = new TextureGraphicPresenter();
    if (!graphicPresenter || !graphicPresenter->init(gpu::GetGraphicDevice(), _size, gpu::GraphicResourceFormat::R8G8B8A8UNorm))
    {
        EGO_SAFE_RESET_POINTER_WITH_RELEASING(graphicPresenter);

        return false;
    }

    _sessionInitData.m_mainPresentation.m_graphicPresenter = graphicPresenter;
    const engine::EngineSessionPointer engineSession = m_engine->createSession(_sessionInitData);
    if (!engineSession)
    {
        graphicPresenter->release();
        graphicPresenter = nullptr;

        return false;
    }

    _session.m_engineSession = engineSession;
    _session.m_graphicPresenter = graphicPresenter;

    return true;
}

void ego::editor::EditorApplication::releaseSceneSession(SceneSession& _session)
{
    if (m_engine && _session.m_engineSession)
    {
        m_engine->destroySession(_session.m_engineSession->getID());
    }
    _session.m_engineSession = nullptr;

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(_session.m_graphicPresenter);
}

bool ego::editor::EditorApplication::initScene()
{
    EGO_CHECK_RETURN_FALSE(m_sceneSession.m_engineSession);
    EGO_CHECK_RETURN_FALSE(!m_sceneLevel && !m_sceneCameraEntity);

    LevelController& levelController = m_sceneSession.m_engineSession->getLevelController();
    m_sceneLevel = levelController.createLevel();
    EGO_CHECK_RETURN_FALSE(m_sceneLevel);
    EGO_CHECK_RETURN_FALSE(levelController.setActiveLevel(m_sceneLevel->getID()));

    m_sceneCameraEntity = m_sceneLevel->createNode();
    EGO_CHECK_RETURN_FALSE(m_sceneCameraEntity);
    EGO_CHECK_RETURN_FALSE(m_sceneLevel->addOrReplaceComponent<render::CameraComponent>(m_sceneCameraEntity));

    m_sceneSession.m_engineSession->setRenderCameraEntity(m_sceneCameraEntity);

    return true;
}

void ego::editor::EditorApplication::releaseScene()
{
    if (m_sceneSession.m_engineSession && m_sceneLevel)
    {
        LevelController& levelController = m_sceneSession.m_engineSession->getLevelController();
        const LevelPointer activeLevel = levelController.getActiveLevel();
        if (activeLevel && activeLevel->getID() == m_sceneLevel->getID())
        {
            levelController.clearActiveLevel();
        }

        m_sceneSession.m_engineSession->clearRenderCameraEntity();
    }

    m_sceneLevel = nullptr;
    m_sceneCameraEntity = ecs::Entity();
}

void ego::editor::EditorApplication::drawSceneEditor()
{
    if (!m_sceneSession.m_engineSession || !m_sceneLevel)
    {
        return;
    }

    render::Render& render = m_sceneSession.m_engineSession->getRender();

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

bool ego::editor::EditorApplication::initEditorUi()
{
    EGO_CHECK_RETURN_FALSE(m_editorSession.m_engineSession && m_sceneSession.m_engineSession);
    EGO_CHECK_RETURN_FALSE(m_editorViewport.isExpired() && m_editorPanels.empty() && m_editorWindows.empty());

    const gui::GuiControllerPointer guiController = m_editorSession.m_engineSession->getGuiControllerPointer();
    const gui::ViewportPointer viewport = guiController ? guiController->getPrimaryViewport() : nullptr;
    EGO_CHECK_RETURN_FALSE(viewport);
    guiController->setMultiViewportEnabled(true);
    EGO_CHECK_RETURN_FALSE(guiController->isMultiViewportEnabled());
    EGO_CHECK_RETURN_FALSE(viewport->setDockingEnabled(true));

    const gui::ImagePointer sceneImage = gui::Image::Create(m_sceneSession.m_graphicPresenter->getTextureView());
    const gui::VerticalPanelPointer hierarchyPanel = gui::VerticalPanel::Create(
        {
            gui::Text::Create("Scene"),
            gui::Text::Create("  Camera"),
        });
    const gui::VerticalPanelPointer inspectorPanel = gui::VerticalPanel::Create(
        {
            gui::Text::Create("Selection: Camera"),
            gui::Text::Create("Component: CameraComponent"),
        });
    const gui::VerticalPanelPointer consolePanel = gui::VerticalPanel::Create(
        {
            gui::Text::Create("EgoEditor is ready."),
            gui::Text::Create("The scene session is always active."),
            gui::Text::Create("Static editor test primitives are rendered every frame."),
        });

    const gui::WindowPointer sceneWindow = gui::Window::Create(
        {
            .m_title = "Scene",
            .m_bounds = gui::Rect(230.0f, 20.0f, 620.0f, 480.0f),
            .m_content = sceneImage,
        });
    const gui::WindowPointer hierarchyWindow = gui::Window::Create(
        {
            .m_title = "Hierarchy",
            .m_bounds = gui::Rect(15.0f, 20.0f, 200.0f, 480.0f),
            .m_content = hierarchyPanel,
        });
    const gui::WindowPointer inspectorWindow = gui::Window::Create(
        {
            .m_title = "Inspector",
            .m_bounds = gui::Rect(865.0f, 20.0f, 220.0f, 480.0f),
            .m_content = inspectorPanel,
        });
    const gui::WindowPointer consoleWindow = gui::Window::Create(
        {
            .m_title = "Console",
            .m_bounds = gui::Rect(230.0f, 515.0f, 620.0f, 170.0f),
            .m_content = consolePanel,
        });

    EGO_CHECK_RETURN_FALSE(
        sceneImage && hierarchyPanel && inspectorPanel && consolePanel && sceneWindow && hierarchyWindow && inspectorWindow && consoleWindow);

    m_editorViewport = viewport;
    m_editorPanels = {hierarchyPanel, inspectorPanel, consolePanel};
    m_editorWindows = {sceneWindow, hierarchyWindow, inspectorWindow, consoleWindow};

    const gui::DockingSpaceID defaultSpaceID = viewport->getDefaultDockingSpaceID();
    EGO_CHECK_RETURN_FALSE(defaultSpaceID != gui::InvalidDockingSpaceID);

    EGO_CHECK_RETURN_FALSE(viewport->addWindow(sceneWindow));
    EGO_CHECK_RETURN_FALSE(viewport->moveWindow(sceneWindow, {.m_spaceID = defaultSpaceID}));

    EGO_CHECK_RETURN_FALSE(viewport->addWindow(hierarchyWindow));
    EGO_CHECK_RETURN_FALSE(
        viewport->moveWindow(hierarchyWindow, {.m_spaceID = defaultSpaceID, .m_placement = gui::DockingPlacement::Left, .m_splitRatio = 0.22f}));

    EGO_CHECK_RETURN_FALSE(viewport->addWindow(inspectorWindow));
    EGO_CHECK_RETURN_FALSE(
        viewport->moveWindow(inspectorWindow, {.m_spaceID = defaultSpaceID, .m_placement = gui::DockingPlacement::Right, .m_splitRatio = 0.28f}));

    EGO_CHECK_RETURN_FALSE(viewport->addWindow(consoleWindow));
    EGO_CHECK_RETURN_FALSE(
        viewport->moveWindow(consoleWindow, {.m_spaceID = defaultSpaceID, .m_placement = gui::DockingPlacement::Bottom, .m_splitRatio = 0.25f}));

    return true;
}

void ego::editor::EditorApplication::releaseEditorUi()
{
    for (const gui::VerticalPanelPointer& panel : m_editorPanels)
    {
        if (panel)
        {
            panel->clearChildren();
        }
    }

    for (const gui::WindowPointer& window : m_editorWindows)
    {
        if (window)
        {
            window->setContent(nullptr);
        }
    }

    const gui::GuiControllerPointer guiController = m_editorSession.m_engineSession ? m_editorSession.m_engineSession->getGuiControllerPointer() : nullptr;
    const gui::ViewportPointer primaryViewport = m_editorViewport.lock();
    for (const gui::WindowPointer& window : m_editorWindows)
    {
        const gui::ViewportPointer viewport = guiController ? guiController->findViewport(window) : primaryViewport;
        if (viewport)
        {
            viewport->removeWindow(window);
        }
    }

    m_editorWindows.clear();
    m_editorPanels.clear();
    m_editorViewport.reset();
}

bool ego::editor::EditorApplication::runMainLoop()
{
    EGO_CHECK_RETURN_FALSE(m_application && m_engine && m_editorSession.m_engineSession && m_sceneSession.m_engineSession);

    while (!m_application->isExitRequested())
    {
        m_application->processWindowEvents();
        if (m_application->isExitRequested() || !IsSurfaceValid(m_editorSession.m_surface))
        {
            break;
        }

        m_application->updateInputDevices();

        drawSceneEditor();

        EGO_CHECK_RETURN_FALSE(m_engine->tick());
    }

    return true;
}

bool ego::editor::EditorApplication::loadDefaultGuiFont(gui::FontAtlasDesc& _fontAtlasDesc) const
{
    const PlatformPointer platform = GetPlatformPointer();
    const FileSystemPointer fileSystem = platform ? platform->getFileSystem() : nullptr;
    return fileSystem && fileSystem->readFile(DefaultGuiFontPath, _fontAtlasDesc.m_fontData);
}

void ego::editor::EditorApplication::ParseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options)
{
    ArgParser argParser;
    argParser.addOptionValue("--pluginDirectory", _options.m_pluginDirectoryPath);
    argParser.addOptionValue("--profiler", _options.m_profilerPluginName);
    argParser.addOptionValue("--render", _options.m_renderPluginModuleName);
    argParser.addOptionValue("--guiRender", _options.m_guiRenderPluginModuleName);
    argParser.addOptionValue("--graphicHardware", _options.m_graphicHardwarePluginModuleName);

    argParser.parse(_argCount, _argValues);
}

bool ego::editor::EditorApplication::IsSurfaceValid(const PlatformSurfacePointer& _surface)
{
    return _surface && _surface->isValid();
}
