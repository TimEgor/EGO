#include "Application.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/UtilsMacros.h"

ego::application::Application::~Application()
{
    release();
}

bool ego::application::Application::init(const InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(initPluginController());
    EGO_CHECK_INITIALIZATION(initEventController());
    EGO_CHECK_INITIALIZATION(initWindowSystem(_initData));

    return true;
}

void ego::application::Application::release()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_windowSystem);
    m_windowSystemPlugin = nullptr;

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_eventController);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_pluginController);
}

const ego::EventController& ego::application::Application::getEventController() const
{
    EGO_ASSERT(m_eventController);
    return *m_eventController;
}

ego::EventController& ego::application::Application::getEventController()
{
    EGO_ASSERT(m_eventController);
    return *m_eventController;
}

const ego::WindowSystem& ego::application::Application::getWindowSystem() const
{
    EGO_ASSERT(m_windowSystem);
    return *m_windowSystem;
}

ego::WindowSystem& ego::application::Application::getWindowSystem()
{
    EGO_ASSERT(m_windowSystem);
    return *m_windowSystem;
}

bool ego::application::Application::initPluginController()
{
    m_pluginController = new ApplicationPluginController();
    EGO_CHECK_RETURN_FALSE(m_pluginController && m_pluginController->init());

    return true;
}

bool ego::application::Application::initEventController()
{
    m_eventController = new EventController();
    EGO_CHECK_RETURN_FALSE(m_eventController && m_eventController->init());

    return true;
}

bool ego::application::Application::initWindowSystem(const InitData& _initData)
{
    EGO_ASSERT(m_pluginController);

    FileName windowSystemPluginModuleName = _initData.m_windowSystemPluginModuleName;
    if (!windowSystemPluginModuleName)
    {
        windowSystemPluginModuleName = m_pluginController->selectPluginModule<WindowSystemPlugin>();
    }

    EGO_CHECK_RETURN_FALSE(windowSystemPluginModuleName);

    m_windowSystemPlugin = m_pluginController->loadPlugin<WindowSystemPlugin>(windowSystemPluginModuleName);
    EGO_CHECK_RETURN_FALSE(m_windowSystemPlugin);

    m_windowSystem = m_windowSystemPlugin->createWindowSystem(_initData.m_nativeInstanceHandle);
    EGO_CHECK_RETURN_FALSE(m_windowSystem && m_windowSystem->init());

    return true;
}

bool ego::application::ApplicationCore::init(const ApplicationPointer& _application)
{
    EGO_ASSERT(_application);

    if (m_application)
    {
        EGO_ASSERT_FAIL_MESSAGE("Application has been already inited.");
        return false;
    }

    m_application = _application;

    return static_cast<bool>(m_application);
}

void ego::application::ApplicationCore::release()
{
    m_application = nullptr;
}

ego::application::ApplicationPointer ego::application::ApplicationCore::getApplication() const
{
    return m_application;
}

ego::application::Application& ego::application::GetApplication()
{
    const ApplicationPointer application = ApplicationCore::GetInstance().getApplication();

    EGO_ASSERT(application);
    return *application.get();
}
