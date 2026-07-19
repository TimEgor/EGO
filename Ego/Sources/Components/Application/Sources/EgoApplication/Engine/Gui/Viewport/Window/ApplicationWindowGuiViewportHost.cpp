#include "ApplicationWindowGuiViewportHost.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoApplication/Window/WindowGraphicPresenter.h"

#include "ApplicationWindowGuiViewportEventSource.h"

ego::application::ApplicationWindowGuiViewportHost::~ApplicationWindowGuiViewportHost()
{
    release();
}

bool ego::application::ApplicationWindowGuiViewportHost::init(const ApplicationWindowPointer& _window, gui::ViewportRole _role)
{
    EGO_CHECK_INITIALIZATION(!m_window);
    EGO_CHECK_INITIALIZATION(!m_graphicPresenter);
    EGO_CHECK_INITIALIZATION(!m_guiEventSource);
    EGO_CHECK_INITIALIZATION(_window && _window->isValid());

    m_state = State::Active;

    m_window = _window;
    m_role = _role;

    EGO_CHECK_INITIALIZATION(initGraphicPresenter());
    EGO_CHECK_INITIALIZATION(initGuiEventSource());
    update();

    return true;
}

void ego::application::ApplicationWindowGuiViewportHost::release()
{
    m_state = State::Closing;

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_guiEventSource);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_graphicPresenter);

    if (m_window && m_role == gui::ViewportRole::Secondary)
    {
        m_window->release();
    }

    m_window = nullptr;
    m_size = gui::SizeZero;
    m_presenterSize = DefaultWindowSize;
}

void ego::application::ApplicationWindowGuiViewportHost::update()
{
    if (m_state != State::Active)
    {
        return;
    }

    if (!m_window || !m_window->isValid())
    {
        m_state = State::Closing;
        return;
    }

    const WindowSize& windowSize = m_window->getSize();
    m_size = gui::Size(static_cast<float>(windowSize.m_x), static_cast<float>(windowSize.m_y));
}

void ego::application::ApplicationWindowGuiViewportHost::beginClosing()
{
    m_state = State::Closing;
}

ego::engine::EngineViewportPrepareResult ego::application::ApplicationWindowGuiViewportHost::prepareForRender()
{
    update();

    if (m_state == State::Failed)
    {
        return engine::EngineViewportPrepareResult::Failed;
    }

    if (m_state != State::Active || !m_window || !m_window->isStable())
    {
        return engine::EngineViewportPrepareResult::Unavailable;
    }

    const WindowSize& windowSize = m_window->getSize();
    if (windowSize.m_x == 0 || windowSize.m_y == 0)
    {
        return engine::EngineViewportPrepareResult::Unavailable;
    }

    const bool presenterSizeChanged = windowSize.m_x != m_presenterSize.m_x || windowSize.m_y != m_presenterSize.m_y;
    return presenterSizeChanged ? engine::EngineViewportPrepareResult::TargetResizeRequired : engine::EngineViewportPrepareResult::Ready;
}

bool ego::application::ApplicationWindowGuiViewportHost::resizeRenderTarget()
{
    if (m_state != State::Active || !m_window || !m_window->isStable() || !m_graphicPresenter)
    {
        return false;
    }

    const WindowSize& windowSize = m_window->getSize();
    if (windowSize.m_x == 0 || windowSize.m_y == 0)
    {
        return false;
    }

    const gpu::Texture2DSize targetSize(windowSize.m_x, windowSize.m_y);
    if (!m_graphicPresenter->resize(targetSize))
    {
        m_state = State::Failed;
        return false;
    }

    m_presenterSize = windowSize;
    return true;
}

ego::GraphicPresenterPointer ego::application::ApplicationWindowGuiViewportHost::getGraphicPresenterPointer() const
{
    return m_graphicPresenter;
}

ego::gui::ViewportUpdateStatus ego::application::ApplicationWindowGuiViewportHost::getUpdateStatus() const
{
    switch (m_state)
    {
    case State::Active:
        return gui::ViewportUpdateStatus::Alive;
    case State::Closing:
        return gui::ViewportUpdateStatus::CloseRequested;
    case State::Failed:
    default:
        return gui::ViewportUpdateStatus::Lost;
    }
}

const ego::gui::Size& ego::application::ApplicationWindowGuiViewportHost::getSize() const
{
    return m_size;
}

void ego::application::ApplicationWindowGuiViewportHost::drainInput(gui::InputEventCollection& _input)
{
    if (m_guiEventSource)
    {
        m_guiEventSource->drainInput(_input);
    }
}

bool ego::application::ApplicationWindowGuiViewportHost::initGraphicPresenter()
{
    const gpu::GraphicHardwareSubsystemPointer graphicHardwareSubsystem = gpu::GetGraphicHardwareSubsystemPointer();
    EGO_CHECK_RETURN_FALSE(graphicHardwareSubsystem && graphicHardwareSubsystem->getGraphicDevicePointer());

    m_graphicPresenter = new WindowGraphicPresenter();
    EGO_CHECK_RETURN_FALSE(m_graphicPresenter);

    gpu::SwapChainDesc swapChainDesc;
    swapChainDesc.m_format = gpu::GraphicResourceFormat::R8G8B8A8UNorm;
    swapChainDesc.m_bufferCount = 2;

    return m_graphicPresenter->init(graphicHardwareSubsystem->getGraphicDevice(), *m_window, swapChainDesc, graphicHardwareSubsystem->getGraphicCommandQueue());
}

bool ego::application::ApplicationWindowGuiViewportHost::initGuiEventSource()
{
    m_guiEventSource = new ApplicationWindowGuiViewportEventSource();
    return m_guiEventSource && m_guiEventSource->init(m_window);
}
