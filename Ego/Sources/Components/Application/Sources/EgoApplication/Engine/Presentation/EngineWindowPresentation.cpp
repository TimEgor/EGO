#include "EngineWindowPresentation.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoEvent/EventSubsystem.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoApplication/Window/ApplicationWindow.h"
#include "EgoApplication/Window/ApplicationWindowEvents.h"
#include "EgoApplication/Window/WindowGraphicPresenter.h"

ego::application::EngineWindowPresentation::~EngineWindowPresentation()
{
    release();
}

bool ego::application::EngineWindowPresentation::init(const ApplicationWindowPointer& _window)
{
    EGO_CHECK_INITIALIZATION(!m_window);
    EGO_CHECK_INITIALIZATION(!m_graphicPresenter);
    EGO_CHECK_INITIALIZATION(_window && _window->isValid());

    const gpu::GraphicHardwareSubsystemPointer graphicHardwareSubsystem = gpu::GetGraphicHardwareSubsystemPointer();
    EGO_CHECK_INITIALIZATION(graphicHardwareSubsystem && graphicHardwareSubsystem->getGraphicDevicePointer());

    m_window = _window;

    m_graphicPresenter = new WindowGraphicPresenter();
    EGO_CHECK_INITIALIZATION(m_graphicPresenter);

    gpu::SwapChainDesc swapChainDesc;
    swapChainDesc.m_format = gpu::GraphicResourceFormat::R8G8B8A8UNorm;
    swapChainDesc.m_bufferCount = 2;
    EGO_CHECK_INITIALIZATION(m_graphicPresenter->init(graphicHardwareSubsystem->getGraphicDevice(), *m_window, swapChainDesc, graphicHardwareSubsystem->getGraphicCommandQueue()));

    EGO_CHECK_INITIALIZATION(registerWindowEvents());

    return true;
}

void ego::application::EngineWindowPresentation::release()
{
    unregisterWindowEvents();

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_graphicPresenter);
    m_window = nullptr;
    m_isResizePending = false;
}

bool ego::application::EngineWindowPresentation::update()
{
    if (!m_isResizePending)
    {
        return true;
    }

    EGO_CHECK_RETURN_FALSE(m_window && m_window->isValid());
    if (!m_window->isStable())
    {
        return true;
    }

    const WindowSize& clientAreaSize = m_window->getSize();
    if (clientAreaSize.m_x == 0 || clientAreaSize.m_y == 0)
    {
        return true;
    }

    EGO_CHECK_RETURN_FALSE(m_graphicPresenter);

    const gpu::Texture2DSize targetSize(clientAreaSize.m_x, clientAreaSize.m_y);
    EGO_CHECK_RETURN_FALSE(m_graphicPresenter->resize(targetSize));

    m_isResizePending = false;
    return true;
}

ego::GraphicPresenterPointer ego::application::EngineWindowPresentation::getGraphicPresenterPointer() const
{
    return m_graphicPresenter;
}

bool ego::application::EngineWindowPresentation::registerWindowEvents()
{
    EGO_CHECK_RETURN_FALSE(m_window);
    EGO_CHECK_RETURN_FALSE(m_sizeChangedCallbackID == InvalidInstancedEventCallbackID);

    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    const EventControllerPointer eventController = eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(eventController);

    m_sizeChangedCallbackID = eventController->addInstanceEventCallback<ApplicationWindowSizeChangedEvent>(
        m_window->getSizeEventID(),
        [this](const ApplicationWindowSizeChangedEvent& _event)
        {
            handleWindowSizeChanged(_event);
        });

    return m_sizeChangedCallbackID != InvalidInstancedEventCallbackID;
}

void ego::application::EngineWindowPresentation::unregisterWindowEvents()
{
    if (m_sizeChangedCallbackID == InvalidInstancedEventCallbackID)
    {
        return;
    }

    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();
    const EventControllerPointer eventController = eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
    if (eventController)
    {
        eventController->removeInstancedEventDispatcher(m_sizeChangedCallbackID);
    }

    m_sizeChangedCallbackID = InvalidInstancedEventCallbackID;
}

void ego::application::EngineWindowPresentation::handleWindowSizeChanged(const ApplicationWindowSizeChangedEvent& _event)
{
    if (m_window && _event.m_window.get() == m_window.get())
    {
        m_isResizePending = true;
    }
}
