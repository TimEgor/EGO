#include "EgoDemoLauncher/LauncherApplication.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Parsers/ArgParser/Parser.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Project/ProjectReader.h"

ego::demo::launcher::LauncherApplication::~LauncherApplication()
{
    release();
}

bool ego::demo::launcher::LauncherApplication::init(void* _nativeInstanceHandle, int _argCount, char** _argValues)
{
    release();

    CommandLineOptions commandLineOptions;
    parseCommandLine(_argCount, _argValues, commandLineOptions);

    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(
        initApplicationSubsystem(_nativeInstanceHandle, commandLineOptions),
        "Failed to initialize the application subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initEngineSubsystem(commandLineOptions), "Failed to initialize the engine subsystem.");

    return true;
}

void ego::demo::launcher::LauncherApplication::release()
{
    releaseEngineSubsystem();
    releaseApplicationSubsystem();
}

int ego::demo::launcher::LauncherApplication::run()
{
    const application::ApplicationPointer application = m_applicationSubsystem ? m_applicationSubsystem->getApplicationPointer() : nullptr;
    const engine::EnginePointer engine = m_engineSubsystem ? m_engineSubsystem->getEnginePointer() : nullptr;
    if (!application || !engine || !m_engineSession)
    {
        return InitializationFailedExitCode;
    }

    const int exitCode = runMainLoop() ? SuccessExitCode : RuntimeFailedExitCode;
    release();

    return exitCode;
}

bool ego::demo::launcher::LauncherApplication::initApplicationSubsystem(void* _nativeInstanceHandle, const CommandLineOptions& _options)
{
    EGO_CHECK_RETURN_FALSE(!m_applicationSubsystem);

    m_applicationSubsystem = MakePointer<application::ApplicationSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_applicationSubsystem);

    application::Application::InitData applicationInitData;
    applicationInitData.m_nativeInstanceHandle = _nativeInstanceHandle;
    applicationInitData.m_pluginDirectory = FileName(_options.m_pluginDirectoryPath);
    applicationInitData.m_profilerPluginModuleName = FileName(_options.m_profilerPluginName);
    applicationInitData.m_graphicHardwarePluginModuleName = FileName(_options.m_graphicHardwarePluginModuleName);
    applicationInitData.m_enableGraphicHardware = true;
    EGO_CHECK_RETURN_CALL_FALSE(m_applicationSubsystem->init(applicationInitData), releaseApplicationSubsystem());

    return true;
}

void ego::demo::launcher::LauncherApplication::releaseApplicationSubsystem()
{
    EGO_CHECK_RETURN(m_applicationSubsystem);

    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    if (subsystemRegistry && subsystemRegistry->findSubsystem(m_applicationSubsystem->getType()).get() == m_applicationSubsystem.get())
    {
        subsystemRegistry->unregisterSubsystem(m_applicationSubsystem);
    }

    m_applicationSubsystem = nullptr;
}

bool ego::demo::launcher::LauncherApplication::initEngineSubsystem(const CommandLineOptions& _options)
{
    const application::ApplicationPointer application = m_applicationSubsystem ? m_applicationSubsystem->getApplicationPointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(application);
    EGO_CHECK_RETURN_FALSE(!m_engineSubsystem);
    EGO_CHECK_RETURN_FALSE(!m_engineSession);

    const application::PresenterProviderPointer presenterProvider = application->getPresenterProviderPointer();
    EGO_CHECK_RETURN_FALSE(presenterProvider);

    const application::Presentation mainPresentation = presenterProvider->createPresentation(CreateMainPresentationDesc());
    EGO_CHECK_RETURN_FALSE(mainPresentation.m_surface && mainPresentation.m_graphicPresenter);
    EGO_CHECK_RETURN_FALSE(mainPresentation.m_surface->isValid());
    m_mainSurface = mainPresentation.m_surface;

    engine::EngineSession::InitData sessionInitData;
    EGO_CHECK_RETURN_FALSE(fillEngineSessionInitData(_options, sessionInitData));

    sessionInitData.m_mainPresentation = mainPresentation;
    engine::EngineSession::GuiOptions& guiOptions = sessionInitData.m_gui;
    guiOptions.m_isEnabled = true;
    guiOptions.m_pluginModuleName = FileName(_options.m_guiRenderPluginModuleName);

    m_engineSubsystem = MakePointer<engine::EngineSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_engineSubsystem);
    EGO_CHECK_RETURN_CALL_FALSE(m_engineSubsystem->init(), releaseEngineSubsystem());

    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    EGO_CHECK_RETURN_CALL_FALSE(subsystemRegistry && subsystemRegistry->registerSubsystem(m_engineSubsystem), releaseEngineSubsystem());

    const engine::EnginePointer engine = m_engineSubsystem->getEnginePointer();
    EGO_CHECK_RETURN_FALSE(engine);

    m_engineSession = engine->createSession(sessionInitData);
    EGO_CHECK_RETURN_FALSE(m_engineSession);

    EGO_CHECK_RETURN_FALSE(mainPresentation.m_surface->show());

    return true;
}

void ego::demo::launcher::LauncherApplication::releaseEngineSubsystem()
{
    const engine::EnginePointer engine = m_engineSubsystem ? m_engineSubsystem->getEnginePointer() : nullptr;
    if (engine && m_engineSession)
    {
        engine->destroySession(m_engineSession->getID());
    }

    m_engineSession = nullptr;

    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    if (subsystemRegistry && m_engineSubsystem && subsystemRegistry->findSubsystem(m_engineSubsystem->getType()).get() == m_engineSubsystem.get())
    {
        subsystemRegistry->unregisterSubsystem(m_engineSubsystem);
    }

    m_engineSubsystem = nullptr;

    const application::ApplicationPointer application = m_applicationSubsystem ? m_applicationSubsystem->getApplicationPointer() : nullptr;
    const application::PresenterProviderPointer presenterProvider = application ? application->getPresenterProviderPointer() : nullptr;
    if (presenterProvider && m_mainSurface)
    {
        presenterProvider->destroyPresentation(m_mainSurface);
    }

    m_mainSurface = nullptr;
}

bool ego::demo::launcher::LauncherApplication::runMainLoop()
{
    const application::ApplicationPointer application = m_applicationSubsystem ? m_applicationSubsystem->getApplicationPointer() : nullptr;
    const engine::EnginePointer engine = m_engineSubsystem ? m_engineSubsystem->getEnginePointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(application);
    EGO_CHECK_RETURN_FALSE(engine);
    EGO_CHECK_RETURN_FALSE(m_engineSession);

    while (!application->isExitRequested())
    {
        application->processWindowEvents();
        const PlatformSurfacePointer mainSurface = m_mainSurface;
        if (application->isExitRequested() || !mainSurface || !mainSurface->isValid())
        {
            break;
        }

        application->updateInputDevices();

        EGO_CHECK_RETURN_FALSE(engine->tick());
    }

    return true;
}

void ego::demo::launcher::LauncherApplication::parseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options) const
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

bool ego::demo::launcher::LauncherApplication::fillEngineSessionInitData(const CommandLineOptions& _options, engine::EngineSession::InitData& _sessionInitData)
{
    FileName projectFileName(_options.m_projectFilePath);
    if (!projectFileName)
    {
        projectFileName = selectProjectFile();
    }

    EGO_CHECK_RETURN_FALSE(loadProject(projectFileName, _sessionInitData.m_project));
    _sessionInitData.m_sceneRender.m_pluginModuleName = FileName(_options.m_renderPluginModuleName);

    return true;
}

bool ego::demo::launcher::LauncherApplication::loadProject(const FileName& _projectFileName, engine::ProjectPointer& _project) const
{
    _project = nullptr;
    if (!_projectFileName)
    {
        return true;
    }

    _project = MakePointer<engine::Project>();
    EGO_CHECK_RETURN_FALSE(_project);
    EGO_CHECK_RETURN_FALSE(engine::ProjectReader::ReadFromFile(_projectFileName, *_project));

    return true;
}

ego::application::PresentationDesc ego::demo::launcher::LauncherApplication::CreateMainPresentationDesc()
{
    application::PresentationDesc presentationDesc;
    presentationDesc.m_name = "EGO";
    presentationDesc.m_size = SurfaceSize(500, 500);

    return presentationDesc;
}

ego::FileName ego::demo::launcher::LauncherApplication::selectProjectFile() const
{
    const Platform::OpenFileDialogFilter filters[] = {{"EGO Project (*.egoproj)", "*.egoproj"}, {"All Files (*.*)", "*.*"}};

    Platform::SelectFileDialogParams params;
    params.m_title = "Select EGO project";
    params.m_defaultExtension = "egoproj";
    params.m_filters = filters;
    params.m_filterCount = sizeof(filters) / sizeof(filters[0]);

    const PlatformSubsystemPointer platformSubsystem = subsystem::FindSubsystem<PlatformSubsystem>();

    return platformSubsystem ? platformSubsystem->getPlatform().selectOpenFile(params) : FileName();
}
