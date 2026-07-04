#include "LauncherApplication.h"

#include "EgoCore/UtilsMacros.h"

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
    const int initializationResult = initRuntimeFromCommandLine(_nativeInstanceHandle, _argCount, _argValues);
    if (initializationResult != SuccessExitCode)
    {
        return initializationResult;
    }

    if (!initGraphicPresenter())
    {
        return WindowInitializationFailedExitCode;
    }

    runEngineWindowLoop();

    return SuccessExitCode;
}

int ego::demo::launcher::LauncherApplication::initRuntimeFromCommandLine(void* _nativeInstanceHandle, int _argCount, char** _argValues)
{
    CommandLineOptions commandLineOptions;
    EGO_CHECK_RETURN_VALUE(parseCommandLine(_argCount, _argValues, commandLineOptions), EngineFrameworkInitializationFailedExitCode);

    const int applicationInitializationResult = initApplicationFromOptions(_nativeInstanceHandle, commandLineOptions);
    if (applicationInitializationResult != SuccessExitCode)
    {
        return applicationInitializationResult;
    }

    return initEngineFrameworkFromOptions(commandLineOptions);
}

int ego::demo::launcher::LauncherApplication::initApplicationFromOptions(void* _nativeInstanceHandle, const CommandLineOptions& _options)
{
    ContextInitData applicationContextInitData;
    applicationContextInitData.m_nativeInstanceHandle = _nativeInstanceHandle;
    if (!initApplicationContext(applicationContextInitData))
    {
        return ApplicationInitializationFailedExitCode;
    }

    InitData applicationInitData;
    EGO_CHECK_RETURN_VALUE(fillApplicationInitData(_nativeInstanceHandle, _options, applicationInitData), ApplicationInitializationFailedExitCode);

    if (!initApplicationRuntime(applicationInitData))
    {
        return ApplicationInitializationFailedExitCode;
    }

    return SuccessExitCode;
}

int ego::demo::launcher::LauncherApplication::initEngineFrameworkFromOptions(const CommandLineOptions& _options)
{
    engine_framework::EngineFramework::InitData engineFrameworkInitData;
    EGO_CHECK_RETURN_VALUE(loadProject(_options, engineFrameworkInitData.m_project), ProjectLoadingFailedExitCode);
    EGO_CHECK_RETURN_VALUE(createGraphicPresenter(), WindowInitializationFailedExitCode);
    EGO_CHECK_RETURN_VALUE(fillEngineFrameworkInitData(_options, engineFrameworkInitData), EngineFrameworkInitializationFailedExitCode);

    if (!initWindowRuntime())
    {
        return WindowInitializationFailedExitCode;
    }

    if (!initEngineFramework(engineFrameworkInitData))
    {
        return EngineFrameworkInitializationFailedExitCode;
    }

    return SuccessExitCode;
}

ego::WindowDesc ego::demo::launcher::LauncherApplication::createMainWindowDesc() const
{
    WindowDesc windowDesc;
    windowDesc.m_title = "EGO";
    windowDesc.m_size = WindowSize(500, 500);
    windowDesc.m_showOnInit = true;

    return windowDesc;
}

ego::FileName ego::demo::launcher::LauncherApplication::selectProjectFile() const
{
    const Platform::OpenFileDialogFilter filters[] = {{"EGO Project (*.xml)", "*.xml"}, {"All Files (*.*)", "*.*"}};

    Platform::OpenFileDialogParams params;
    params.m_title = "Select EGO project";
    params.m_defaultExtension = "xml";
    params.m_filters = filters;
    params.m_filterCount = sizeof(filters) / sizeof(filters[0]);

    return getPlatform().selectOpenFile(params);
}

ego::GraphicPresenterPointer ego::demo::launcher::LauncherApplication::getEngineGraphicPresenter() const
{
    return m_graphicPresenter;
}

void ego::demo::launcher::LauncherApplication::releaseRuntime()
{
    releaseEngineFramework();
    m_graphicPresenter = nullptr;
    releaseApplicationRuntime();
}
