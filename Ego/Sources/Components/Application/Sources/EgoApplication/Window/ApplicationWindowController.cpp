#include "ApplicationWindowController.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/Window/WindowSystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEvent/EventController.h"

#include "ApplicationWindowEvents.h"

bool ego::application::ApplicationWindowController::init(const PlatformPointer& _platform, const EventControllerPointer& _eventController)
{
    EGO_CHECK_RETURN_FALSE(!m_isInitialized);
    EGO_CHECK_RETURN_FALSE(!m_platform);
    EGO_CHECK_RETURN_FALSE(!m_eventController);
    EGO_CHECK_RETURN_FALSE(_platform);
    EGO_CHECK_RETURN_FALSE(_eventController);

    m_platform = _platform;
    m_eventController = _eventController;

    EGO_CHECK_INITIALIZATION(registerApplicationWindowEvents());
    EGO_CHECK_INITIALIZATION(registerWindowSystemEventListener());
    m_isInitialized = true;

    return true;
}

void ego::application::ApplicationWindowController::release()
{
    unregisterWindowSystemEventListener();
    releaseApplicationWindows();
    unregisterApplicationWindowEvents();

    m_eventController = nullptr;
    m_platform = nullptr;
    m_isInitialized = false;
}

ego::application::ApplicationWindowPointer ego::application::ApplicationWindowController::createApplicationWindow(const WindowDesc& _desc)
{
    EGO_ASSERT(m_isInitialized);
    EGO_CHECK_RETURN_NULL(m_isInitialized);

    ego::WindowPointer nativeWindow = getNativeWindowSystem().createWindow(_desc);
    EGO_CHECK_RETURN_NULL(nativeWindow);

    ApplicationWindowPointer window = new ApplicationWindow();
    if (!window)
    {
        EGO_SAFE_RESET_POINTER_WITH_RELEASING(nativeWindow);
        return nullptr;
    }

    EGO_CHECK_RETURN_NULL(window->init(nativeWindow, m_eventController));

    m_windows.push_back(window);

    return window;
}

bool ego::application::ApplicationWindowController::containsApplicationWindow(const ApplicationWindowPointer& _window) const
{
    if (!_window)
    {
        return false;
    }

    for (const ApplicationWindowPointer& window : m_windows)
    {
        if (window.get() == _window.get())
        {
            return true;
        }
    }

    return false;
}

void ego::application::ApplicationWindowController::processWindowEvents()
{
    EGO_CHECK_RETURN(m_isInitialized);
    getNativeWindowSystem().processEvents();
}

ego::WindowSystem& ego::application::ApplicationWindowController::getNativeWindowSystem() const
{
    EGO_ASSERT(m_platform);
    return m_platform->getWindowSystem();
}

bool ego::application::ApplicationWindowController::registerWindowSystemEventListener()
{
    const WindowSystemEventListenerPointer listener = sharedFromThis();
    EGO_CHECK_RETURN_FALSE(getNativeWindowSystem().registerEventListener(listener));

    m_isWindowSystemEventListenerRegistered = true;
    return true;
}

void ego::application::ApplicationWindowController::unregisterWindowSystemEventListener()
{
    if (!m_isWindowSystemEventListenerRegistered)
    {
        return;
    }

    const WindowSystemEventListenerPointer listener = sharedFromThis();
    getNativeWindowSystem().unregisterEventListener(listener);
    m_isWindowSystemEventListenerRegistered = false;
}

bool ego::application::ApplicationWindowController::registerApplicationWindowEvents()
{
    EGO_CHECK_RETURN_FALSE(m_eventController);

    EGO_CHECK_RETURN_FALSE(m_eventController->registerEvent<ApplicationQuitRequestedEvent>());
    EGO_CHECK_RETURN_FALSE(m_eventController->registerEvent<ApplicationWindowDestroyingEvent>());
    EGO_CHECK_RETURN_FALSE(m_eventController->registerEvent<ApplicationWindowActivationEvent>());
    EGO_CHECK_RETURN_FALSE(m_eventController->registerEvent<ApplicationWindowKeyboardInputEvent>());
    EGO_CHECK_RETURN_FALSE(m_eventController->registerEvent<ApplicationWindowTextInputEvent>());

    return true;
}

void ego::application::ApplicationWindowController::unregisterApplicationWindowEvents()
{
    if (!m_eventController)
    {
        return;
    }

    m_eventController->unregisterEvent<ApplicationWindowTextInputEvent>();
    m_eventController->unregisterEvent<ApplicationWindowKeyboardInputEvent>();
    m_eventController->unregisterEvent<ApplicationWindowActivationEvent>();
    m_eventController->unregisterEvent<ApplicationWindowDestroyingEvent>();
    m_eventController->unregisterEvent<ApplicationQuitRequestedEvent>();
}

void ego::application::ApplicationWindowController::releaseApplicationWindows()
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

ego::application::ApplicationWindowPointer ego::application::ApplicationWindowController::findApplicationWindow(const ego::WindowPointer& _nativeWindow) const
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

void ego::application::ApplicationWindowController::removeApplicationWindow(const ApplicationWindowPointer& _window)
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

void ego::application::ApplicationWindowController::onWindowSystemQuitRequested()
{
    emitApplicationQuitRequested();
}

void ego::application::ApplicationWindowController::onWindowDestroying(const ego::WindowPointer& _nativeWindow)
{
    const ApplicationWindowPointer window = findApplicationWindow(_nativeWindow);
    EGO_CHECK_RETURN(window);

    emitApplicationWindowDestroying(window);
    window->detachNativeWindow();
    removeApplicationWindow(window);
}

void ego::application::ApplicationWindowController::onWindowActivation(const ego::WindowPointer& _nativeWindow, bool _isActive)
{
    const ApplicationWindowPointer window = findApplicationWindow(_nativeWindow);
    EGO_CHECK_RETURN(window);

    emitApplicationWindowActivation(window, _isActive);
}

void ego::application::ApplicationWindowController::onWindowSizeChanged(const ego::WindowPointer& _nativeWindow, const WindowSize& _prevSize)
{
    const ApplicationWindowPointer window = findApplicationWindow(_nativeWindow);
    EGO_CHECK_RETURN(window);

    emitApplicationWindowSizeChanged(window, _prevSize);
}

void ego::application::ApplicationWindowController::onWindowKeyboardInput(const ego::WindowPointer& _nativeWindow, const WindowKeyboardInputData& _inputData)
{
    const ApplicationWindowPointer window = findApplicationWindow(_nativeWindow);
    EGO_CHECK_RETURN(window);

    emitApplicationWindowKeyboardInput(window, _inputData);
}

void ego::application::ApplicationWindowController::onWindowTextInput(const ego::WindowPointer& _nativeWindow, const WindowTextInputData& _inputData)
{
    const ApplicationWindowPointer window = findApplicationWindow(_nativeWindow);
    EGO_CHECK_RETURN(window);

    emitApplicationWindowTextInput(window, _inputData);
}

void ego::application::ApplicationWindowController::emitApplicationQuitRequested() const
{
    EGO_CHECK_RETURN(m_eventController);

    const ApplicationQuitRequestedEvent event;
    m_eventController->emitEvent(event);
}

void ego::application::ApplicationWindowController::emitApplicationWindowDestroying(const ApplicationWindowPointer& _window) const
{
    EGO_CHECK_RETURN(m_eventController);

    const ApplicationWindowDestroyingEvent event(_window);
    m_eventController->emitEvent(event);
}

void ego::application::ApplicationWindowController::emitApplicationWindowActivation(const ApplicationWindowPointer& _window, bool _isActive) const
{
    EGO_CHECK_RETURN(m_eventController);

    const ApplicationWindowActivationEvent event(_window, _isActive);
    m_eventController->emitEvent(event);
}

void ego::application::ApplicationWindowController::emitApplicationWindowSizeChanged(const ApplicationWindowPointer& _window, const WindowSize& _prevSize) const
{
    EGO_CHECK_RETURN(m_eventController);

    const ApplicationWindowSizeChangedEvent event(_window, _prevSize);
    m_eventController->emitInstancedEvent(_window->getSizeEventID(), event);
}

void ego::application::ApplicationWindowController::emitApplicationWindowKeyboardInput(const ApplicationWindowPointer& _window, const WindowKeyboardInputData& _inputData) const
{
    EGO_CHECK_RETURN(m_eventController);

    const ApplicationWindowKeyboardInputEvent event(_window, _inputData);
    m_eventController->emitEvent(event);
    m_eventController->emitInstancedEvent(_window->getKeyboardInputEventID(), event);
}

void ego::application::ApplicationWindowController::emitApplicationWindowTextInput(const ApplicationWindowPointer& _window, const WindowTextInputData& _inputData) const
{
    EGO_CHECK_RETURN(m_eventController);

    const ApplicationWindowTextInputEvent event(_window, _inputData);
    m_eventController->emitEvent(event);
    m_eventController->emitInstancedEvent(_window->getTextInputEventID(), event);
}
