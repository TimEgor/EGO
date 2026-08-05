#include "Runtime.h"

#include <string>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Diagnostic/DiagnosticSubsystem.h"
#include "EgoCore/Event/EventSubsystem.h"
#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoInput/InputController.h"

#include "EgoPlugin/Catalog/PluginCatalogBuilder.h"
#include "EgoPlugin/PluginSubsystem.h"

#include "EgoResource/ResourceSubsystem.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "RuntimeEvents.h"
#include "Input/RuntimeInputKeyProvider.h"
#include "Profile/RuntimeProfiler.h"
#include "Surface/PlatformSurfacePresentationProvider.h"

ego::runtime::Runtime::Runtime() = default;

ego::runtime::Runtime::~Runtime()
{
    release();
}

bool ego::runtime::Runtime::init(const InitData& _initData)
{
    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();

    EGO_ASSERT(!m_isInitialized);
    EGO_ASSERT(subsystemRegistry);

    EGO_CHECK_RETURN_FALSE(!m_isInitialized);
    EGO_CHECK_RETURN_FALSE(subsystemRegistry);

    m_isExitRequested = false;

    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initDiagnosticSubsystem(), "Failed to initialize the diagnostic subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initEventSubsystem(), "Failed to initialize the event subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initPlatformSubsystem(_initData.m_nativeInstanceHandle), "Failed to initialize the platform subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initPluginSubsystem(_initData.m_pluginDirectory), "Failed to initialize the plugin subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initRuntimeProfiler(_initData.m_profilerPluginModuleName), "Failed to initialize the runtime profiler.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(
        !_initData.m_enableGraphicHardware || initGraphicHardwareSubsystem(_initData.m_graphicHardwarePluginModuleName),
        "Failed to initialize the graphic hardware subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initResourceSubsystem(), "Failed to initialize the resource subsystem.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initInputController(), "Failed to initialize the input controller.");
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(!_initData.m_enableWindowing || initWindowing(), "Failed to initialize runtime windowing.");

    m_isInitialized = true;

    return true;
}

void ego::runtime::Runtime::release()
{
    releaseWindowing();
    releaseInputController();
    releaseResourceSubsystem();
    releaseGraphicHardwareSubsystem();
    releaseRuntimeProfiler();
    releasePluginSubsystem();
    releasePlatformSubsystem();
    releaseEventSubsystem();
    releaseDiagnosticSubsystem();

    m_isExitRequested = false;
    m_isInitialized = false;
}

const ego::runtime::PresenterProviderPointer& ego::runtime::Runtime::getPresenterProviderPointer() const
{
    return m_presenterProvider;
}

ego::InputControllerPointer ego::runtime::Runtime::getInputControllerPointer() const
{
    return m_inputController;
}

void ego::runtime::Runtime::processWindowEvents()
{
    if (m_presenterProvider)
    {
        m_presenterProvider->processEvents();
    }
}

void ego::runtime::Runtime::updateInputDevices()
{
    if (m_inputController)
    {
        m_inputController->update();
    }
}

void ego::runtime::Runtime::requestExit()
{
    m_isExitRequested = true;
}

bool ego::runtime::Runtime::isExitRequested() const
{
    return m_isExitRequested;
}

bool ego::runtime::Runtime::initDiagnosticSubsystem()
{
    m_diagnosticSubsystem = MakePointer<DiagnosticSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_diagnosticSubsystem);
    EGO_CHECK_RETURN_FALSE(m_diagnosticSubsystem->init());
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_diagnosticSubsystem));

    return true;
}

bool ego::runtime::Runtime::initEventSubsystem()
{
    m_eventSubsystem = MakePointer<EventSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_eventSubsystem);
    EGO_CHECK_RETURN_FALSE(m_eventSubsystem->init());

    const EventControllerPointer eventController = m_eventSubsystem->getEventControllerPointer();
    EGO_CHECK_RETURN_FALSE(eventController);
    EGO_CHECK_RETURN_FALSE(eventController->registerEvent<RuntimeQuitRequestedEvent>());

    m_runtimeQuitRequestedEventCallbackID = eventController->addEventCallback<RuntimeQuitRequestedEvent>(*this, &Runtime::handleQuitRequested);
    EGO_CHECK_RETURN_CALL_FALSE(m_runtimeQuitRequestedEventCallbackID != InvalidEventCallbackID, eventController->unregisterEvent<RuntimeQuitRequestedEvent>());
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_eventSubsystem));

    return true;
}

bool ego::runtime::Runtime::initPlatformSubsystem(void* _nativeInstanceHandle)
{
    m_platformSubsystem = MakePointer<PlatformSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_platformSubsystem);

    PlatformSubsystem::InitData platformSubsystemInitData;
    platformSubsystemInitData.m_nativeInstanceHandle = _nativeInstanceHandle;
    EGO_CHECK_RETURN_FALSE(m_platformSubsystem->init(platformSubsystemInitData));
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_platformSubsystem));

    return true;
}

bool ego::runtime::Runtime::initPluginSubsystem(const FileName& _pluginDirectory)
{
    m_pluginSubsystem = MakePointer<PluginSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_pluginSubsystem);
    EGO_CHECK_RETURN_FALSE(m_pluginSubsystem->init());
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_pluginSubsystem));
    EGO_CHECK_RETURN_FALSE(registerPluginDirectory(_pluginDirectory));

    return true;
}

bool ego::runtime::Runtime::initRuntimeProfiler(const FileName& _pluginModuleName)
{
    // TODO: remove it with explicit profile handler
    m_runtimeProfiler = MakePointer<RuntimeProfiler>();
    EGO_CHECK_RETURN_FALSE(m_runtimeProfiler);
    EGO_CHECK_RETURN_FALSE(m_runtimeProfiler->init(_pluginModuleName));

    return true;
}

bool ego::runtime::Runtime::initGraphicHardwareSubsystem(const FileName& _pluginModuleName)
{
    m_graphicHardwareSubsystem = MakePointer<gpu::GraphicHardwareSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_graphicHardwareSubsystem);

    gpu::GraphicHardwareSubsystem::InitData graphicHardwareSubsystemInitData;
    graphicHardwareSubsystemInitData.m_pluginModuleName = _pluginModuleName;
    EGO_CHECK_RETURN_FALSE(m_graphicHardwareSubsystem->init(graphicHardwareSubsystemInitData));
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_graphicHardwareSubsystem));

    return true;
}

bool ego::runtime::Runtime::initResourceSubsystem()
{
    const PlatformPointer platform = GetPlatformPointer();
    const FileSystemPointer resourceFileSystem = platform ? platform->getFileSystem() : nullptr;
    EGO_CHECK_RETURN_FALSE(resourceFileSystem);

    m_resourceSubsystem = MakePointer<ResourceSubsystem>();
    EGO_CHECK_RETURN_FALSE(m_resourceSubsystem);

    ResourceSubsystem::InitData resourceSubsystemInitData;
    resourceSubsystemInitData.m_resourceFileSystem = resourceFileSystem;
    resourceSubsystemInitData.m_resourceJobThreadCount = 2;
    EGO_CHECK_RETURN_FALSE(m_resourceSubsystem->init(resourceSubsystemInitData));
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_resourceSubsystem));
    EGO_CHECK_RETURN_FALSE(registerGraphicResourceProvider());

    return true;
}

void ego::runtime::Runtime::releaseResourceSubsystem()
{
    unregisterSubsystem(m_resourceSubsystem);
    m_resourceSubsystem = nullptr;
}

void ego::runtime::Runtime::releaseGraphicHardwareSubsystem()
{
    unregisterSubsystem(m_graphicHardwareSubsystem);
    m_graphicHardwareSubsystem = nullptr;
}

void ego::runtime::Runtime::releaseRuntimeProfiler()
{
    m_runtimeProfiler = nullptr;
}

void ego::runtime::Runtime::releasePluginSubsystem()
{
    unregisterPluginDirectory();
    unregisterSubsystem(m_pluginSubsystem);
    m_pluginSubsystem = nullptr;
}

void ego::runtime::Runtime::releasePlatformSubsystem()
{
    unregisterSubsystem(m_platformSubsystem);
    m_platformSubsystem = nullptr;
}

void ego::runtime::Runtime::releaseEventSubsystem()
{
    if (m_runtimeQuitRequestedEventCallbackID != InvalidEventCallbackID)
    {
        const EventControllerPointer eventController = m_eventSubsystem ? m_eventSubsystem->getEventControllerPointer() : nullptr;
        if (eventController)
        {
            eventController->removeEventCallback(m_runtimeQuitRequestedEventCallbackID);
            eventController->unregisterEvent<RuntimeQuitRequestedEvent>();
        }

        m_runtimeQuitRequestedEventCallbackID = InvalidEventCallbackID;
    }

    unregisterSubsystem(m_eventSubsystem);
    m_eventSubsystem = nullptr;
}

void ego::runtime::Runtime::releaseDiagnosticSubsystem()
{
    unregisterSubsystem(m_diagnosticSubsystem);
    m_diagnosticSubsystem = nullptr;
}

bool ego::runtime::Runtime::initInputController()
{
    EGO_CHECK_RETURN_FALSE(!m_inputController && !m_platformInputKeyProvider);

    m_inputController = MakePointer<InputController>();
    EGO_CHECK_RETURN_FALSE(m_inputController && m_inputController->init());

    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN_FALSE(platform);

    InputDeviceController& platformInputDeviceController = platform->getInputDeviceController();
    EGO_CHECK_RETURN_FALSE(platformInputDeviceController.isInitialized());

    m_platformInputKeyProvider = MakePointer<RuntimeInputKeyProvider>(platformInputDeviceController);
    EGO_CHECK_RETURN_FALSE(m_platformInputKeyProvider);
    EGO_CHECK_RETURN_FALSE(m_inputController->registerKeyProvider(m_platformInputKeyProvider));

    return true;
}

void ego::runtime::Runtime::releaseInputController()
{
    if (m_inputController && m_platformInputKeyProvider)
    {
        m_inputController->unregisterKeyProvider(m_platformInputKeyProvider);
    }

    m_platformInputKeyProvider = nullptr;
    m_inputController = nullptr;
}

bool ego::runtime::Runtime::registerSubsystem(const subsystem::SubsystemPointer& _subsystem)
{
    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();

    return subsystemRegistry && subsystemRegistry->registerSubsystem(_subsystem);
}

void ego::runtime::Runtime::unregisterSubsystem(const subsystem::SubsystemPointer& _subsystem)
{
    if (!_subsystem)
    {
        return;
    }

    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    if (subsystemRegistry)
    {
        const subsystem::SubsystemType subsystemType = _subsystem->getType();
        if (subsystemRegistry->findSubsystem(subsystemType).get() == _subsystem.get())
        {
            subsystemRegistry->unregisterSubsystem(_subsystem);
        }
    }
}

bool ego::runtime::Runtime::registerPluginDirectory(const FileName& _pluginDirectory)
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

void ego::runtime::Runtime::unregisterPluginDirectory()
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

bool ego::runtime::Runtime::registerGraphicResourceProvider()
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

bool ego::runtime::Runtime::initWindowing()
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

void ego::runtime::Runtime::releaseWindowing()
{
    m_presenterProvider = nullptr;
}

void ego::runtime::Runtime::handleQuitRequested(const RuntimeQuitRequestedEvent&)
{
    requestExit();
}
