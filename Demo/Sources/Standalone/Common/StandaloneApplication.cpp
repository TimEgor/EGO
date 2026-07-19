#include "StandaloneApplication.h"

#include "EgoCore/Parsers/ArgParser/Parser.h"
#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Project/ProjectReader.h"

namespace
{
    constexpr const char* DefaultGuiFontPath = "C:/Windows/Fonts/segoeui.ttf";
} // namespace

ego::demo::standalone::StandaloneApplication::~StandaloneApplication()
{
    release();
}

bool ego::demo::standalone::StandaloneApplication::init(void* _nativeInstanceHandle, int _argCount, char** _argValues)
{
    release();

    CommandLineOptions commandLineOptions;
    parseCommandLine(_argCount, _argValues, commandLineOptions);

    if (!initApplication(_nativeInstanceHandle, commandLineOptions))
    {
        release();
        return false;
    }

    if (!initEngine(commandLineOptions))
    {
        release();
        return false;
    }

    return true;
}

void ego::demo::standalone::StandaloneApplication::release()
{
    releaseEngine();
    releaseApplication();
}

int ego::demo::standalone::StandaloneApplication::run()
{
    if (!m_application || !m_engine || !m_engineSession)
    {
        return InitializationFailedExitCode;
    }

    const int exitCode = runMainLoop() ? SuccessExitCode : RuntimeFailedExitCode;
    release();

    return exitCode;
}

bool ego::demo::standalone::StandaloneApplication::initApplication(void* _nativeInstanceHandle, const CommandLineOptions& _options)
{
    EGO_CHECK_RETURN_FALSE(!m_application);

    m_application = new application::Application();
    EGO_CHECK_RETURN_FALSE(m_application);

    application::Application::InitData applicationInitData;
    applicationInitData.m_nativeInstanceHandle = _nativeInstanceHandle;
    applicationInitData.m_pluginDirectory = FileName(_options.m_pluginDirectoryPath);
    applicationInitData.m_profilerPluginModuleName = FileName(_options.m_profilerPluginName);
    applicationInitData.m_graphicHardwarePluginModuleName = FileName(_options.m_graphicHardwarePluginModuleName);
    applicationInitData.m_enableGraphicHardware = true;
    EGO_CHECK_RETURN_FALSE(m_application->init(applicationInitData));

    return true;
}

void ego::demo::standalone::StandaloneApplication::releaseApplication()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_application);
}

bool ego::demo::standalone::StandaloneApplication::initEngine(const CommandLineOptions& _options)
{
    EGO_CHECK_RETURN_FALSE(m_application);
    EGO_CHECK_RETURN_FALSE(!m_engine);
    EGO_CHECK_RETURN_FALSE(!m_engineSession);

    const application::ApplicationWindowPointer mainWindow = m_application->createWindow(CreateMainWindowDesc());
    EGO_CHECK_RETURN_FALSE(mainWindow && mainWindow->isValid());
    m_mainWindow = mainWindow;

    m_guiViewportSystem = new application::ApplicationGuiViewportSystem();
    EGO_CHECK_RETURN_FALSE(m_guiViewportSystem);

    EGO_CHECK_RETURN_FALSE(m_guiViewportSystem->init(m_application, mainWindow));

    engine::EngineSession::InitData sessionInitData;
    EGO_CHECK_RETURN_FALSE(fillEngineSessionInitData(_options, sessionInitData));

    sessionInitData.m_enablePresentation = true;
    engine::EngineSession::PresentationOptions& presentationOptions = sessionInitData.m_presentation;
    presentationOptions.m_viewportBackend = m_guiViewportSystem;

    sessionInitData.m_enableGui = true;
    engine::EngineSession::GuiOptions& guiOptions = sessionInitData.m_gui;
    guiOptions.m_renderPluginModuleName = FileName(_options.m_guiRenderPluginModuleName);
    EGO_CHECK_RETURN_FALSE(loadDefaultGuiFont(guiOptions.m_fontAtlasDesc));

    m_engine = new engine::Engine();
    EGO_CHECK_RETURN_FALSE(m_engine);
    EGO_CHECK_RETURN_FALSE(m_engine->init());

    m_engineSession = m_engine->createSession(sessionInitData);
    EGO_CHECK_RETURN_FALSE(m_engineSession);

    mainWindow->show();

    return true;
}

void ego::demo::standalone::StandaloneApplication::releaseEngine()
{
    if (m_engine && m_engineSession)
    {
        m_engine->destroySession(m_engineSession->getID());
    }
    m_engineSession = nullptr;
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_engine);

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_guiViewportSystem);
    m_mainWindow.reset();
}

bool ego::demo::standalone::StandaloneApplication::runMainLoop()
{
    EGO_CHECK_RETURN_FALSE(m_application);
    EGO_CHECK_RETURN_FALSE(m_engine);
    EGO_CHECK_RETURN_FALSE(m_engineSession);

    while (!m_application->isExitRequested())
    {
        m_application->processWindowEvents();
        const application::ApplicationWindowPointer mainWindow = m_mainWindow.lock();
        if (m_application->isExitRequested() || !mainWindow || !mainWindow->isValid())
        {
            break;
        }

        m_application->updateInputDevices();

        EGO_CHECK_RETURN_FALSE(m_engine->tick());
    }

    return true;
}

void ego::demo::standalone::StandaloneApplication::parseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options) const
{
    ArgParser argParser;
    argParser.addOptionValue("--pluginDirectory", _options.m_pluginDirectoryPath);
    argParser.addOptionValue("--profiler", _options.m_profilerPluginName);
    argParser.addOptionValue("--render", _options.m_renderPluginModuleName);
    argParser.addOptionValue("--guiRender", _options.m_guiRenderPluginModuleName);
    argParser.addOptionValue("--graphicHardware", _options.m_graphicHardwarePluginModuleName);
    argParser.addOptionValue("--project", _options.m_projectFilePath);

    argParser.parse(_argCount, _argValues);
}

bool ego::demo::standalone::StandaloneApplication::fillEngineSessionInitData(const CommandLineOptions& _options, engine::EngineSession::InitData& _sessionInitData)
{
    FileName projectFileName(_options.m_projectFilePath);
    if (!projectFileName)
    {
        projectFileName = selectProjectFile();
    }

    EGO_CHECK_RETURN_FALSE(loadProject(projectFileName, _sessionInitData.m_project));
    _sessionInitData.m_renderPluginModuleName = FileName(_options.m_renderPluginModuleName);
    return true;
}

bool ego::demo::standalone::StandaloneApplication::loadDefaultGuiFont(gui::FontAtlasDesc& _fontAtlasDesc) const
{
    const PlatformPointer platform = GetPlatformPointer();
    const FileSystemPointer fileSystem = platform ? platform->getFileSystem() : nullptr;
    return fileSystem && fileSystem->readFile(DefaultGuiFontPath, _fontAtlasDesc.m_fontData);
}

bool ego::demo::standalone::StandaloneApplication::loadProject(const FileName& _projectFileName, engine::ProjectPointer& _project) const
{
    _project = nullptr;
    if (!_projectFileName)
    {
        return true;
    }

    _project = new engine::Project();
    EGO_CHECK_RETURN_FALSE(_project);
    EGO_CHECK_RETURN_FALSE(engine::ProjectReader::ReadFromFile(_projectFileName, *_project));

    return true;
}

ego::WindowDesc ego::demo::standalone::StandaloneApplication::CreateMainWindowDesc()
{
    WindowDesc windowDesc;
    windowDesc.m_title = "EGO";
    windowDesc.m_size = WindowSize(500, 500);
    windowDesc.m_showOnInit = false;

    return windowDesc;
}

ego::FileName ego::demo::standalone::StandaloneApplication::selectProjectFile() const
{
    const Platform::OpenFileDialogFilter filters[] = {{"EGO Project (*.xml)", "*.xml"}, {"All Files (*.*)", "*.*"}};

    Platform::OpenFileDialogParams params;
    params.m_title = "Select EGO project";
    params.m_defaultExtension = "xml";
    params.m_filters = filters;
    params.m_filterCount = sizeof(filters) / sizeof(filters[0]);

    const PlatformSubsystemPointer platformSubsystem = subsystem::FindSubsystem<PlatformSubsystem>();
    return platformSubsystem ? platformSubsystem->getPlatform().selectOpenFile(params) : FileName();
}
