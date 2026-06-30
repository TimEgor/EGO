#include "LauncherApplication.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/FileDialog/FileDialog.h"
#include "EgoCore/Parsers/ArgParser/Parser.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoApplication/Window/WindowEvents.h"

#include "EgoEngine/Engine.h"
#include "EgoFramework/ProjectReader.h"

ego::demo::launcher::LauncherApplication::~LauncherApplication()
{
    releaseRuntime();
}

int ego::demo::launcher::LauncherApplication::run(void* _nativeInstanceHandle, int _argCount, char** _argValues)
{
    releaseRuntime();

    const int result = runInternal(_nativeInstanceHandle, _argCount, _argValues);
    releaseRuntime();

    return result;
}

int ego::demo::launcher::LauncherApplication::runInternal(void* _nativeInstanceHandle, int _argCount, char** _argValues)
{
    CommandLineOptions commandLineOptions;
    EGO_CHECK_RETURN_VALUE(parseCommandLine(_argCount, _argValues, commandLineOptions), FrameworkInitializationFailedExitCode);

    framework::Framework::InitData frameworkInitData;
    EGO_CHECK_RETURN_VALUE(loadProject(commandLineOptions, frameworkInitData.m_project), ProjectLoadingFailedExitCode);

    application::Application::InitData applicationInitData;
    fillApplicationInitData(_nativeInstanceHandle, commandLineOptions, applicationInitData);

    m_graphicPresenter = new WindowGraphicPresenter();
    EGO_CHECK_RETURN_VALUE(m_graphicPresenter, WindowInitializationFailedExitCode);

    fillFrameworkInitData(_nativeInstanceHandle, commandLineOptions, frameworkInitData);

    const int applicationInitResult = initApplication(applicationInitData);
    if (applicationInitResult != SuccessExitCode)
    {
        return applicationInitResult;
    }

    if (!initWindowRuntime())
    {
        return WindowInitializationFailedExitCode;
    }

    const int frameworkInitResult = initFramework(frameworkInitData);
    if (frameworkInitResult != SuccessExitCode)
    {
        return frameworkInitResult;
    }

    if (!initGraphicPresenter())
    {
        return WindowInitializationFailedExitCode;
    }

    runWindowLoop();

    return SuccessExitCode;
}

bool ego::demo::launcher::LauncherApplication::parseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options) const
{
    ArgParser argParser;
    argParser.addOptionValue("--platform", _options.m_platformPluginModuleName);
    argParser.addOptionValue("--windowSystem", _options.m_windowSystemPluginModuleName);
    argParser.addOptionValue("--profiler", _options.m_profilerPluginModuleName);
    argParser.addOptionValue("--render", _options.m_renderPluginModuleName);
    argParser.addOptionValue("--renderHardware", _options.m_renderHardwarePluginModuleName);
    argParser.addOptionValue("--project", _options.m_projectFilePath);

    argParser.parse(_argCount, _argValues);
    return true;
}

void ego::demo::launcher::LauncherApplication::fillFrameworkInitData(
    void* _nativeInstanceHandle,
    const CommandLineOptions& _options,
    framework::Framework::InitData& _frameworkInitData) const
{
    _frameworkInitData.m_engineInitData.m_nativeInstanceHandle = _nativeInstanceHandle;
    _frameworkInitData.m_engineInitData.m_platformPluginModuleName = _options.m_platformPluginModuleName;
    _frameworkInitData.m_engineInitData.m_renderPluginModuleName = _options.m_renderPluginModuleName;
    _frameworkInitData.m_engineInitData.m_renderHardwarePluginModuleName = _options.m_renderHardwarePluginModuleName;
    _frameworkInitData.m_engineInitData.m_graphicPresenter = m_graphicPresenter;
    _frameworkInitData.m_profilerPluginModuleName = _options.m_profilerPluginModuleName;
}

void ego::demo::launcher::LauncherApplication::fillApplicationInitData(
    void* _nativeInstanceHandle,
    const CommandLineOptions& _options,
    application::Application::InitData& _applicationInitData) const
{
    _applicationInitData.m_nativeInstanceHandle = _nativeInstanceHandle;
    _applicationInitData.m_windowSystemPluginModuleName = _options.m_windowSystemPluginModuleName;
}

bool ego::demo::launcher::LauncherApplication::loadProject(const CommandLineOptions& _options, framework::ProjectPointer& _project) const
{
    _project = nullptr;

    FileName projectFileName(_options.m_projectFilePath);
    if (!projectFileName)
    {
        projectFileName = selectProjectFile();
    }

    return projectFileName ? loadProjectFile(projectFileName, _project) : true;
}

ego::FileName ego::demo::launcher::LauncherApplication::selectProjectFile() const
{
    const OpenFileDialogFilter filters[] = {{"EGO Project (*.xml)", "*.xml"}, {"All Files (*.*)", "*.*"}};

    OpenFileDialogParams params;
    params.m_title = "Select EGO project";
    params.m_defaultExtension = "xml";
    params.m_filters = filters;
    params.m_filterCount = sizeof(filters) / sizeof(filters[0]);

    return SelectOpenFile(params);
}

bool ego::demo::launcher::LauncherApplication::loadProjectFile(const FileName& _fileName, framework::ProjectPointer& _project) const
{
    _project = nullptr;
    EGO_CHECK_RETURN_FALSE(_fileName);

    _project = new framework::Project();

    EGO_CHECK_RETURN_FALSE(_project);
    EGO_CHECK_RETURN_FALSE(framework::ProjectReader::ReadFromFile(_fileName, *_project));

    return true;
}

int ego::demo::launcher::LauncherApplication::initFramework(const framework::Framework::InitData& _frameworkInitData)
{
    m_framework = new framework::Framework();
    if (!m_framework)
    {
        return FrameworkInitializationFailedExitCode;
    }

    if (!framework::FrameworkCore::GetInstance().init(m_framework))
    {
        return FrameworkCoreInitializationFailedExitCode;
    }

    m_frameworkCoreInitialized = true;
    return m_framework->init(_frameworkInitData) ? SuccessExitCode : FrameworkInitializationFailedExitCode;
}

int ego::demo::launcher::LauncherApplication::initApplication(const application::Application::InitData& _applicationInitData)
{
    return application::Application::init(_applicationInitData) ? SuccessExitCode : ApplicationInitializationFailedExitCode;
}

bool ego::demo::launcher::LauncherApplication::initWindowRuntime()
{
    EGO_CHECK_RETURN_FALSE(prepareMainWindow());
    EGO_CHECK_RETURN_FALSE(initWindowEventCallbacks());

    return true;
}

bool ego::demo::launcher::LauncherApplication::initGraphicPresenter()
{
    EGO_CHECK_RETURN_FALSE(m_framework && m_graphicPresenter && m_mainWindow);

    engine::Engine& engine = m_framework->getEngine();
    gpu::SwapChainDesc swapChainDesc;
    swapChainDesc.m_format = gpu::GraphicResourceFormat::R8G8B8A8UNorm;
    swapChainDesc.m_bufferCount = 2;

    return m_graphicPresenter->init(engine.getGraphicDevice(), *m_mainWindow, swapChainDesc, engine.getRenderDeviceContext().getGraphicCommandQueue());
}

bool ego::demo::launcher::LauncherApplication::initWindowEventCallbacks()
{
    EventController& eventController = getEventController();

    m_windowDestroyingEventCallbackID = eventController.addEventCallback<WindowDestroyingEvent>(
        [this](const WindowDestroyingEvent& _event)
        {
            onMainWindowDestroying(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_windowDestroyingEventCallbackID != InvalidEventCallbackID);

    m_windowSystemQuitRequestedEventCallbackID = eventController.addEventCallback<WindowSystemQuitRequestedEvent>(
        [this](const WindowSystemQuitRequestedEvent& _event)
        {
            onWindowSystemQuitRequested(_event);
        });
    if (m_windowSystemQuitRequestedEventCallbackID == InvalidEventCallbackID)
    {
        releaseWindowEventCallbacks();
        return false;
    }

    return true;
}

void ego::demo::launcher::LauncherApplication::runWindowLoop()
{
    EGO_ASSERT(m_framework);

    engine::Engine& engine = m_framework->getEngine();
    WindowSystem& windowSystem = getWindowSystem();
    while (!engine.isStopped())
    {
        windowSystem.processEvents();
        if (engine.isStopped() || !m_framework->runFrame())
        {
            break;
        }
    }

    engine.completeRun();
}

bool ego::demo::launcher::LauncherApplication::prepareMainWindow()
{
    WindowDesc windowDesc;
    windowDesc.m_title = "EGO";
    windowDesc.m_size = WindowSize(500, 500);
    windowDesc.m_showOnInit = true;

    m_mainWindow = getWindowSystem().createWindow(windowDesc);
    EGO_CHECK_RETURN_FALSE(isMainWindowValid());

    return true;
}

bool ego::demo::launcher::LauncherApplication::isMainWindowValid() const
{
    return m_mainWindow && m_mainWindow->isValid();
}

void ego::demo::launcher::LauncherApplication::releaseWindowEventCallbacks()
{
    if (m_windowDestroyingEventCallbackID == InvalidEventCallbackID && m_windowSystemQuitRequestedEventCallbackID == InvalidEventCallbackID)
    {
        return;
    }

    EventController& eventController = getEventController();

    if (m_windowDestroyingEventCallbackID != InvalidEventCallbackID)
    {
        eventController.removeEventCallback(m_windowDestroyingEventCallbackID);
        m_windowDestroyingEventCallbackID = InvalidEventCallbackID;
    }

    if (m_windowSystemQuitRequestedEventCallbackID != InvalidEventCallbackID)
    {
        eventController.removeEventCallback(m_windowSystemQuitRequestedEventCallbackID);
        m_windowSystemQuitRequestedEventCallbackID = InvalidEventCallbackID;
    }
}

void ego::demo::launcher::LauncherApplication::releaseApplication()
{
    releaseWindowEventCallbacks();
    m_mainWindow = nullptr;
    application::Application::release();
}

void ego::demo::launcher::LauncherApplication::releaseRuntime()
{
    releaseFramework();
    releaseApplication();
}

void ego::demo::launcher::LauncherApplication::releaseFramework()
{
    if (m_frameworkCoreInitialized)
    {
        framework::FrameworkCore::GetInstance().release();
        m_frameworkCoreInitialized = false;
    }

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_framework);
    m_graphicPresenter = nullptr;
}

void ego::demo::launcher::LauncherApplication::onMainWindowDestroying(const WindowDestroyingEvent& _event)
{
    if (_event.m_window.get() == m_mainWindow.get() && m_framework)
    {
        m_framework->getEngine().stop();
    }
}

void ego::demo::launcher::LauncherApplication::onWindowSystemQuitRequested(const WindowSystemQuitRequestedEvent&)
{
    if (m_framework)
    {
        m_framework->getEngine().stop();
    }
}
