#include "ApplicationWindow.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Platform/Window/Window.h"
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

ego::InstancedEventID ego::application::ApplicationWindow::getSizeEventID() const
{
    return m_sizeEventID;
}

ego::application::ApplicationWindow::~ApplicationWindow()
{
    release();
}

bool ego::application::ApplicationWindow::init(const ego::WindowPointer& _nativeWindow)
{
    EGO_CHECK_RETURN_FALSE(_nativeWindow);
    EGO_CHECK_RETURN_FALSE(!m_nativeWindow);

    m_nativeWindow = _nativeWindow;

    EGO_CHECK_INITIALIZATION(initInstancedEvents());

    return true;
}

void ego::application::ApplicationWindow::release()
{
    releaseInstancedEvents();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_nativeWindow);
}

void ego::application::ApplicationWindow::detachNativeWindow()
{
    releaseInstancedEvents();
    m_nativeWindow = nullptr;
}

bool ego::application::ApplicationWindow::isValid() const
{
    return m_nativeWindow && m_nativeWindow->isValid();
}

void ego::application::ApplicationWindow::show()
{
    EGO_CHECK_RETURN(m_nativeWindow);
    m_nativeWindow->show();
}

void ego::application::ApplicationWindow::hide()
{
    EGO_CHECK_RETURN(m_nativeWindow);
    m_nativeWindow->hide();
}

bool ego::application::ApplicationWindow::isShown() const
{
    return m_nativeWindow && m_nativeWindow->isShown();
}

void* ego::application::ApplicationWindow::getNativeHandle() const
{
    return m_nativeWindow ? m_nativeWindow->getNativeHandle() : nullptr;
}

bool ego::application::ApplicationWindow::isStable() const
{
    return m_nativeWindow && m_nativeWindow->isStable();
}

bool ego::application::ApplicationWindow::screenToClient(const WindowPoint& _screenPoint, WindowPoint& _clientPoint) const
{
    return m_nativeWindow && m_nativeWindow->screenToClient(_screenPoint, _clientPoint);
}

const ego::WindowSize& ego::application::ApplicationWindow::getWindowSize() const
{
    return m_nativeWindow ? m_nativeWindow->getWindowSize() : DefaultWindowSize;
}

const ego::WindowSize& ego::application::ApplicationWindow::getClientAreaSize() const
{
    return m_nativeWindow ? m_nativeWindow->getClientAreaSize() : DefaultWindowSize;
}

const ego::WindowArea& ego::application::ApplicationWindow::getCutoutsArea() const
{
    return m_nativeWindow ? m_nativeWindow->getCutoutsArea() : DefaultWindowArea;
}

ego::WindowPointer ego::application::ApplicationWindow::getNativeWindowPointer() const
{
    return m_nativeWindow;
}

bool ego::application::ApplicationWindow::initInstancedEvents()
{
    EventController& eventController = GetCurrentEventController();

    m_sizeEventID = eventController.registerInstancedEvent<ApplicationWindowSizeChangedEvent>();
    EGO_CHECK_RETURN_FALSE(m_sizeEventID != InvalidInstancedEventID);

    return true;
}

void ego::application::ApplicationWindow::releaseInstancedEvents()
{
    EventController& eventController = GetCurrentEventController();

    if (m_sizeEventID != InvalidInstancedEventID)
    {
        eventController.unregisterInstancedEvent(m_sizeEventID);
        m_sizeEventID = InvalidInstancedEventID;
    }
}
