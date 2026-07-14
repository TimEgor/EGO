#include "EngineWindowPresentation.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoApplication/Window/ApplicationWindow.h"
#include "EgoApplication/Window/ApplicationWindowEvents.h"

#include "EgoEngine/EngineSession.h"
#include "EgoEngine/Graphic/Presenter/WindowGraphicPresenter.h"

ego::engine::EngineWindowPresentation::~EngineWindowPresentation()
{
    release();
}

bool ego::engine::EngineWindowPresentation::init(
    const EngineSessionPointer& _engineSession,
    const gpu::GraphicHardwareSubsystemPointer& _graphicHardwareSubsystem,
    const EventControllerPointer& _eventController,
    const application::ApplicationWindowPointer& _window,
    bool _makePrimary)
{
    EGO_CHECK_INITIALIZATION(!m_engineSession);
    EGO_CHECK_INITIALIZATION(!m_window);
    EGO_CHECK_INITIALIZATION(!m_graphicPresenter);
    EGO_CHECK_INITIALIZATION(!m_eventController);
    EGO_CHECK_INITIALIZATION(_engineSession);
    EGO_CHECK_INITIALIZATION(_graphicHardwareSubsystem && _graphicHardwareSubsystem->getGraphicDevicePointer());
    EGO_CHECK_INITIALIZATION(_eventController);
    EGO_CHECK_INITIALIZATION(_window && _window->isValid());

    m_engineSession = _engineSession;
    m_window = _window;
    m_eventController = _eventController;

    m_graphicPresenter = new WindowGraphicPresenter();
    EGO_CHECK_INITIALIZATION(m_graphicPresenter);

    gpu::SwapChainDesc swapChainDesc;
    swapChainDesc.m_format = gpu::GraphicResourceFormat::R8G8B8A8UNorm;
    swapChainDesc.m_bufferCount = 2;
    EGO_CHECK_INITIALIZATION(
        m_graphicPresenter->init(_graphicHardwareSubsystem->getGraphicDevice(), *m_window, swapChainDesc, _graphicHardwareSubsystem->getGraphicCommandQueue()));
    EGO_CHECK_INITIALIZATION(m_engineSession->registerGraphicPresenter(m_graphicPresenter, _makePrimary));
    EGO_CHECK_INITIALIZATION(registerWindowEvents());

    return true;
}

void ego::engine::EngineWindowPresentation::release()
{
    unregisterWindowEvents();

    if (m_engineSession && m_graphicPresenter)
    {
        m_engineSession->unregisterGraphicPresenter(m_graphicPresenter);
    }

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_graphicPresenter);
    m_window = nullptr;
    m_engineSession = nullptr;
    m_isResizePending = false;
}

bool ego::engine::EngineWindowPresentation::update()
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

    const WindowSize& clientAreaSize = m_window->getClientAreaSize();
    if (clientAreaSize.m_x == 0 || clientAreaSize.m_y == 0)
    {
        return true;
    }

    EGO_CHECK_RETURN_FALSE(m_engineSession && m_graphicPresenter);

    const gpu::Texture2DSize targetSize(clientAreaSize.m_x, clientAreaSize.m_y);
    EGO_CHECK_RETURN_FALSE(m_graphicPresenter->resize(targetSize));

    m_isResizePending = false;
    return true;
}

bool ego::engine::EngineWindowPresentation::registerWindowEvents()
{
    EGO_CHECK_RETURN_FALSE(m_window);
    EGO_CHECK_RETURN_FALSE(m_eventController);
    EGO_CHECK_RETURN_FALSE(m_sizeChangedCallbackID == InvalidInstancedEventCallbackID);

    m_sizeChangedCallbackID = m_eventController->addInstanceEventCallback<application::ApplicationWindowSizeChangedEvent>(
        m_window->getSizeEventID(),
        [this](const application::ApplicationWindowSizeChangedEvent& _event)
        {
            handleWindowSizeChanged(_event);
        });

    return m_sizeChangedCallbackID != InvalidInstancedEventCallbackID;
}

void ego::engine::EngineWindowPresentation::unregisterWindowEvents()
{
    if (m_eventController && m_sizeChangedCallbackID != InvalidInstancedEventCallbackID)
    {
        m_eventController->removeInstancedEventDispatcher(m_sizeChangedCallbackID);
    }

    m_sizeChangedCallbackID = InvalidInstancedEventCallbackID;
    m_eventController = nullptr;
}

void ego::engine::EngineWindowPresentation::handleWindowSizeChanged(const application::ApplicationWindowSizeChangedEvent& _event)
{
    if (m_window && _event.m_window.get() == m_window.get())
    {
        m_isResizePending = true;
    }
}
