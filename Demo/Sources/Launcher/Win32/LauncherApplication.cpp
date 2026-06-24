#include "LauncherApplication.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/FileDialog/FileDialog.h"
#include "EgoCore/Parsers/ArgParser/Parser.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Platform/Window/MainWindowProvider.h"
#include "EgoFramework/ProjectReader.h"


ego::demo::launcher::LauncherApplication::~LauncherApplication()
{
    releaseFramework();
}

int ego::demo::launcher::LauncherApplication::run(void* _nativeInstanceHandle, int _argCount, char** _argValues)
{
    releaseFramework();

    CommandLineOptions commandLineOptions;
    EGO_CHECK_RETURN_VALUE(
        parseCommandLine(_argCount, _argValues, commandLineOptions),
        FrameworkInitializationFailedExitCode);

    framework::Framework::InitData frameworkInitData;
    EGO_CHECK_RETURN_VALUE(loadProject(commandLineOptions, frameworkInitData.m_project), ProjectLoadingFailedExitCode);

    m_graphicPresenter = new WindowGraphicPresenter();
    EGO_CHECK_RETURN_VALUE(m_graphicPresenter, WindowInitializationFailedExitCode);

    fillFrameworkInitData(_nativeInstanceHandle, commandLineOptions, frameworkInitData);

    const int frameworkInitResult = initFramework(frameworkInitData);
    if (frameworkInitResult != SuccessExitCode)
    {
        releaseFramework();
        return frameworkInitResult;
    }

    if (!initWindowRuntime())
    {
        releaseFramework();
        return WindowInitializationFailedExitCode;
    }

    runWindowLoop();
    releaseFramework();

    return SuccessExitCode;
}

bool ego::demo::launcher::LauncherApplication::parseCommandLine(
    int _argCount,
    char** _argValues,
    CommandLineOptions& _options) const
{
    ArgParser argParser;
    argParser.addOptionValue("--platform", _options.m_platformPluginModuleName);
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

bool ego::demo::launcher::LauncherApplication::loadProject(
    const CommandLineOptions& _options,
    framework::ProjectPointer& _project) const
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

bool ego::demo::launcher::LauncherApplication::loadProjectFile(
    const FileName& _fileName,
    framework::ProjectPointer& _project) const
{
    _project = nullptr;
    EGO_CHECK_RETURN_FALSE(_fileName);

    _project = new framework::Project();
    EGO_CHECK_RETURN_FALSE(_project);
    EGO_CHECK_RETURN_FALSE(framework::ProjectReader::ReadFromFile(_fileName, *_project));

    return true;
}

int ego::demo::launcher::LauncherApplication::initFramework(
    const framework::Framework::InitData& _frameworkInitData)
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

bool ego::demo::launcher::LauncherApplication::initWindowRuntime()
{
    EGO_CHECK_RETURN_FALSE(m_framework && m_graphicPresenter);

    engine::Engine& engine = m_framework->getEngine();
    EGO_CHECK_RETURN_FALSE(prepareMainWindow(engine));

    gpu::SwapChainDesc swapChainDesc;
    swapChainDesc.m_format = gpu::GraphicResourceFormat::R8G8B8A8UNorm;
    swapChainDesc.m_bufferCount = 2;

    const WindowPointer mainWindow = engine.getPlatform().getMainWindowProvider().getMainWindow();
    return m_graphicPresenter->init(
        engine.getGraphicDevice(),
        *mainWindow,
        swapChainDesc,
        engine.getRenderDeviceContext().getGraphicCommandQueue());
}

void ego::demo::launcher::LauncherApplication::runWindowLoop() const
{
    EGO_ASSERT(m_framework);

    engine::Engine& engine = m_framework->getEngine();
    while (!engine.isStopped())
    {
        if (!isMainWindowValid(engine))
        {
            engine.stop();
            break;
        }

        if (!m_framework->runFrame())
        {
            break;
        }
    }

    engine.completeRun();
}

bool ego::demo::launcher::LauncherApplication::prepareMainWindow(engine::Engine& _engine) const
{
    MainWindowProvider& mainWindowProvider = _engine.getPlatform().getMainWindowProvider();
    if (!mainWindowProvider.isWindowPlatformProvided())
    {
        EGO_CHECK_RETURN_FALSE(mainWindowProvider.prepareMainWindow("EGO", WindowSize(500, 500)));
    }

    const WindowPointer mainWindow = mainWindowProvider.getMainWindow();
    EGO_CHECK_RETURN_FALSE(mainWindow && mainWindow->isValid());

    return true;
}

bool ego::demo::launcher::LauncherApplication::isMainWindowValid(const engine::Engine& _engine) const
{
    const MainWindowProvider& mainWindowProvider = _engine.getPlatform().getMainWindowProvider();
    const WindowPointer mainWindow = mainWindowProvider.getMainWindow();

    return mainWindow && mainWindow->isValid();
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
