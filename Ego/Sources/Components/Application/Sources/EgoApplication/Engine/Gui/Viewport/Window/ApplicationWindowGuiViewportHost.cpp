#include "ApplicationWindowGuiViewportHost.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoApplication/Window/WindowGraphicPresenter.h"

#include "ApplicationWindowGuiViewportEventSource.h"

ego::application::ApplicationWindowGuiViewportHost::~ApplicationWindowGuiViewportHost()
{
    release();
}

bool ego::application::ApplicationWindowGuiViewportHost::init(const ApplicationWindowPointer& _window, gui::GuiViewportRole _role)
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

    if (m_window && m_role == gui::GuiViewportRole::Secondary)
    {
        m_window->release();
    }

    m_window = nullptr;
    m_size = gui::GuiSizeZero;
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
    m_size = gui::GuiSize(static_cast<float>(windowSize.m_x), static_cast<float>(windowSize.m_y));
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
    if (!presenterSizeChanged)
    {
        return engine::EngineViewportPrepareResult::Ready;
    }

    if (!m_graphicPresenter)
    {
        m_state = State::Failed;
        return engine::EngineViewportPrepareResult::Failed;
    }

    const gpu::Texture2DSize targetSize(windowSize.m_x, windowSize.m_y);
    if (!m_graphicPresenter->resize(targetSize))
    {
        m_state = State::Failed;
        return engine::EngineViewportPrepareResult::Failed;
    }

    m_presenterSize = windowSize;
    return engine::EngineViewportPrepareResult::Ready;
}

ego::GraphicPresenterPointer ego::application::ApplicationWindowGuiViewportHost::getGraphicPresenterPointer() const
{
    return m_graphicPresenter;
}

bool ego::application::ApplicationWindowGuiViewportHost::isCloseRequested() const
{
    return m_state != State::Active;
}

const ego::gui::GuiSize& ego::application::ApplicationWindowGuiViewportHost::getSize() const
{
    return m_size;
}

void ego::application::ApplicationWindowGuiViewportHost::drainEvents(gui::GuiViewportEventCollection& _events)
{
    if (m_guiEventSource)
    {
        m_guiEventSource->drainEvents(_events);
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
