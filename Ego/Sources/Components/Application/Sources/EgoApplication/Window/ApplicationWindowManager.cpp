#include "ApplicationWindowManager.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Context/PlatformContext.h"
#include "EgoCore/Platform/Window/WindowSystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoRuntime/RuntimeContext.h"

#include "ApplicationWindowEvents.h"

namespace
{
    ego::EventController& GetCurrentEventController()
    {
        return ego::context::GetRuntimeContext().getEventController();
    }
} // namespace

bool ego::application::ApplicationWindowManager::init()
{
    EGO_CHECK_RETURN_FALSE(!m_isInitialized);
    EGO_CHECK_RETURN_FALSE(registerApplicationWindowEvents());

    bindNativeWindowSystemHandlers();
    m_isInitialized = true;

    return true;
}

void ego::application::ApplicationWindowManager::release()
{
    if (!m_isInitialized)
    {
        return;
    }

    unbindNativeWindowSystemHandlers();
    releaseApplicationWindows();
    unregisterApplicationWindowEvents();
    m_isInitialized = false;
}

ego::application::ApplicationWindowPointer ego::application::ApplicationWindowManager::createApplicationWindow(const WindowDesc& _desc)
{
    EGO_ASSERT(m_isInitialized);
    EGO_CHECK_RETURN_NULL(m_isInitialized);

    ego::WindowPointer nativeWindow = getNativeWindowSystem().createWindow(_desc);
    EGO_CHECK_RETURN_NULL(nativeWindow);

    ApplicationWindowPointer window = new ApplicationWindow();
    if (!window || !window->init(nativeWindow))
    {
        EGO_SAFE_RESET_POINTER_WITH_RELEASING(nativeWindow);
        return nullptr;
    }

    m_windows.push_back(window);
    return window;
}

void ego::application::ApplicationWindowManager::processWindowEvents()
{
    EGO_CHECK_RETURN(m_isInitialized);
    getNativeWindowSystem().processEvents();
}

ego::WindowSystem& ego::application::ApplicationWindowManager::getNativeWindowSystem() const
{
    return context::GetPlatform().getWindowSystem();
}

bool ego::application::ApplicationWindowManager::registerApplicationWindowEvents()
{
    EventController& eventController = GetCurrentEventController();

    EGO_CHECK_RETURN_FALSE(eventController.registerEvent<ApplicationQuitRequestedEvent>());
    if (!eventController.registerEvent<ApplicationWindowDestroyingEvent>())
    {
        eventController.unregisterEvent<ApplicationQuitRequestedEvent>();
        return false;
    }

    if (!eventController.registerEvent<ApplicationWindowActivationEvent>())
    {
        eventController.unregisterEvent<ApplicationWindowDestroyingEvent>();
        eventController.unregisterEvent<ApplicationQuitRequestedEvent>();
        return false;
    }

    return true;
}

void ego::application::ApplicationWindowManager::unregisterApplicationWindowEvents()
{
    EventController& eventController = GetCurrentEventController();

    eventController.unregisterEvent<ApplicationWindowActivationEvent>();
    eventController.unregisterEvent<ApplicationWindowDestroyingEvent>();
    eventController.unregisterEvent<ApplicationQuitRequestedEvent>();
}

void ego::application::ApplicationWindowManager::bindNativeWindowSystemHandlers()
{
    ego::WindowSystem& nativeWindowSystem = getNativeWindowSystem();

    nativeWindowSystem.setQuitRequestedHandler(
        [this]()
        {
            handleNativeQuitRequested();
        });
    nativeWindowSystem.setWindowDestroyingHandler(
        [this](const ego::WindowPointer& _nativeWindow)
        {
            handleNativeWindowDestroying(_nativeWindow);
        });
    nativeWindowSystem.setWindowActivationHandler(
        [this](const ego::WindowPointer& _nativeWindow, bool _isActive)
        {
            handleNativeWindowActivation(_nativeWindow, _isActive);
        });
    nativeWindowSystem.setWindowSizeChangeHandler(
        [this](const ego::WindowPointer& _nativeWindow, const WindowSize& _prevSize)
        {
            handleNativeWindowSizeChanged(_nativeWindow, _prevSize);
        });
}

void ego::application::ApplicationWindowManager::unbindNativeWindowSystemHandlers()
{
    ego::WindowSystem& nativeWindowSystem = getNativeWindowSystem();
    nativeWindowSystem.setWindowSizeChangeHandler(nullptr);
    nativeWindowSystem.setWindowActivationHandler(nullptr);
    nativeWindowSystem.setWindowDestroyingHandler(nullptr);
    nativeWindowSystem.setQuitRequestedHandler(nullptr);
}

void ego::application::ApplicationWindowManager::releaseApplicationWindows()
{
    for (const ApplicationWindowPointer& window : m_windows)
    {
        if (window)
        {
            window->release();
        }
    }

    m_windows.clear();
}

ego::application::ApplicationWindowPointer ego::application::ApplicationWindowManager::findApplicationWindow(const ego::WindowPointer& _nativeWindow) const
{
    if (!_nativeWindow)
    {
        return nullptr;
    }

    for (const ApplicationWindowPointer& window : m_windows)
    {
        if (window && window->getNativeWindowPointer().get() == _nativeWindow.get())
        {
            return window;
        }
    }

    return nullptr;
}

void ego::application::ApplicationWindowManager::removeApplicationWindow(const ApplicationWindowPointer& _window)
{
    for (ApplicationWindowCollection::iterator windowIt = m_windows.begin(); windowIt != m_windows.end(); ++windowIt)
    {
        if (windowIt->get() == _window.get())
        {
            m_windows.erase(windowIt);
            return;
        }
    }
}

void ego::application::ApplicationWindowManager::handleNativeQuitRequested()
{
    emitApplicationQuitRequested();
}

void ego::application::ApplicationWindowManager::handleNativeWindowDestroying(const ego::WindowPointer& _nativeWindow)
{
    const ApplicationWindowPointer window = findApplicationWindow(_nativeWindow);
    EGO_CHECK_RETURN(window);

    emitApplicationWindowDestroying(window);
    window->detachNativeWindow();
    removeApplicationWindow(window);
}

void ego::application::ApplicationWindowManager::handleNativeWindowActivation(const ego::WindowPointer& _nativeWindow, bool _isActive)
{
    const ApplicationWindowPointer window = findApplicationWindow(_nativeWindow);
    EGO_CHECK_RETURN(window);

    emitApplicationWindowActivation(window, _isActive);
}

void ego::application::ApplicationWindowManager::handleNativeWindowSizeChanged(const ego::WindowPointer& _nativeWindow, const WindowSize& _prevSize)
{
    const ApplicationWindowPointer window = findApplicationWindow(_nativeWindow);
    EGO_CHECK_RETURN(window);

    emitApplicationWindowSizeChanged(window, _prevSize);
}

void ego::application::ApplicationWindowManager::emitApplicationQuitRequested() const
{
    const ApplicationQuitRequestedEvent event;
    GetCurrentEventController().emitEvent(event);
}

void ego::application::ApplicationWindowManager::emitApplicationWindowDestroying(const ApplicationWindowPointer& _window) const
{
    const ApplicationWindowDestroyingEvent event(_window);
    GetCurrentEventController().emitEvent(event);
}

void ego::application::ApplicationWindowManager::emitApplicationWindowActivation(const ApplicationWindowPointer& _window, bool _isActive) const
{
    const ApplicationWindowActivationEvent event(_window, _isActive);
    GetCurrentEventController().emitEvent(event);
}

void ego::application::ApplicationWindowManager::emitApplicationWindowSizeChanged(const ApplicationWindowPointer& _window, const WindowSize& _prevSize) const
{
    const ApplicationWindowSizeChangedEvent event(_window, _prevSize);
    GetCurrentEventController().emitInstancedEvent(_window->getSizeEventID(), event);
}
