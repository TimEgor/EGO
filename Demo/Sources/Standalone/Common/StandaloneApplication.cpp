#include "StandaloneApplication.h"

#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/Parsers/ArgParser/Parser.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEvent/EventSubsystem.h"

#include "EgoEngine/Project/ProjectReader.h"

#include "EgoApplication/Window/ApplicationWindowEvents.h"

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

    engine::EngineSession::InitData sessionInitData;
    EGO_CHECK_RETURN_FALSE(fillEngineSessionInitData(_options, sessionInitData));

    m_engine = new engine::Engine();
    EGO_CHECK_RETURN_FALSE(m_engine);
    EGO_CHECK_RETURN_FALSE(m_engine->init());

    m_engineSession = m_engine->createSession(sessionInitData);
    EGO_CHECK_RETURN_FALSE(m_engineSession);

    const EventSubsystemPointer eventSubsystem = subsystem::FindSubsystem<EventSubsystem>();
    EGO_CHECK_RETURN_FALSE(eventSubsystem);

    const EventControllerPointer eventController = eventSubsystem->getEventControllerPointer();
    EGO_CHECK_RETURN_FALSE(eventController);

    const application::ApplicationWindowPointer mainWindow = m_application->createWindow(CreateMainWindowDesc());
    EGO_CHECK_RETURN_FALSE(mainWindow && mainWindow->isValid());
    m_mainWindow = mainWindow;

    m_mainWindowPresentation = new application::EngineWindowPresentation();
    EGO_CHECK_RETURN_FALSE(m_mainWindowPresentation);
    EGO_CHECK_RETURN_FALSE(m_mainWindowPresentation->init(mainWindow));
    EGO_CHECK_RETURN_FALSE(m_engineSession->setGraphicPresenter(m_mainWindowPresentation->getGraphicPresenterPointer()));

    m_mainWindowInputBinding = new application::EngineWindowInputBinding();
    EGO_CHECK_RETURN_FALSE(m_mainWindowInputBinding);
    EGO_CHECK_RETURN_FALSE(m_mainWindowInputBinding->init(m_engineSession, eventController, mainWindow));
    EGO_CHECK_RETURN_FALSE(registerMainWindowEvents(mainWindow));

    return true;
}

void ego::demo::standalone::StandaloneApplication::releaseEngine()
{
    unregisterMainWindowEvents();

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_mainWindowInputBinding);
    if (m_engineSession)
    {
        m_engineSession->clearGraphicPresenter();
    }
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_mainWindowPresentation);
    m_mainWindow.reset();

    if (m_engine && m_engineSession)
    {
        m_engine->destroySession(m_engineSession->getID());
    }
    m_engineSession = nullptr;
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_engine);
}

bool ego::demo::standalone::StandaloneApplication::runMainLoop()
{
    EGO_CHECK_RETURN_FALSE(m_application);
    EGO_CHECK_RETURN_FALSE(m_engine);
    EGO_CHECK_RETURN_FALSE(m_engineSession);

    while (!m_application->isExitRequested())
    {
        m_application->processWindowEvents();
        if (m_application->isExitRequested())
        {
            break;
        }

        if (m_mainWindowPresentation)
        {
            EGO_CHECK_RETURN_FALSE(m_mainWindowPresentation->update());
        }

        m_application->updateInputDevices();

        if (m_mainWindowInputBinding)
        {
            m_mainWindowInputBinding->updateInputDevices();
        }

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

bool ego::demo::standalone::StandaloneApplication::registerMainWindowEvents(const application::ApplicationWindowPointer& _mainWindow)
{
    EGO_CHECK_RETURN_FALSE(m_mainWindowDestroyingEventCallbackID == InvalidEventCallbackID);

    EGO_CHECK_RETURN_FALSE(m_application);

    const EventSubsystemPointer eventSubsystem = subsystem::FindSubsystem<EventSubsystem>();
    EGO_CHECK_RETURN_FALSE(eventSubsystem);

    const EventControllerPointer eventController = eventSubsystem->getEventControllerPointer();
    EGO_CHECK_RETURN_FALSE(eventController);

    m_mainWindowDestroyingEventCallbackID = eventController->addEventCallback<application::ApplicationWindowDestroyingEvent>(
        [this](const application::ApplicationWindowDestroyingEvent& _event)
        {
            const application::ApplicationWindowPointer currentMainWindow = m_mainWindow.lock();
            if (!currentMainWindow || _event.m_window.get() != currentMainWindow.get())
            {
                return;
            }

            if (m_application)
            {
                m_application->requestExit();
            }
        });

    return m_mainWindowDestroyingEventCallbackID != InvalidEventCallbackID;
}

void ego::demo::standalone::StandaloneApplication::unregisterMainWindowEvents()
{
    if (m_mainWindowDestroyingEventCallbackID == InvalidEventCallbackID)
    {
        return;
    }

    const EventSubsystemPointer eventSubsystem = subsystem::FindSubsystem<EventSubsystem>();
    const EventControllerPointer eventController = eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
    if (eventController)
    {
        eventController->removeEventCallback(m_mainWindowDestroyingEventCallbackID);
    }

    m_mainWindowDestroyingEventCallbackID = InvalidEventCallbackID;
}

ego::WindowDesc ego::demo::standalone::StandaloneApplication::CreateMainWindowDesc()
{
    WindowDesc windowDesc;
    windowDesc.m_title = "EGO";
    windowDesc.m_size = WindowSize(500, 500);
    windowDesc.m_showOnInit = true;

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
