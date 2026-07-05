#include "EngineApplication.h"

#include <string>

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Context/ContextStack.h"
#include "EgoCore/Parsers/ArgParser/Parser.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoRuntime/Event/EventController.h"
#include "EgoRuntime/Plugin/PluginController.h"
#include "EgoRuntime/RuntimeContext.h"

#include "EgoGraphicHardware/GraphicHardwareContext.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Profile/ProfilerPlugin.h"

#include "EgoEngineFramework/ProjectReader.h"

#include "Gui/ApplicationWindowGuiViewportInputAdapter.h"

#include "Window/ApplicationWindowEvents.h"

ego::application::EngineApplication::~EngineApplication()
{
    releaseApplicationWindowEventCallbacks();
}

bool ego::application::EngineApplication::parseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options) const
{
    ArgParser argParser;
    argParser.addOptionValue("--pluginDirectory", _options.m_pluginDirectoryPath);
    argParser.addOptionValue("--profiler", _options.m_profilerPluginModuleName);
    argParser.addOptionValue("--render", _options.m_renderPluginModuleName);
    argParser.addOptionValue("--graphicHardware", _options.m_graphicHardwarePluginModuleName);
    argParser.addOptionValue("--project", _options.m_projectFilePath);

    argParser.parse(_argCount, _argValues);
    return true;
}

bool ego::application::EngineApplication::fillApplicationInitData(void* _nativeInstanceHandle, const CommandLineOptions& _options, InitData& _applicationInitData) const
{
    _applicationInitData.m_nativeInstanceHandle = _nativeInstanceHandle;
    _applicationInitData.m_pluginDirectory = FileName(_options.m_pluginDirectoryPath);

    const FileName profilerPluginModuleName(_options.m_profilerPluginModuleName);
    const FileName graphicHardwarePluginModuleName(_options.m_graphicHardwarePluginModuleName);

    if (profilerPluginModuleName || graphicHardwarePluginModuleName)
    {
        const PluginControllerPointer pluginController = GetCurrentPluginController();
        EGO_CHECK_RETURN_FALSE(pluginController);

        if (profilerPluginModuleName)
        {
            _applicationInitData.m_profilerPlugin = pluginController->loadPlugin<ProfilerPlugin>(profilerPluginModuleName);
            EGO_CHECK_RETURN_FALSE(_applicationInitData.m_profilerPlugin);
        }

        if (graphicHardwarePluginModuleName)
        {
            _applicationInitData.m_graphicHardwarePlugin = pluginController->loadPlugin<ego::gpu::GraphicHardwarePlugin>(graphicHardwarePluginModuleName);
            EGO_CHECK_RETURN_FALSE(_applicationInitData.m_graphicHardwarePlugin);
        }
    }

    return true;
}

bool ego::application::EngineApplication::fillEngineFrameworkInitData(
    const CommandLineOptions& _options,
    engine_framework::EngineFramework::InitData& _engineFrameworkInitData) const
{
    const FileName renderPluginModuleName(_options.m_renderPluginModuleName);

    if (renderPluginModuleName)
    {
        const PluginControllerPointer pluginController = GetCurrentPluginController();
        EGO_CHECK_RETURN_FALSE(pluginController);

        if (renderPluginModuleName)
        {
            _engineFrameworkInitData.m_engineInitData.m_renderPlugin = pluginController->loadPlugin<ego::render::RenderPlugin>(renderPluginModuleName);
            EGO_CHECK_RETURN_FALSE(_engineFrameworkInitData.m_engineInitData.m_renderPlugin);
        }
    }

    _engineFrameworkInitData.m_engineInitData.m_graphicPresenter = getEngineGraphicPresenter();
    return fillGuiViewportDesc(_engineFrameworkInitData.m_engineInitData);
}

bool ego::application::EngineApplication::loadProject(const CommandLineOptions& _options, engine_framework::ProjectPointer& _project) const
{
    _project = nullptr;

    FileName projectFileName(_options.m_projectFilePath);
    if (!projectFileName)
    {
        projectFileName = selectProjectFile();
    }

    return projectFileName ? loadProjectFile(projectFileName, _project) : true;
}

bool ego::application::EngineApplication::loadProjectFile(const FileName& _fileName, engine_framework::ProjectPointer& _project) const
{
    _project = nullptr;
    EGO_CHECK_RETURN_FALSE(_fileName);

    _project = new engine_framework::Project();
    EGO_CHECK_RETURN_FALSE(_project);
    EGO_CHECK_RETURN_FALSE(engine_framework::ProjectReader::ReadFromFile(_fileName, *_project));

    return true;
}

bool ego::application::EngineApplication::initApplicationContext(const ContextInitData& _applicationContextInitData)
{
    return initContext(_applicationContextInitData);
}

bool ego::application::EngineApplication::initApplicationRuntime(const InitData& _applicationInitData)
{
    return initRuntime(_applicationInitData);
}

bool ego::application::EngineApplication::initEngineFramework(const engine_framework::EngineFramework::InitData& _engineFrameworkInitData)
{
    m_engineFramework = new engine_framework::EngineFramework();
    EGO_CHECK_RETURN_FALSE(m_engineFramework);

    if (useOwnEngineFrameworkContextScope())
    {
        EGO_CHECK_RETURN_FALSE(initEngineFrameworkContextScope());
    }

    context::ScopedContextScope scopedContextScope(m_engineFrameworkContextScope);
    return m_engineFramework->init(_engineFrameworkInitData);
}

bool ego::application::EngineApplication::initWindowRuntime()
{
    EGO_CHECK_RETURN_FALSE(prepareMainWindow());
    EGO_CHECK_RETURN_FALSE(initApplicationWindowEventCallbacks());

    return true;
}

bool ego::application::EngineApplication::createGraphicPresenter()
{
    m_graphicPresenter = new WindowGraphicPresenter();
    return m_graphicPresenter.get() != nullptr;
}

bool ego::application::EngineApplication::initGraphicPresenter()
{
    context::ScopedContextScope scopedContextScope(m_engineFrameworkContextScope);
    EGO_CHECK_RETURN_FALSE(m_engineFramework && m_graphicPresenter && m_mainWindow);

    gpu::SwapChainDesc swapChainDesc;
    swapChainDesc.m_format = gpu::GraphicResourceFormat::R8G8B8A8UNorm;
    swapChainDesc.m_bufferCount = 2;

    return m_graphicPresenter->init(gpu::GetGraphicDevice(), *m_mainWindow, swapChainDesc, gpu::GetGraphicCommandQueue());
}

void ego::application::EngineApplication::runEngineWindowLoop()
{
    EGO_ASSERT(m_engineFramework);

    engine::Engine& engine = m_engineFramework->getEngine();
    ApplicationWindowManager& applicationWindowManager = getApplicationWindowManager();
    while (!engine.isStopped())
    {
        applicationWindowManager.processWindowEvents();
        bool frameResult = false;
        {
            context::ScopedContextScope scopedContextScope(m_engineFrameworkContextScope);
            frameResult = !engine.isStopped() && m_engineFramework->runFrame();
        }

        if (!frameResult)
        {
            break;
        }
    }

    context::ScopedContextScope scopedContextScope(m_engineFrameworkContextScope);
    engine.cleanResources();
}

bool ego::application::EngineApplication::isMainWindowValid() const
{
    return m_mainWindow && m_mainWindow->isValid();
}

void ego::application::EngineApplication::releaseApplicationWindowEventCallbacks()
{
    if (m_applicationWindowDestroyingEventCallbackID == InvalidEventCallbackID && m_applicationQuitRequestedEventCallbackID == InvalidEventCallbackID)
    {
        return;
    }

    EventController& eventController = context::GetRuntimeContext().getEventController();

    if (m_applicationWindowDestroyingEventCallbackID != InvalidEventCallbackID)
    {
        eventController.removeEventCallback(m_applicationWindowDestroyingEventCallbackID);
        m_applicationWindowDestroyingEventCallbackID = InvalidEventCallbackID;
    }

    if (m_applicationQuitRequestedEventCallbackID != InvalidEventCallbackID)
    {
        eventController.removeEventCallback(m_applicationQuitRequestedEventCallbackID);
        m_applicationQuitRequestedEventCallbackID = InvalidEventCallbackID;
    }
}

void ego::application::EngineApplication::releaseGraphicPresenter()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_graphicPresenter);
}

void ego::application::EngineApplication::releaseApplicationRuntime()
{
    releaseApplicationWindowEventCallbacks();
    m_mainWindow = nullptr;
    Application::release();
}

void ego::application::EngineApplication::releaseEngineFramework()
{
    const context::ContextScopePointer engineFrameworkContextScope = m_engineFrameworkContextScope;
    context::ScopedContextScope scopedContextScope(engineFrameworkContextScope);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_engineFramework);

    if (engineFrameworkContextScope)
    {
        engineFrameworkContextScope->clear();
    }

    m_engineFrameworkContextScope = nullptr;
}

ego::context::ContextScopePointer ego::application::EngineApplication::getEngineFrameworkContextScope() const
{
    return m_engineFrameworkContextScope;
}

ego::FileName ego::application::EngineApplication::selectProjectFile() const
{
    return FileName();
}

ego::GraphicPresenterPointer ego::application::EngineApplication::getEngineGraphicPresenter() const
{
    return nullptr;
}

bool ego::application::EngineApplication::useOwnEngineFrameworkContextScope() const
{
    return false;
}

bool ego::application::EngineApplication::fillGuiViewportDesc(engine::Engine::InitData& _engineInitData) const
{
    EGO_CHECK_RETURN_FALSE(m_mainWindow && m_graphicPresenter);

    ApplicationWindowGuiViewportInputAdapterPointer adapter = new ApplicationWindowGuiViewportInputAdapter();
    EGO_CHECK_RETURN_FALSE(adapter && adapter->init(m_mainWindow, m_graphicPresenter));

    _engineInitData.m_guiViewportDesc.m_inputAdapter = adapter;

    return true;
}

bool ego::application::EngineApplication::initEngineFrameworkContextScope()
{
    EGO_CHECK_RETURN_FALSE(!m_engineFrameworkContextScope);

    const context::ContextScopePointer currentScope = context::ContextStackCore::GetInstance().getCurrentScope();
    EGO_CHECK_RETURN_FALSE(currentScope);

    m_engineFrameworkContextScope = new context::ContextScope();
    EGO_CHECK_RETURN_FALSE(m_engineFrameworkContextScope);

    for (const context::ContextPointer& context : currentScope->getContexts())
    {
        m_engineFrameworkContextScope->addContext(context);
    }

    return true;
}

bool ego::application::EngineApplication::prepareMainWindow()
{
    m_mainWindow = getApplicationWindowManager().createApplicationWindow(createMainWindowDesc());
    EGO_CHECK_RETURN_FALSE(isMainWindowValid());

    return true;
}

bool ego::application::EngineApplication::initApplicationWindowEventCallbacks()
{
    EventController& eventController = context::GetRuntimeContext().getEventController();

    m_applicationWindowDestroyingEventCallbackID = eventController.addEventCallback<ApplicationWindowDestroyingEvent>(
        [this](const ApplicationWindowDestroyingEvent& _event)
        {
            onMainWindowDestroying(_event);
        });
    if (m_applicationWindowDestroyingEventCallbackID == InvalidEventCallbackID)
    {
        releaseApplicationWindowEventCallbacks();
        return false;
    }

    m_applicationQuitRequestedEventCallbackID = eventController.addEventCallback<ApplicationQuitRequestedEvent>(
        [this](const ApplicationQuitRequestedEvent& _event)
        {
            onApplicationQuitRequested(_event);
        });
    if (m_applicationQuitRequestedEventCallbackID == InvalidEventCallbackID)
    {
        releaseApplicationWindowEventCallbacks();
        return false;
    }

    return true;
}

void ego::application::EngineApplication::onMainWindowDestroying(const ApplicationWindowDestroyingEvent& _event)
{
    if (_event.m_window.get() == m_mainWindow.get() && m_engineFramework)
    {
        m_engineFramework->getEngine().stop();
    }
}

void ego::application::EngineApplication::onApplicationQuitRequested(const ApplicationQuitRequestedEvent&)
{
    if (m_engineFramework)
    {
        m_engineFramework->getEngine().stop();
    }
}
