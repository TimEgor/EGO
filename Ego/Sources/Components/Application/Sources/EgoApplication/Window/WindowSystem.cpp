#include "WindowSystem.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Context/PlatformContext.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoRuntime/RuntimeContext.h"

#include "WindowEvents.h"

namespace
{
    ego::EventController& GetCurrentEventController()
    {
        return ego::context::GetRuntimeContext().getEventController();
    }
} // namespace

bool ego::WindowSystem::init()
{
    EGO_CHECK_RETURN_FALSE(!m_platformWindowSystem);

    EventController& eventController = GetCurrentEventController();

    EGO_CHECK_RETURN_FALSE(eventController.registerEvent<WindowSystemQuitRequestedEvent>());
    if (!eventController.registerEvent<WindowDestroyingEvent>())
    {
        eventController.unregisterEvent<WindowSystemQuitRequestedEvent>();
        return false;
    }

    if (!eventController.registerEvent<WindowActivationEvent>())
    {
        eventController.unregisterEvent<WindowDestroyingEvent>();
        eventController.unregisterEvent<WindowSystemQuitRequestedEvent>();
        return false;
    }

    m_platformWindowSystem = context::GetPlatform().createWindowSystem();
    EGO_CHECK_INITIALIZATION(m_platformWindowSystem);
    setupPlatformWindowSystemHandlers();
    EGO_CHECK_INITIALIZATION(m_platformWindowSystem->init());

    return true;
}

void ego::WindowSystem::release()
{
    EventController& eventController = GetCurrentEventController();

    for (const WindowPointer& window : m_windows)
    {
        if (window)
        {
            window->release();
        }
    }

    m_windows.clear();

    resetPlatformWindowSystemHandlers();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_platformWindowSystem);

    eventController.unregisterEvent<WindowActivationEvent>();
    eventController.unregisterEvent<WindowDestroyingEvent>();
    eventController.unregisterEvent<WindowSystemQuitRequestedEvent>();
}

ego::WindowPointer ego::WindowSystem::createWindow(const WindowDesc& _desc)
{
    EGO_ASSERT(m_platformWindowSystem);
    EGO_CHECK_RETURN_NULL(m_platformWindowSystem);

    PlatformWindowPointer platformWindow = m_platformWindowSystem->createWindow(CreatePlatformWindowDesc(_desc));
    EGO_CHECK_RETURN_NULL(platformWindow);

    WindowPointer window = new Window();
    if (!window || !window->init(platformWindow))
    {
        EGO_SAFE_RESET_POINTER_WITH_RELEASING(platformWindow);
        return nullptr;
    }

    m_windows.push_back(window);
    return window;
}

void ego::WindowSystem::processEvents()
{
    EGO_CHECK_RETURN(m_platformWindowSystem);
    m_platformWindowSystem->processEvents();
}

ego::PlatformWindowDesc ego::WindowSystem::CreatePlatformWindowDesc(const WindowDesc& _desc)
{
    PlatformWindowDesc platformWindowDesc;
    platformWindowDesc.m_title = _desc.m_title;
    platformWindowDesc.m_size.m_x = _desc.m_size.m_x;
    platformWindowDesc.m_size.m_y = _desc.m_size.m_y;
    platformWindowDesc.m_showOnInit = _desc.m_showOnInit;

    return platformWindowDesc;
}

ego::WindowSize ego::WindowSystem::CreateWindowSize(const PlatformWindowSize& _size)
{
    return WindowSize(_size.m_x, _size.m_y);
}

ego::WindowPointer ego::WindowSystem::findWindow(const PlatformWindowPointer& _platformWindow) const
{
    if (!_platformWindow)
    {
        return nullptr;
    }

    for (const WindowPointer& window : m_windows)
    {
        if (window && window->getPlatformWindowPointer().get() == _platformWindow.get())
        {
            return window;
        }
    }

    return nullptr;
}

void ego::WindowSystem::setupPlatformWindowSystemHandlers()
{
    EGO_ASSERT(m_platformWindowSystem);
    EGO_CHECK_RETURN(m_platformWindowSystem);

    m_platformWindowSystem->setQuitRequestedHandler(
        [this]()
        {
            onPlatformQuitRequested();
        });
    m_platformWindowSystem->setWindowDestroyingHandler(
        [this](const PlatformWindowPointer& _platformWindow)
        {
            onPlatformWindowDestroying(_platformWindow);
        });
    m_platformWindowSystem->setWindowActivationHandler(
        [this](const PlatformWindowPointer& _platformWindow, bool _isActive)
        {
            onPlatformWindowActivate(_platformWindow, _isActive);
        });
    m_platformWindowSystem->setWindowSizeChangeHandler(
        [this](const PlatformWindowPointer& _platformWindow, const PlatformWindowSize& _prevSize)
        {
            onPlatformWindowSizeChange(_platformWindow, _prevSize);
        });
}

void ego::WindowSystem::resetPlatformWindowSystemHandlers()
{
    if (!m_platformWindowSystem)
    {
        return;
    }

    m_platformWindowSystem->setWindowSizeChangeHandler(nullptr);
    m_platformWindowSystem->setWindowActivationHandler(nullptr);
    m_platformWindowSystem->setWindowDestroyingHandler(nullptr);
    m_platformWindowSystem->setQuitRequestedHandler(nullptr);
}

void ego::WindowSystem::onPlatformQuitRequested()
{
    emitQuitRequested();
}

void ego::WindowSystem::onPlatformWindowDestroying(const PlatformWindowPointer& _platformWindow)
{
    const WindowPointer window = findWindow(_platformWindow);
    EGO_CHECK_RETURN(window);

    emitWindowDestroying(window);
}

void ego::WindowSystem::onPlatformWindowActivate(const PlatformWindowPointer& _platformWindow, bool _isActive)
{
    const WindowPointer window = findWindow(_platformWindow);
    EGO_CHECK_RETURN(window);

    emitWindowActivate(window, _isActive);
}

void ego::WindowSystem::onPlatformWindowSizeChange(const PlatformWindowPointer& _platformWindow, const PlatformWindowSize& _prevSize)
{
    const WindowPointer window = findWindow(_platformWindow);
    EGO_CHECK_RETURN(window);

    emitWindowSizeChange(window, CreateWindowSize(_prevSize));
}

void ego::WindowSystem::emitQuitRequested() const
{
    const WindowSystemQuitRequestedEvent windowSystemEvent;
    GetCurrentEventController().emitEvent(windowSystemEvent);
}

void ego::WindowSystem::emitWindowDestroying(const WindowPointer& _window) const
{
    const WindowDestroyingEvent windowEvent(_window);
    GetCurrentEventController().emitEvent(windowEvent);
}

void ego::WindowSystem::emitWindowActivate(const WindowPointer& _window, bool _isActive) const
{
    const WindowActivationEvent windowEvent(_window, _isActive);
    GetCurrentEventController().emitEvent(windowEvent);
}

void ego::WindowSystem::emitWindowSizeChange(const WindowPointer& _window, const WindowSize& _prevSize) const
{
    const WindowSizeEvent windowEvent(_window, _prevSize);
    GetCurrentEventController().emitInstancedEvent(_window->getSizeEventID(), windowEvent);
}
