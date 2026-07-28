#include "LauncherApplication.h"

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

void ego::demo::launcher::LauncherApplication::release()
{
    releaseEngine();
    releaseApplication();
}

int ego::demo::launcher::LauncherApplication::run()
{
    if (!m_application || !m_engine || !m_engineSession)
    {
        return InitializationFailedExitCode;
    }

    const int exitCode = runMainLoop() ? SuccessExitCode : RuntimeFailedExitCode;
    release();

    return exitCode;
}

bool ego::demo::launcher::LauncherApplication::initApplication(void* _nativeInstanceHandle, const CommandLineOptions& _options)
{
    EGO_CHECK_RETURN_FALSE(!m_application);

    m_application = MakePointer<application::Application>();
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

void ego::demo::launcher::LauncherApplication::releaseApplication()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_application);
}

bool ego::demo::launcher::LauncherApplication::initEngine(const CommandLineOptions& _options)
{
    EGO_CHECK_RETURN_FALSE(m_application);
    EGO_CHECK_RETURN_FALSE(!m_engine);
    EGO_CHECK_RETURN_FALSE(!m_engineSession);

    const application::PresenterProviderPointer presenterProvider = m_application->getPresenterProviderPointer();
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

    m_engine = MakePointer<engine::Engine>();
    EGO_CHECK_RETURN_FALSE(m_engine);
    EGO_CHECK_RETURN_FALSE(m_engine->init());

    m_engineSession = m_engine->createSession(sessionInitData);
    EGO_CHECK_RETURN_FALSE(m_engineSession);

    EGO_CHECK_RETURN_FALSE(mainPresentation.m_surface->show());

    return true;
}

void ego::demo::launcher::LauncherApplication::releaseEngine()
{
    if (m_engine && m_engineSession)
    {
        m_engine->destroySession(m_engineSession->getID());
    }
    m_engineSession = nullptr;
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_engine);

    const application::PresenterProviderPointer presenterProvider = m_application ? m_application->getPresenterProviderPointer() : nullptr;
    if (presenterProvider && m_mainSurface)
    {
        presenterProvider->destroyPresentation(m_mainSurface);
    }
    m_mainSurface = nullptr;
}

bool ego::demo::launcher::LauncherApplication::runMainLoop()
{
    EGO_CHECK_RETURN_FALSE(m_application);
    EGO_CHECK_RETURN_FALSE(m_engine);
    EGO_CHECK_RETURN_FALSE(m_engineSession);

    while (!m_application->isExitRequested())
    {
        m_application->processWindowEvents();
        const PlatformSurfacePointer mainSurface = m_mainSurface;
        if (m_application->isExitRequested() || !mainSurface || !mainSurface->isValid())
        {
            break;
        }

        m_application->updateInputDevices();

        EGO_CHECK_RETURN_FALSE(m_engine->tick());
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
    const Platform::OpenFileDialogFilter filters[] = {{"EGO Project (*.xml)", "*.xml"}, {"All Files (*.*)", "*.*"}};

    Platform::OpenFileDialogParams params;
    params.m_title = "Select EGO project";
    params.m_defaultExtension = "xml";
    params.m_filters = filters;
    params.m_filterCount = sizeof(filters) / sizeof(filters[0]);

    const PlatformSubsystemPointer platformSubsystem = subsystem::FindSubsystem<PlatformSubsystem>();
    return platformSubsystem ? platformSubsystem->getPlatform().selectOpenFile(params) : FileName();
}
