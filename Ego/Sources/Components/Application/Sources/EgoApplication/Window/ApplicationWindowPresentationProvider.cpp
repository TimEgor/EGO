#include "ApplicationWindowPresentationProvider.h"

#include <cstdint>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEvent/EventSubsystem.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoApplication/Presentation/PresentationSurfaceEvents.h"

#include "ApplicationWindowEvents.h"

ego::application::ApplicationWindowPresentationProvider::~ApplicationWindowPresentationProvider()
{
    release();
}

bool ego::application::ApplicationWindowPresentationProvider::init(const InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(!m_isInitialized && m_presentations.empty() && m_windows.empty());

    const PlatformPointer platform = GetPlatformPointer();
    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_INITIALIZATION(platform && eventController);

    m_swapChainDesc = _initData.m_swapChainDesc;

    EGO_CHECK_RETURN_CALL_FALSE(registerApplicationEvents(), release());
    EGO_CHECK_RETURN_CALL_FALSE(registerWindowSystemEventListener(), release());

    m_isInitialized = true;
    return true;
}

void ego::application::ApplicationWindowPresentationProvider::release()
{
    unregisterWindowSystemEventListener();
    releasePresentations();
    releaseApplicationWindows();
    unregisterApplicationEvents();

    m_swapChainDesc = gpu::SwapChainDesc();
    m_isInitialized = false;
}

ego::application::Presentation ego::application::ApplicationWindowPresentationProvider::createPresentation(const PresentationDesc& _desc)
{
    Presentation presentation;
    if (!m_isInitialized)
    {
        return presentation;
    }

    WindowDesc windowDesc;
    windowDesc.m_title = _desc.m_name.c_str();
    windowDesc.m_size = _desc.m_size;
    windowDesc.m_showOnInit = _desc.m_isVisible;

    const ApplicationWindowPointer window = createApplicationWindow(windowDesc);
    if (!window)
    {
        return presentation;
    }

    InstancedEventCallbackID sizeChangedCallbackID = InvalidInstancedEventCallbackID;
    const WindowGraphicPresenterPointer graphicPresenter = createSurfaceGraphicPresenter(window, sizeChangedCallbackID);
    if (!graphicPresenter)
    {
        removeApplicationWindow(window);
        window->release();
        return presentation;
    }

    PresentationEntry entry;
    entry.m_surface = window;
    entry.m_graphicPresenter = graphicPresenter;
    entry.m_sizeChangedCallbackID = sizeChangedCallbackID;
    m_presentations.push_back(entry);

    presentation.m_surface = window;
    presentation.m_graphicPresenter = graphicPresenter;
    presentation.m_surfaceEventIDs.m_destroying = window->getDestroyingEventID();
    presentation.m_surfaceEventIDs.m_activation = window->getActivationEventID();
    presentation.m_surfaceEventIDs.m_sizeChanged = window->getSizeChangedEventID();
    presentation.m_surfaceEventIDs.m_keyboardInput = window->getKeyboardInputEventID();
    presentation.m_surfaceEventIDs.m_textInput = window->getTextInputEventID();
    return presentation;
}

bool ego::application::ApplicationWindowPresentationProvider::destroyPresentation(const PresentationSurfacePointer& _surface)
{
    if (!_surface)
    {
        return false;
    }

    ApplicationWindowPointer window = nullptr;
    if (rtti::IsObjectBasedOn<ApplicationWindow>(*_surface))
    {
        window = StaticPointerCast<ApplicationWindow>(_surface);
    }

    if (!removePresentation(_surface))
    {
        return false;
    }

    if (window)
    {
        emitPresentationSurfaceDestroying(window);
        removeApplicationWindow(window);
        window->release();
    }

    return true;
}

ego::application::ApplicationWindowPointer ego::application::ApplicationWindowPresentationProvider::createApplicationWindow(const WindowDesc& _desc)
{
    EGO_ASSERT(m_isInitialized);
    EGO_CHECK_RETURN_NULL(m_isInitialized);

    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN_NULL(eventController);

    WindowPointer nativeWindow = getNativeWindowSystem().createWindow(_desc);
    EGO_CHECK_RETURN_NULL(nativeWindow);

    ApplicationWindowPointer window = new ApplicationWindow();
    if (!window)
    {
        EGO_SAFE_RESET_POINTER_WITH_RELEASING(nativeWindow);
        return nullptr;
    }

    EGO_CHECK_RETURN_NULL(window->init(nativeWindow, eventController));

    m_windows.push_back(window);
    return window;
}

ego::WindowSystem& ego::application::ApplicationWindowPresentationProvider::getNativeWindowSystem() const
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_ASSERT(platform);

    return platform->getWindowSystem();
}

bool ego::application::ApplicationWindowPresentationProvider::registerApplicationEvents()
{
    EGO_CHECK_RETURN_FALSE(!m_areApplicationEventsRegistered);

    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN_FALSE(eventController);

    EGO_CHECK_RETURN_FALSE(eventController->registerEvent<ApplicationQuitRequestedEvent>());

    m_areApplicationEventsRegistered = true;
    return true;
}

void ego::application::ApplicationWindowPresentationProvider::unregisterApplicationEvents()
{
    if (!m_areApplicationEventsRegistered)
    {
        return;
    }

    const EventControllerPointer eventController = GetEventControllerPointer();
    if (eventController)
    {
        eventController->unregisterEvent<ApplicationQuitRequestedEvent>();
    }

    m_areApplicationEventsRegistered = false;
}

bool ego::application::ApplicationWindowPresentationProvider::registerWindowSystemEventListener()
{
    EGO_CHECK_RETURN_FALSE(!m_isWindowSystemEventListenerRegistered);

    const WindowSystemEventListenerPointer listener = sharedFromThis();
    EGO_CHECK_RETURN_FALSE(getNativeWindowSystem().registerEventListener(listener));

    m_isWindowSystemEventListenerRegistered = true;
    return true;
}

void ego::application::ApplicationWindowPresentationProvider::unregisterWindowSystemEventListener()
{
    if (!m_isWindowSystemEventListenerRegistered)
    {
        return;
    }

    const PlatformPointer platform = GetPlatformPointer();
    if (platform)
    {
        const WindowSystemEventListenerPointer listener = sharedFromThis();
        platform->getWindowSystem().unregisterEventListener(listener);
    }

    m_isWindowSystemEventListenerRegistered = false;
}

void ego::application::ApplicationWindowPresentationProvider::releasePresentations()
{
    while (!m_presentations.empty())
    {
        const PresentationSurfacePointer surface = m_presentations.back().m_surface;
        if (!destroyPresentation(surface))
        {
            PresentationEntry presentation = m_presentations.back();
            m_presentations.pop_back();
            releasePresentation(presentation);
        }
    }
}

void ego::application::ApplicationWindowPresentationProvider::releasePresentation(PresentationEntry& _presentation)
{
    const EventControllerPointer eventController = GetEventControllerPointer();
    if (eventController && _presentation.m_sizeChangedCallbackID != InvalidInstancedEventCallbackID)
    {
        eventController->removeInstancedEventDispatcher(_presentation.m_sizeChangedCallbackID);
    }
    _presentation.m_sizeChangedCallbackID = InvalidInstancedEventCallbackID;

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(_presentation.m_graphicPresenter);
    _presentation.m_surface = nullptr;
}

bool ego::application::ApplicationWindowPresentationProvider::removePresentation(const PresentationSurfacePointer& _surface)
{
    if (!_surface)
    {
        return false;
    }

    for (PresentationCollection::iterator presentationIt = m_presentations.begin(); presentationIt != m_presentations.end(); ++presentationIt)
    {
        if (presentationIt->m_surface.get() != _surface.get())
        {
            continue;
        }

        releasePresentation(*presentationIt);
        m_presentations.erase(presentationIt);
        return true;
    }

    return false;
}

ego::application::WindowGraphicPresenterPointer ego::application::ApplicationWindowPresentationProvider::createSurfaceGraphicPresenter(
    const PresentationSurfacePointer& _surface,
    InstancedEventCallbackID& _sizeChangedCallbackID)
{
    EGO_CHECK_RETURN_NULL(_surface);
    EGO_CHECK_RETURN_NULL(_sizeChangedCallbackID == InvalidInstancedEventCallbackID);

    const gpu::GraphicHardwareSubsystemPointer graphicHardwareSubsystem = gpu::GetGraphicHardwareSubsystemPointer();
    const GraphicDevicePointer graphicDevice = graphicHardwareSubsystem ? graphicHardwareSubsystem->getGraphicDevicePointer() : nullptr;
    const gpu::CommandQueueReference presentationQueue = graphicHardwareSubsystem ? graphicHardwareSubsystem->getGraphicCommandQueue() : nullptr;
    EGO_CHECK_RETURN_NULL(graphicDevice && presentationQueue);

    const WindowGraphicPresenterPointer existingGraphicPresenter = findSurfaceGraphicPresenter(_surface);
    EGO_CHECK_RETURN_NULL(!existingGraphicPresenter);

    WindowGraphicPresenterPointer graphicPresenter = new WindowGraphicPresenter();
    if (!graphicPresenter || !graphicPresenter->init(*graphicDevice, *_surface, m_swapChainDesc, presentationQueue))
    {
        EGO_SAFE_RESET_POINTER_WITH_RELEASING(graphicPresenter);
        return nullptr;
    }

    if (rtti::IsObjectBasedOn<ApplicationWindow>(*_surface))
    {
        const EventControllerPointer eventController = GetEventControllerPointer();
        EGO_CHECK_RETURN_CALL_NULL(eventController, graphicPresenter->release());

        const ApplicationWindowPointer window = StaticPointerCast<ApplicationWindow>(_surface);
        _sizeChangedCallbackID = eventController->addInstanceEventCallback<PresentationSurfaceSizeChangedEvent>(
            window->getSizeChangedEventID(),
            *this,
            &ApplicationWindowPresentationProvider::handlePresentationSurfaceSizeChanged);
        if (_sizeChangedCallbackID == InvalidInstancedEventCallbackID)
        {
            graphicPresenter->release();
            return nullptr;
        }
    }

    return graphicPresenter;
}

ego::GraphicPresenterPointer ego::application::ApplicationWindowPresentationProvider::findGraphicPresenter(const PresentationSurfacePointer& _surface) const
{
    return findSurfaceGraphicPresenter(_surface);
}

ego::application::WindowGraphicPresenterPointer ego::application::ApplicationWindowPresentationProvider::findSurfaceGraphicPresenter(
    const PresentationSurfacePointer& _surface) const
{
    if (!_surface)
    {
        return nullptr;
    }

    for (const PresentationEntry& presentation : m_presentations)
    {
        if (presentation.m_surface.get() == _surface.get())
        {
            return presentation.m_graphicPresenter;
        }
    }

    return nullptr;
}

void ego::application::ApplicationWindowPresentationProvider::releaseApplicationWindows()
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

ego::application::ApplicationWindowPointer ego::application::ApplicationWindowPresentationProvider::findApplicationWindow(
    const WindowPointer& _nativeWindow) const
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

void ego::application::ApplicationWindowPresentationProvider::removeApplicationWindow(const ApplicationWindowPointer& _window)
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

ego::EventControllerPointer ego::application::ApplicationWindowPresentationProvider::GetEventControllerPointer()
{
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    return eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
}

void ego::application::ApplicationWindowPresentationProvider::processEvents()
{
    if (m_isInitialized)
    {
        getNativeWindowSystem().processEvents();
    }
}

void ego::application::ApplicationWindowPresentationProvider::onWindowSystemQuitRequested()
{
    emitApplicationQuitRequested();
}

void ego::application::ApplicationWindowPresentationProvider::onWindowDestroying(const WindowPointer& _nativeWindow)
{
    const ApplicationWindowPointer window = findApplicationWindow(_nativeWindow);
    EGO_CHECK_RETURN(window);

    removePresentation(window);
    emitPresentationSurfaceDestroying(window);
    window->detachNativeWindow();
    removeApplicationWindow(window);
}

void ego::application::ApplicationWindowPresentationProvider::onWindowActivation(const WindowPointer& _nativeWindow, bool _isActive)
{
    const ApplicationWindowPointer window = findApplicationWindow(_nativeWindow);
    EGO_CHECK_RETURN(window);

    emitPresentationSurfaceActivation(window, _isActive);
}

void ego::application::ApplicationWindowPresentationProvider::onWindowSizeChanged(const WindowPointer& _nativeWindow, const WindowSize& _prevSize)
{
    const ApplicationWindowPointer window = findApplicationWindow(_nativeWindow);
    EGO_CHECK_RETURN(window);

    emitPresentationSurfaceSizeChanged(window, _prevSize);
}

void ego::application::ApplicationWindowPresentationProvider::onWindowKeyboardInput(
    const WindowPointer& _nativeWindow,
    const WindowKeyboardInputData& _inputData)
{
    const ApplicationWindowPointer window = findApplicationWindow(_nativeWindow);
    EGO_CHECK_RETURN(window);

    emitPresentationSurfaceKeyboardInput(window, _inputData);
}

void ego::application::ApplicationWindowPresentationProvider::onWindowTextInput(const WindowPointer& _nativeWindow, const WindowTextInputData& _inputData)
{
    const ApplicationWindowPointer window = findApplicationWindow(_nativeWindow);
    EGO_CHECK_RETURN(window);

    emitPresentationSurfaceTextInput(window, _inputData);
}

void ego::application::ApplicationWindowPresentationProvider::emitApplicationQuitRequested() const
{
    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN(eventController);

    const ApplicationQuitRequestedEvent event;
    eventController->emitEvent(event);
}

void ego::application::ApplicationWindowPresentationProvider::emitPresentationSurfaceDestroying(const ApplicationWindowPointer& _window) const
{
    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN(eventController && _window);

    const PresentationSurfaceDestroyingEvent event(_window);
    eventController->emitInstancedEvent(_window->getDestroyingEventID(), event);
}

void ego::application::ApplicationWindowPresentationProvider::emitPresentationSurfaceActivation(
    const ApplicationWindowPointer& _window,
    bool _isActive) const
{
    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN(eventController && _window);

    const PresentationSurfaceActivationEvent event(_window, _isActive);
    eventController->emitInstancedEvent(_window->getActivationEventID(), event);
}

void ego::application::ApplicationWindowPresentationProvider::emitPresentationSurfaceSizeChanged(
    const ApplicationWindowPointer& _window,
    const WindowSize& _previousSize) const
{
    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN(eventController);

    const PresentationSurfaceSizeChangedEvent event(_window, _previousSize);
    eventController->emitInstancedEvent(_window->getSizeChangedEventID(), event);
}

void ego::application::ApplicationWindowPresentationProvider::emitPresentationSurfaceKeyboardInput(
    const ApplicationWindowPointer& _window,
    const WindowKeyboardInputData& _inputData) const
{
    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN(eventController);

    const PresentationSurfaceKeyboardInputEvent event(_window, _inputData.m_key, _inputData.m_isRepeat, _inputData.m_action);
    eventController->emitInstancedEvent(_window->getKeyboardInputEventID(), event);
}

void ego::application::ApplicationWindowPresentationProvider::emitPresentationSurfaceTextInput(
    const ApplicationWindowPointer& _window,
    const WindowTextInputData& _inputData) const
{
    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN(eventController);

    const PresentationSurfaceTextInputEvent event(_window, _inputData.m_codepoint);
    eventController->emitInstancedEvent(_window->getTextInputEventID(), event);
}

void ego::application::ApplicationWindowPresentationProvider::handlePresentationSurfaceSizeChanged(const PresentationSurfaceSizeChangedEvent& _event)
{
    const WindowGraphicPresenterPointer graphicPresenter = findSurfaceGraphicPresenter(_event.m_surface);
    if (!graphicPresenter || !_event.m_surface)
    {
        return;
    }

    const PresentationSurfaceSize& surfaceSize = _event.m_surface->getSize();
    if (surfaceSize.m_x == 0 || surfaceSize.m_y == 0)
    {
        return;
    }

    const gpu::Texture2DSize targetSize(static_cast<uint32_t>(surfaceSize.m_x), static_cast<uint32_t>(surfaceSize.m_y));
    const bool resizeResult = graphicPresenter->resize(targetSize);
    EGO_ASSERT_MESSAGE(resizeResult, "Failed to request a window graphic presenter resize.");
}
