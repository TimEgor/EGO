#include "ApplicationWindowGuiViewportHost.h"

#include "EgoCore/UtilsMacros.h"

#include "ApplicationWindowGuiViewportEventSource.h"

ego::application::ApplicationWindowGuiViewportHost::~ApplicationWindowGuiViewportHost()
{
    release();
}

bool ego::application::ApplicationWindowGuiViewportHost::init(const ApplicationWindowPointer& _window)
{
    EGO_CHECK_INITIALIZATION(!m_window);
    EGO_CHECK_INITIALIZATION(!m_guiEventSource);
    EGO_CHECK_INITIALIZATION(_window && _window->isValid());

    m_state = State::Active;

    m_window = _window;

    EGO_CHECK_INITIALIZATION(initGuiEventSource());
    update();

    return true;
}

void ego::application::ApplicationWindowGuiViewportHost::release()
{
    m_state = State::Closing;

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_guiEventSource);

    m_window = nullptr;
    m_size = gui::SizeZero;
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

ego::gui::ViewportUpdateStatus ego::application::ApplicationWindowGuiViewportHost::getUpdateStatus() const
{
    switch (m_state)
    {
    case State::Active:
        return gui::ViewportUpdateStatus::Alive;
    case State::Closing:
    default:
        return gui::ViewportUpdateStatus::CloseRequested;
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

bool ego::application::ApplicationWindowGuiViewportHost::initGuiEventSource()
{
    m_guiEventSource = new ApplicationWindowGuiViewportEventSource();
    return m_guiEventSource && m_guiEventSource->init(m_window);
}
