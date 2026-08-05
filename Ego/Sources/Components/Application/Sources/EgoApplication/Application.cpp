#include "Application.h"

#include <string>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Diagnostic/DiagnosticSubsystem.h"
#include "EgoCore/Event/EventSubsystem.h"
#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoInput/InputController.h"

#include "EgoPlugin/Catalog/PluginCatalogBuilder.h"
#include "EgoPlugin/PluginSubsystem.h"

#include "EgoResource/ResourceSubsystem.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "ApplicationEvents.h"
#include "ApplicationSubsystem.h"
#include "Input/ApplicationInputKeyProvider.h"
#include "Profile/ApplicationProfiler.h"
#include "Surface/PlatformSurfacePresentationProvider.h"

ego::application::Application::Application() = default;

ego::application::Application::~Application()
{
    release();
}

bool ego::application::Application::init(const InitData& _initData, const ApplicationSubsystemPointer& _applicationSubsystem)
{
    EGO_ASSERT(!m_subsystemRegistry);
    EGO_ASSERT(_applicationSubsystem);

    EGO_CHECK_RETURN_FALSE(!m_subsystemRegistry);
    EGO_CHECK_RETURN_FALSE(_applicationSubsystem);

    m_isExitRequested = false;

    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initSubsystemRegistry(), "Failed to initialize the application subsystem registry.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initDiagnosticSubsystem(), "Failed to initialize the diagnostic subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initEventSubsystem(), "Failed to initialize the event subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initPlatformSubsystem(_initData.m_nativeInstanceHandle), "Failed to initialize the platform subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initPluginSubsystem(_initData.m_pluginDirectory), "Failed to initialize the plugin subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initApplicationProfiler(_initData.m_profilerPluginModuleName), "Failed to initialize the application profiler.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(registerSubsystem(_applicationSubsystem), "Failed to register the application subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(
        !_initData.m_enableGraphicHardware || initGraphicHardwareSubsystem(_initData.m_graphicHardwarePluginModuleName),
        "Failed to initialize the graphic hardware subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initResourceSubsystem(), "Failed to initialize the resource subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initInputController(), "Failed to initialize the input controller.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(!_initData.m_enableWindowing || initWindowing(), "Failed to initialize application windowing.");

    return true;
}

void ego::application::Application::release()
{
    releaseWindowing();
    releaseInputController();
    releaseResourceSubsystem();
    releaseGraphicHardwareSubsystem();
    releaseApplicationProfiler();
    releasePluginSubsystem();
    releasePlatformSubsystem();
    releaseEventSubsystem();
    releaseDiagnosticSubsystem();
    releaseSubsystemRegistry();

    m_isExitRequested = false;
}

const ego::application::PresenterProviderPointer& ego::application::Application::getPresenterProviderPointer() const
{
    return m_presenterProvider;
}

ego::InputControllerPointer ego::application::Application::getInputControllerPointer() const
{
    return m_inputController;
}

void ego::application::Application::processWindowEvents()
{
    if (m_presenterProvider)
    {
        m_presenterProvider->processEvents();
    }
}

void ego::application::Application::updateInputDevices()
{
    if (m_inputController)
    {
        m_inputController->update();
    }
}

void ego::application::Application::requestExit()
{
    m_isExitRequested = true;
}

bool ego::application::Application::isExitRequested() const
{
    return m_isExitRequested;
}

bool ego::application::Application::initDiagnosticSubsystem()
{
    m_diagnosticSubsystem = MakePointer<DiagnosticSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_diagnosticSubsystem);
    EGO_CHECK_RETURN_FALSE(m_diagnosticSubsystem->init());
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_diagnosticSubsystem));

    return true;
}

bool ego::application::Application::initEventSubsystem()
{
    m_eventSubsystem = MakePointer<EventSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_eventSubsystem);
    EGO_CHECK_RETURN_FALSE(m_eventSubsystem->init());

    const EventControllerPointer eventController = m_eventSubsystem->getEventControllerPointer();
    EGO_CHECK_RETURN_FALSE(eventController);
    EGO_CHECK_RETURN_FALSE(eventController->registerEvent<ApplicationQuitRequestedEvent>());

    m_applicationQuitRequestedEventCallbackID = eventController->addEventCallback<ApplicationQuitRequestedEvent>(*this, &Application::handleQuitRequested);
    EGO_CHECK_RETURN_CALL_FALSE(
        m_applicationQuitRequestedEventCallbackID != InvalidEventCallbackID,
        eventController->unregisterEvent<ApplicationQuitRequestedEvent>());
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_eventSubsystem));

    return true;
}

bool ego::application::Application::initPlatformSubsystem(void* _nativeInstanceHandle)
{
    m_platformSubsystem = MakePointer<PlatformSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_platformSubsystem);

    PlatformSubsystem::InitData platformSubsystemInitData;
    platformSubsystemInitData.m_nativeInstanceHandle = _nativeInstanceHandle;
    EGO_CHECK_RETURN_FALSE(m_platformSubsystem->init(platformSubsystemInitData));
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_platformSubsystem));

    return true;
}

bool ego::application::Application::initPluginSubsystem(const FileName& _pluginDirectory)
{
    m_pluginSubsystem = MakePointer<PluginSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_pluginSubsystem);
    EGO_CHECK_RETURN_FALSE(m_pluginSubsystem->init());
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_pluginSubsystem));
    EGO_CHECK_RETURN_FALSE(registerPluginDirectory(_pluginDirectory));

    return true;
}

bool ego::application::Application::initApplicationProfiler(const FileName& _pluginModuleName)
{
    // TODO: remove it with explicit profile handler
    m_applicationProfiler = MakePointer<ApplicationProfiler>();
    EGO_CHECK_RETURN_FALSE(m_applicationProfiler);
    EGO_CHECK_RETURN_FALSE(m_applicationProfiler->init(_pluginModuleName));

    return true;
}

bool ego::application::Application::initGraphicHardwareSubsystem(const FileName& _pluginModuleName)
{
    m_graphicHardwareSubsystem = MakePointer<gpu::GraphicHardwareSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_graphicHardwareSubsystem);

    gpu::GraphicHardwareSubsystem::InitData graphicHardwareSubsystemInitData;
    graphicHardwareSubsystemInitData.m_pluginModuleName = _pluginModuleName;
    EGO_CHECK_RETURN_FALSE(m_graphicHardwareSubsystem->init(graphicHardwareSubsystemInitData));
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_graphicHardwareSubsystem));

    return true;
}

bool ego::application::Application::initResourceSubsystem()
{
    const PlatformPointer platform = GetPlatformPointer();
    const FileSystemPointer resourceFileSystem = platform ? platform->getFileSystem() : nullptr;
    EGO_CHECK_RETURN_FALSE(resourceFileSystem);

    m_resourceSubsystem = MakePointer<ResourceSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_resourceSubsystem);

    ResourceSubsystem::InitData resourceSubsystemInitData;
    resourceSubsystemInitData.m_resourceFileSystem = resourceFileSystem;
    EGO_CHECK_RETURN_FALSE(m_resourceSubsystem->init(resourceSubsystemInitData));
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_resourceSubsystem));
    EGO_CHECK_RETURN_FALSE(registerGraphicResourceProvider());

    return true;
}

void ego::application::Application::releaseResourceSubsystem()
{
    unregisterSubsystem(m_resourceSubsystem);
    m_resourceSubsystem = nullptr;
}

void ego::application::Application::releaseGraphicHardwareSubsystem()
{
    unregisterSubsystem(m_graphicHardwareSubsystem);
    m_graphicHardwareSubsystem = nullptr;
}

void ego::application::Application::releaseApplicationProfiler()
{
    m_applicationProfiler = nullptr;
}

void ego::application::Application::releasePluginSubsystem()
{
    unregisterPluginDirectory();
    unregisterSubsystem(m_pluginSubsystem);
    m_pluginSubsystem = nullptr;
}

void ego::application::Application::releasePlatformSubsystem()
{
    unregisterSubsystem(m_platformSubsystem);
    m_platformSubsystem = nullptr;
}

void ego::application::Application::releaseEventSubsystem()
{
    if (m_applicationQuitRequestedEventCallbackID != InvalidEventCallbackID)
    {
        const EventControllerPointer eventController = m_eventSubsystem ? m_eventSubsystem->getEventControllerPointer() : nullptr;
        if (eventController)
        {
            eventController->removeEventCallback(m_applicationQuitRequestedEventCallbackID);
            eventController->unregisterEvent<ApplicationQuitRequestedEvent>();
        }

        m_applicationQuitRequestedEventCallbackID = InvalidEventCallbackID;
    }

    unregisterSubsystem(m_eventSubsystem);
    m_eventSubsystem = nullptr;
}

void ego::application::Application::releaseDiagnosticSubsystem()
{
    unregisterSubsystem(m_diagnosticSubsystem);
    m_diagnosticSubsystem = nullptr;
}

bool ego::application::Application::initInputController()
{
    EGO_CHECK_RETURN_FALSE(!m_inputController && !m_platformInputKeyProvider);

    m_inputController = MakePointer<InputController>();
    EGO_CHECK_RETURN_FALSE(m_inputController && m_inputController->init());

    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN_FALSE(platform);

    InputDeviceController& platformInputDeviceController = platform->getInputDeviceController();
    EGO_CHECK_RETURN_FALSE(platformInputDeviceController.isInitialized());

    m_platformInputKeyProvider = MakePointer<ApplicationInputKeyProvider>(platformInputDeviceController);
    EGO_CHECK_RETURN_FALSE(m_platformInputKeyProvider);
    EGO_CHECK_RETURN_FALSE(m_inputController->registerKeyProvider(m_platformInputKeyProvider));

    return true;
}

void ego::application::Application::releaseInputController()
{
    if (m_inputController && m_platformInputKeyProvider)
    {
        m_inputController->unregisterKeyProvider(m_platformInputKeyProvider);
    }

    m_platformInputKeyProvider = nullptr;
    m_inputController = nullptr;
}

bool ego::application::Application::initSubsystemRegistry()
{
    EGO_CHECK_RETURN_FALSE(!m_subsystemRegistry);

    m_subsystemRegistry = MakePointer<subsystem::SubsystemRegistry>();
    EGO_CHECK_RETURN_FALSE(m_subsystemRegistry);

    if (!subsystem::SubsystemLocator::GetInstance().bind(m_subsystemRegistry))
    {
        m_subsystemRegistry = nullptr;

        return false;
    }

    return true;
}

void ego::application::Application::releaseSubsystemRegistry()
{
    if (!m_subsystemRegistry)
    {
        return;
    }

    subsystem::SubsystemLocator& subsystemLocator = subsystem::SubsystemLocator::GetInstance();
    if (subsystemLocator.getRegistryPointer().get() == m_subsystemRegistry.get())
    {
        subsystemLocator.unbind();
    }

    m_subsystemRegistry = nullptr;
}

bool ego::application::Application::registerSubsystem(const subsystem::SubsystemPointer& _subsystem)
{
    return m_subsystemRegistry && m_subsystemRegistry->registerSubsystem(_subsystem);
}

void ego::application::Application::unregisterSubsystem(const subsystem::SubsystemPointer& _subsystem)
{
    if (!_subsystem)
    {
        return;
    }

    if (m_subsystemRegistry)
    {
        const subsystem::SubsystemType subsystemType = _subsystem->getType();
        if (m_subsystemRegistry->findSubsystem(subsystemType).get() == _subsystem.get())
        {
            m_subsystemRegistry->unregisterSubsystem(_subsystem);
        }
    }
}

bool ego::application::Application::registerPluginDirectory(const FileName& _pluginDirectory)
{
    EGO_CHECK_RETURN_FALSE(m_pluginDirectoryRegistrationID == PluginCatalog::InvalidRegistrationID);

    if (!_pluginDirectory)
    {
        return true;
    }

    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN_FALSE(platform);

    const FileSystemPointer fileSystem = platform->getFileSystem();
    EGO_CHECK_RETURN_FALSE(fileSystem);

    EGO_CHECK_RETURN_FALSE(m_pluginSubsystem);

    PluginCatalogBuilder::Options options;
    options.m_mode = PluginCatalogBuilder::Mode::BestEffort;
    m_pluginDirectoryRegistrationID = PluginCatalogBuilder::AddPluginsFromPath(m_pluginSubsystem->getPluginCatalog(), *fileSystem, _pluginDirectory, options);

    return m_pluginDirectoryRegistrationID != PluginCatalog::InvalidRegistrationID;
}

void ego::application::Application::unregisterPluginDirectory()
{
    if (m_pluginDirectoryRegistrationID == PluginCatalog::InvalidRegistrationID)
    {
        return;
    }

    if (m_pluginSubsystem)
    {
        m_pluginSubsystem->getPluginCatalog().unregisterModules(m_pluginDirectoryRegistrationID);
    }

    m_pluginDirectoryRegistrationID = PluginCatalog::InvalidRegistrationID;
}

bool ego::application::Application::registerGraphicResourceProvider()
{
    EGO_CHECK_RETURN_FALSE(m_resourceSubsystem);

    if (!m_graphicHardwareSubsystem)
    {
        return true;
    }

    const GraphicDevicePointer graphicDevice = m_graphicHardwareSubsystem->getGraphicDevicePointer();
    EGO_CHECK_RETURN_FALSE(graphicDevice);

    const std::string resourceProviderName = graphicDevice->getResourceProviderName();
    EGO_CHECK_RETURN_FALSE(!resourceProviderName.empty());

    return m_resourceSubsystem->getResourceProviderPluginController().registerProvider(resourceProviderName);
}

bool ego::application::Application::initWindowing()
{
    EGO_CHECK_RETURN_FALSE(!m_presenterProvider);

    PlatformSurfacePresentationProvider::InitData presentationInitData;
    presentationInitData.m_swapChainDesc.m_format = gpu::GraphicResourceFormat::R8G8B8A8UNorm;
    presentationInitData.m_swapChainDesc.m_bufferCount = 2;

    PlatformSurfacePresentationProviderPointer presenterProvider = MakePointer<PlatformSurfacePresentationProvider>();
    EGO_CHECK_RETURN_FALSE(presenterProvider);
    EGO_CHECK_RETURN_FALSE(presenterProvider->init(presentationInitData));
    m_presenterProvider = presenterProvider;

    return true;
}

void ego::application::Application::releaseWindowing()
{
    m_presenterProvider = nullptr;
}

void ego::application::Application::handleQuitRequested(const ApplicationQuitRequestedEvent&)
{
    requestExit();
}
