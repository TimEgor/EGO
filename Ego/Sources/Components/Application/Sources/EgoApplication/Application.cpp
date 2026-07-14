#include "Application.h"

#include <string>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Diagnostic/DiagnosticSubsystem.h"
#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEvent/EventSubsystem.h"

#include "EgoPlugin/PlatformPluginSubsystem.h"
#include "EgoPlugin/PluginCatalogBuilder.h"
#include "EgoPlugin/PluginSubsystem.h"

#include "EgoResource/ResourceSubsystem.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "Profile/ApplicationProfiler.h"
#include "Window/ApplicationWindowController.h"
#include "Window/ApplicationWindowEvents.h"

ego::application::Application::Application() = default;

ego::application::Application::~Application()
{
    release();
}

bool ego::application::Application::init(const InitData& _initData)
{
    EGO_CHECK_RETURN_FALSE(!m_subsystemRegistry);

    m_isExitRequested = false;

    EGO_CHECK_INITIALIZATION(initSubsystems(_initData));

    if (_initData.m_enableWindowing)
    {
        EGO_CHECK_INITIALIZATION(initWindowing());
    }

    return true;
}

void ego::application::Application::release()
{
    releaseWindowing();
    releaseSubsystems();

    m_isExitRequested = false;
}

ego::application::ApplicationWindowPointer ego::application::Application::createWindow(const WindowDesc& _desc)
{
    return m_windowController ? m_windowController->createApplicationWindow(_desc) : nullptr;
}

void ego::application::Application::processWindowEvents()
{
    if (m_windowController)
    {
        m_windowController->processWindowEvents();
    }
}

void ego::application::Application::updateInputDevices()
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_ASSERT(platform);
    if (platform)
    {
        platform->getInputDeviceController().update();
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

bool ego::application::Application::initSubsystems(const InitData& _initData)
{
    EGO_CHECK_RETURN_FALSE(initSubsystemRegistry());

    m_diagnosticSubsystem = new DiagnosticSubsystem();
    EGO_CHECK_RETURN_FALSE(m_diagnosticSubsystem);
    EGO_CHECK_RETURN_FALSE(m_diagnosticSubsystem->init());
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_diagnosticSubsystem));

    m_platformSubsystem = new PlatformSubsystem();
    EGO_CHECK_RETURN_FALSE(m_platformSubsystem);

    PlatformSubsystem::InitData platformSubsystemInitData;
    platformSubsystemInitData.m_nativeInstanceHandle = _initData.m_nativeInstanceHandle;
    EGO_CHECK_RETURN_FALSE(m_platformSubsystem->init(platformSubsystemInitData));
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_platformSubsystem));

    m_platformPluginSubsystem = new PlatformPluginSubsystem();
    EGO_CHECK_RETURN_FALSE(m_platformPluginSubsystem);
    EGO_CHECK_RETURN_FALSE(m_platformPluginSubsystem->init());
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_platformPluginSubsystem));

    m_pluginSubsystem = new PluginSubsystem();
    EGO_CHECK_RETURN_FALSE(m_pluginSubsystem);
    EGO_CHECK_RETURN_FALSE(m_pluginSubsystem->init());
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_pluginSubsystem));
    EGO_CHECK_RETURN_FALSE(registerPluginDirectory(_initData.m_pluginDirectory));

    m_applicationProfiler = new ApplicationProfiler();
    EGO_CHECK_RETURN_FALSE(m_applicationProfiler);
    EGO_CHECK_RETURN_FALSE(m_applicationProfiler->init(_initData.m_profilerPluginModuleName));

    m_eventSubsystem = new EventSubsystem();
    EGO_CHECK_RETURN_FALSE(m_eventSubsystem);
    EGO_CHECK_RETURN_FALSE(m_eventSubsystem->init());
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_eventSubsystem));

    if (_initData.m_enableGraphicHardware)
    {
        m_graphicHardwareSubsystem = new gpu::GraphicHardwareSubsystem();
        EGO_CHECK_RETURN_FALSE(m_graphicHardwareSubsystem);

        gpu::GraphicHardwareSubsystem::InitData graphicHardwareSubsystemInitData;
        graphicHardwareSubsystemInitData.m_pluginModuleName = _initData.m_graphicHardwarePluginModuleName;
        EGO_CHECK_RETURN_FALSE(m_graphicHardwareSubsystem->init(graphicHardwareSubsystemInitData));
        EGO_CHECK_RETURN_FALSE(registerSubsystem(m_graphicHardwareSubsystem));
    }

    const PlatformPointer platform = GetPlatformPointer();
    const FileSystemPointer resourceFileSystem = platform ? platform->getFileSystem() : nullptr;
    EGO_CHECK_RETURN_FALSE(resourceFileSystem);

    m_resourceSubsystem = new ResourceSubsystem();
    EGO_CHECK_RETURN_FALSE(m_resourceSubsystem);

    ResourceSubsystem::InitData resourceSubsystemInitData;
    resourceSubsystemInitData.m_resourceFileSystem = resourceFileSystem;
    EGO_CHECK_RETURN_FALSE(m_resourceSubsystem->init(resourceSubsystemInitData));
    EGO_CHECK_RETURN_FALSE(registerSubsystem(m_resourceSubsystem));
    EGO_CHECK_RETURN_FALSE(registerGraphicResourceProvider());

    return true;
}

void ego::application::Application::releaseSubsystems()
{
    releaseSubsystem(m_resourceSubsystem);
    m_resourceSubsystem = nullptr;

    releaseSubsystem(m_graphicHardwareSubsystem);
    m_graphicHardwareSubsystem = nullptr;

    releaseSubsystem(m_eventSubsystem);
    m_eventSubsystem = nullptr;

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_applicationProfiler);

    releaseSubsystem(m_pluginSubsystem);
    m_pluginSubsystem = nullptr;

    releaseSubsystem(m_platformPluginSubsystem);
    m_platformPluginSubsystem = nullptr;

    releaseSubsystem(m_platformSubsystem);
    m_platformSubsystem = nullptr;

    releaseSubsystem(m_diagnosticSubsystem);
    m_diagnosticSubsystem = nullptr;

    releaseSubsystemRegistry();
}

bool ego::application::Application::initSubsystemRegistry()
{
    EGO_CHECK_RETURN_FALSE(!m_subsystemRegistry);

    m_subsystemRegistry = new subsystem::SubsystemRegistry();
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

void ego::application::Application::releaseSubsystem(const subsystem::SubsystemPointer& _subsystem)
{
    if (!_subsystem)
    {
        return;
    }
    
    _subsystem->release();

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
    if (!_pluginDirectory)
    {
        return true;
    }

    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN_FALSE(platform);

    const FileSystemPointer fileSystem = platform->getFileSystem();
    EGO_CHECK_RETURN_FALSE(fileSystem);

    EGO_CHECK_RETURN_FALSE(m_pluginSubsystem);

    PluginCatalogBuilder::AddPluginsFromPath(m_pluginSubsystem->getPluginCatalog(), *fileSystem, _pluginDirectory);

    return true;
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
    EGO_CHECK_RETURN_FALSE(!m_windowController);
    EGO_CHECK_RETURN_FALSE(m_applicationQuitRequestedEventCallbackID == InvalidEventCallbackID);

    const PlatformPointer platform = GetPlatformPointer();
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    const EventControllerPointer eventController = eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(platform);
    EGO_CHECK_RETURN_FALSE(eventController);

    m_windowController = new ApplicationWindowController();
    EGO_CHECK_RETURN_FALSE(m_windowController);
    EGO_CHECK_RETURN_FALSE(m_windowController->init(platform, eventController));

    m_applicationQuitRequestedEventCallbackID = eventController->addEventCallback<ApplicationQuitRequestedEvent>(
        [this](const ApplicationQuitRequestedEvent&)
        {
            requestExit();
        });

    return m_applicationQuitRequestedEventCallbackID != InvalidEventCallbackID;
}

void ego::application::Application::releaseWindowing()
{
    if (m_applicationQuitRequestedEventCallbackID != InvalidEventCallbackID)
    {
        const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
        const EventControllerPointer eventController = eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
        if (eventController)
        {
            eventController->removeEventCallback(m_applicationQuitRequestedEventCallbackID);
        }

        m_applicationQuitRequestedEventCallbackID = InvalidEventCallbackID;
    }

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_windowController);
}
