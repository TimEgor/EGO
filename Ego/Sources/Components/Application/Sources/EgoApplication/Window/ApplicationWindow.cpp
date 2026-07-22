#include "ApplicationWindow.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Platform/Window/Window.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoApplication/Presentation/PresentationSurfaceEvents.h"

ego::InstancedEventID ego::application::ApplicationWindow::getDestroyingEventID() const
{
    return m_destroyingEventID;
}

ego::InstancedEventID ego::application::ApplicationWindow::getActivationEventID() const
{
    return m_activationEventID;
}

ego::InstancedEventID ego::application::ApplicationWindow::getSizeChangedEventID() const
{
    return m_sizeChangedEventID;
}

ego::InstancedEventID ego::application::ApplicationWindow::getKeyboardInputEventID() const
{
    return m_keyboardInputEventID;
}

ego::InstancedEventID ego::application::ApplicationWindow::getTextInputEventID() const
{
    return m_textInputEventID;
}

ego::application::ApplicationWindow::~ApplicationWindow()
{
    release();
}

bool ego::application::ApplicationWindow::init(const WindowPointer& _nativeWindow, const EventControllerPointer& _eventController)
{
    EGO_CHECK_RETURN_FALSE(_nativeWindow);
    EGO_CHECK_RETURN_FALSE(_eventController);
    EGO_CHECK_RETURN_FALSE(!m_nativeWindow);
    EGO_CHECK_RETURN_FALSE(!m_eventController);

    m_nativeWindow = _nativeWindow;
    m_eventController = _eventController;

    EGO_CHECK_INITIALIZATION(initInstancedEvents());

    return true;
}

void ego::application::ApplicationWindow::release()
{
    releaseInstancedEvents();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_nativeWindow);
    m_eventController = nullptr;
}

void ego::application::ApplicationWindow::detachNativeWindow()
{
    releaseInstancedEvents();
    m_nativeWindow = nullptr;
    m_eventController = nullptr;
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

bool ego::application::ApplicationWindow::screenToLocal(const PresentationSurfacePoint& _screenPoint, PresentationSurfacePoint& _localPoint) const
{
    return m_nativeWindow && m_nativeWindow->screenToClient(_screenPoint, _localPoint);
}

const ego::WindowSize& ego::application::ApplicationWindow::getWindowSize() const
{
    return m_nativeWindow ? m_nativeWindow->getWindowSize() : DefaultWindowSize;
}

const ego::WindowSize& ego::application::ApplicationWindow::getSize() const
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
    EGO_CHECK_RETURN_FALSE(m_eventController);

    m_destroyingEventID = m_eventController->registerInstancedEvent<PresentationSurfaceDestroyingEvent>();
    EGO_CHECK_RETURN_FALSE(m_destroyingEventID != InvalidInstancedEventID);

    m_activationEventID = m_eventController->registerInstancedEvent<PresentationSurfaceActivationEvent>();
    EGO_CHECK_RETURN_FALSE(m_activationEventID != InvalidInstancedEventID);

    m_sizeChangedEventID = m_eventController->registerInstancedEvent<PresentationSurfaceSizeChangedEvent>();
    EGO_CHECK_RETURN_FALSE(m_sizeChangedEventID != InvalidInstancedEventID);

    m_keyboardInputEventID = m_eventController->registerInstancedEvent<PresentationSurfaceKeyboardInputEvent>();
    EGO_CHECK_RETURN_FALSE(m_keyboardInputEventID != InvalidInstancedEventID);

    m_textInputEventID = m_eventController->registerInstancedEvent<PresentationSurfaceTextInputEvent>();
    EGO_CHECK_RETURN_FALSE(m_textInputEventID != InvalidInstancedEventID);

    return true;
}

void ego::application::ApplicationWindow::releaseInstancedEvents()
{
    if (m_eventController)
    {
        if (m_textInputEventID != InvalidInstancedEventID)
        {
            m_eventController->unregisterInstancedEvent(m_textInputEventID);
        }

        if (m_keyboardInputEventID != InvalidInstancedEventID)
        {
            m_eventController->unregisterInstancedEvent(m_keyboardInputEventID);
        }

        if (m_sizeChangedEventID != InvalidInstancedEventID)
        {
            m_eventController->unregisterInstancedEvent(m_sizeChangedEventID);
        }

        if (m_activationEventID != InvalidInstancedEventID)
        {
            m_eventController->unregisterInstancedEvent(m_activationEventID);
        }

        if (m_destroyingEventID != InvalidInstancedEventID)
        {
            m_eventController->unregisterInstancedEvent(m_destroyingEventID);
        }
    }

    m_textInputEventID = InvalidInstancedEventID;
    m_keyboardInputEventID = InvalidInstancedEventID;
    m_sizeChangedEventID = InvalidInstancedEventID;
    m_activationEventID = InvalidInstancedEventID;
    m_destroyingEventID = InvalidInstancedEventID;
}
