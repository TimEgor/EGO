#include "Window.h"

#include "EgoCore/Assert/AssertCore.h"
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

ego::InstancedEventID ego::Window::getSizeEventID() const
{
    return m_sizeEventID;
}

ego::Window::~Window()
{
    release();
}

bool ego::Window::init(const PlatformWindowPointer& _platformWindow)
{
    EGO_CHECK_RETURN_FALSE(_platformWindow);
    EGO_CHECK_RETURN_FALSE(!m_platformWindow);

    m_platformWindow = _platformWindow;
    updatePlatformState();

    EGO_CHECK_INITIALIZATION(initWindowInstancedEvents());

    return true;
}

void ego::Window::release()
{
    releaseWindowInstancedEvents();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_platformWindow);
}

bool ego::Window::isValid() const
{
    return m_platformWindow && m_platformWindow->isValid();
}

void ego::Window::show()
{
    EGO_CHECK_RETURN(m_platformWindow);
    m_platformWindow->show();
}

void ego::Window::hide()
{
    EGO_CHECK_RETURN(m_platformWindow);
    m_platformWindow->hide();
}

bool ego::Window::isShown() const
{
    return m_platformWindow && m_platformWindow->isShown();
}

void* ego::Window::getNativeHandle() const
{
    return m_platformWindow ? m_platformWindow->getNativeHandle() : nullptr;
}

bool ego::Window::isStable() const
{
    return m_platformWindow && m_platformWindow->isStable();
}

const ego::WindowSize& ego::Window::getWindowSize() const
{
    updatePlatformState();
    return m_windowSize;
}

const ego::WindowSize& ego::Window::getClientAreaSize() const
{
    updatePlatformState();
    return m_clientAreaSize;
}

const ego::WindowArea& ego::Window::getCutoutsArea() const
{
    updatePlatformState();
    return m_cutoutsArea;
}

ego::PlatformWindowPointer ego::Window::getPlatformWindowPointer() const
{
    return m_platformWindow;
}

bool ego::Window::initWindowInstancedEvents()
{
    EventController& eventController = GetCurrentEventController();

    m_sizeEventID = eventController.registerInstancedEvent<WindowSizeEvent>();
    EGO_CHECK_RETURN_FALSE(m_sizeEventID != InvalidInstancedEventID);

    return true;
}

void ego::Window::updatePlatformState() const
{
    if (!m_platformWindow)
    {
        return;
    }

    const PlatformWindowSize& platformWindowSize = m_platformWindow->getWindowSize();
    m_windowSize.m_x = platformWindowSize.m_x;
    m_windowSize.m_y = platformWindowSize.m_y;

    const PlatformWindowSize& platformClientAreaSize = m_platformWindow->getClientAreaSize();
    m_clientAreaSize.m_x = platformClientAreaSize.m_x;
    m_clientAreaSize.m_y = platformClientAreaSize.m_y;

    const PlatformWindowArea& platformCutoutsArea = m_platformWindow->getCutoutsArea();
    m_cutoutsArea.m_top = platformCutoutsArea.m_top;
    m_cutoutsArea.m_bottom = platformCutoutsArea.m_bottom;
    m_cutoutsArea.m_right = platformCutoutsArea.m_right;
    m_cutoutsArea.m_left = platformCutoutsArea.m_left;
}

void ego::Window::releaseWindowInstancedEvents()
{
    EventController& eventController = GetCurrentEventController();

    if (m_sizeEventID != InvalidInstancedEventID)
    {
        eventController.unregisterInstancedEvent(m_sizeEventID);
        m_sizeEventID = InvalidInstancedEventID;
    }
}
