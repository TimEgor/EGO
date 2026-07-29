#include "EditorApplication.h"

#include <string>

#include "EgoCore/Math/Color.h"
#include "EgoCore/Parsers/ArgParser/Parser.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/Platform/FileSystem/RootedFileSystem.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoResource/ResourceController.h"
#include "EgoResource/ResourceSubsystem.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoEngine/Graphic/SceneRender/Component/CameraComponent.h"
#include "EgoEngine/Graphic/SceneRender/Render.h"

#include "EgoApplication/Presentation/PresenterProvider.h"

#include "EditorGuiStyle.h"

namespace
{
    constexpr auto EditorConfigFileName = "Editor.xml";
    constexpr ego::gpu::Texture2DSize SceneTextureSize(900, 600);
} // namespace

ego::editor::EditorApplication::~EditorApplication()
{
    release();
}

bool ego::editor::EditorApplication::init(void* _nativeInstanceHandle, int _argCount, char** _argValues)
{
    release();

    XmlDocument config;
    if (!loadConfig(config))
    {
        release();
        return false;
    }

    CommandLineOptions options;
    ParseCommandLine(_argCount, _argValues, options);

    if (!initApplication(_nativeInstanceHandle, options, config))
    {
        release();
        return false;
    }

    if (!initEditorAssets(config))
    {
        release();
        return false;
    }

    if (!initEngine(options, config))
    {
        release();
        return false;
    }

    return true;
}

void ego::editor::EditorApplication::release()
{
    releaseEngine();
    releaseEditorAssets();
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

bool ego::editor::EditorApplication::initApplication(void* _nativeInstanceHandle, const CommandLineOptions& _options, const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(!m_application);

    const XmlNode rootNode = _config.getRootNode();
    EGO_CHECK_RETURN_FALSE(rootNode && rootNode.getNameView() == "Editor");

    const XmlNode applicationNode = rootNode.getChild("Application");
    EGO_CHECK_RETURN_FALSE(applicationNode);

    m_application = new application::Application();
    EGO_CHECK_RETURN_FALSE(m_application);

    application::Application::InitData initData;
    initData.m_nativeInstanceHandle = _nativeInstanceHandle;
    initData.m_pluginDirectory = ResolveOption(_options.m_pluginDirectoryPath, applicationNode, "PluginDirectory");
    initData.m_profilerPluginModuleName = ResolveOption(_options.m_profilerPluginName, applicationNode, "ProfilerPlugin");
    initData.m_graphicHardwarePluginModuleName = ResolveOption(_options.m_graphicHardwarePluginModuleName, applicationNode, "GraphicHardwarePlugin");
    initData.m_enableGraphicHardware = true;
    EGO_CHECK_RETURN_FALSE(m_application->init(initData));

    return true;
}

void ego::editor::EditorApplication::releaseApplication()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_application);
}

bool ego::editor::EditorApplication::loadConfig(XmlDocument& _config) const
{
    return _config.loadFromFile(FileName(EditorConfigFileName));
}

bool ego::editor::EditorApplication::readDefaultFont(const XmlDocument& _config, FileName& _path, float& _size) const
{
    const XmlNode rootNode = _config.getRootNode();
    EGO_CHECK_RETURN_FALSE(rootNode && rootNode.getNameView() == "Editor");

    const XmlNode engineNode = rootNode.getChild("Engine");
    EGO_CHECK_RETURN_FALSE(engineNode);

    const XmlNode fontNode = engineNode.getChild("DefaultFont");
    EGO_CHECK_RETURN_FALSE(fontNode);

    const std::string fontPath = fontNode.getChildValueOr<std::string>("Path", std::string());
    const float fontSize = fontNode.getChildValueOr<float>("Size", 0.0f);
    EGO_CHECK_RETURN_FALSE(!fontPath.empty() && fontSize > 0.0f);

    _path = FileName(fontPath);
    _size = fontSize;

    return static_cast<bool>(_path);
}

bool ego::editor::EditorApplication::initEditorAssets(const XmlDocument& _config)
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

void ego::editor::EditorApplication::releaseEditorAssets()
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

bool ego::editor::EditorApplication::initEngine(const CommandLineOptions& _options, const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(m_application);
    EGO_CHECK_RETURN_FALSE(_config.getRootNode());
    EGO_CHECK_RETURN_FALSE(!m_engine);
    EGO_CHECK_RETURN_FALSE(!m_editorSession.m_engineSession && !m_sceneSession.m_engineSession);

    const XmlNode engineNode = _config.getRootNode().getChild("Engine");
    EGO_CHECK_RETURN_FALSE(engineNode);

    FileName defaultFontPath;
    float defaultFontSize = 0.0f;
    EGO_CHECK_RETURN_FALSE(readDefaultFont(_config, defaultFontPath, defaultFontSize));

    m_renderPluginModuleName = ResolveOption(_options.m_renderPluginModuleName, engineNode, "RenderPlugin");
    m_guiRenderPluginModuleName = ResolveOption(_options.m_guiRenderPluginModuleName, engineNode, "GuiRenderPlugin");

    m_engine = new engine::Engine();
    EGO_CHECK_RETURN_FALSE(m_engine && m_engine->init());

    engine::EngineSession::InitData sceneInitData;
    sceneInitData.m_sceneRender.m_pluginModuleName = m_renderPluginModuleName;
    EGO_CHECK_RETURN_FALSE(createSceneSession(SceneTextureSize, sceneInitData, m_sceneSession));

    engine::EngineSession::InitData editorInitData;
    editorInitData.m_sceneRender.m_isEnabled = false;
    editorInitData.m_gui.m_isEnabled = true;
    editorInitData.m_gui.m_pluginModuleName = m_guiRenderPluginModuleName;
    EGO_CHECK_RETURN_FALSE(createPresentedSession("EgoEditor", SurfaceSize(1100, 700), editorInitData, m_editorSession));

    EGO_CHECK_RETURN_FALSE(initScene());
    EGO_CHECK_RETURN_FALSE(initEditorUi(defaultFontPath, defaultFontSize));

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
    presentationDesc.m_hasFrame = false;
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

bool ego::editor::EditorApplication::initEditorUi(const FileName& _fontPath, float _fontSize)
{
    EGO_CHECK_RETURN_FALSE(m_editorSession.m_engineSession && m_sceneSession.m_engineSession);
    EGO_CHECK_RETURN_FALSE(m_editorLayerID == gui::InvalidGuiLayerID);
    EGO_CHECK_RETURN_FALSE(m_sceneSession.m_graphicPresenter);
    EGO_CHECK_RETURN_FALSE(_fontPath);

    const gui::GuiControllerPointer guiController = m_editorSession.m_engineSession->getGuiControllerPointer();
    EGO_CHECK_RETURN_FALSE(guiController);

    EGO_CHECK_RETURN_FALSE(guiController->setFont(_fontPath, _fontSize));

    const gui::GuiStyle editorStyle = CreateEditorGuiStyle();
    EGO_CHECK_RETURN_FALSE(guiController->setStyle(editorStyle));

    const gpu::TextureViewPointer sceneTexture = m_sceneSession.m_graphicPresenter->getTextureView();
    if (!sceneTexture)
    {
        return false;
    }

    m_guiLayer.setSurface(m_editorSession.m_surface);
    m_guiLayer.setSceneTexture(sceneTexture);
    m_editorLayerID = guiController->registerLayer(m_guiLayer);
    if (m_editorLayerID == gui::InvalidGuiLayerID)
    {
        m_guiLayer.reset();

        return false;
    }

    return true;
}

void ego::editor::EditorApplication::releaseEditorUi()
{
    const gui::GuiControllerPointer guiController = m_editorSession.m_engineSession ? m_editorSession.m_engineSession->getGuiControllerPointer() : nullptr;
    if (guiController)
    {
        if (m_editorLayerID != gui::InvalidGuiLayerID)
        {
            guiController->unregisterLayer(m_editorLayerID);
        }
    }

    m_guiLayer.reset();
    m_editorLayerID = gui::InvalidGuiLayerID;
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

ego::FileName ego::editor::EditorApplication::ResolveOption(const FileName& _option, const XmlNode& _configNode, const char* _configName)
{
    if (_option)
    {
        return _option;
    }

    return FileName(_configNode.getChildValueOr<std::string>(_configName, std::string()));
}

void ego::editor::EditorApplication::ParseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options)
{
    std::string pluginDirectoryPath;
    std::string profilerPluginName;
    std::string renderPluginModuleName;
    std::string guiRenderPluginModuleName;
    std::string graphicHardwarePluginModuleName;

    ArgParser argParser;
    argParser.addOptionValue("--pluginDirectory", pluginDirectoryPath);
    argParser.addOptionValue("--profiler", profilerPluginName);
    argParser.addOptionValue("--render", renderPluginModuleName);
    argParser.addOptionValue("--guiRender", guiRenderPluginModuleName);
    argParser.addOptionValue("--graphicHardware", graphicHardwarePluginModuleName);

    argParser.parse(_argCount, _argValues);

    _options.m_pluginDirectoryPath = pluginDirectoryPath;
    _options.m_profilerPluginName = profilerPluginName;
    _options.m_renderPluginModuleName = renderPluginModuleName;
    _options.m_guiRenderPluginModuleName = guiRenderPluginModuleName;
    _options.m_graphicHardwarePluginModuleName = graphicHardwarePluginModuleName;
}

bool ego::editor::EditorApplication::IsSurfaceValid(const PlatformSurfacePointer& _surface)
{
    return _surface && _surface->isValid();
}
