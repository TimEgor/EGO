#include "GuiViewport.h"

#include <algorithm>

ego::gui::GuiViewport::GuiViewport(GuiViewportID _id, GuiViewportRole _role, const GuiSize& _size)
    : m_id(_id),
      m_role(_role),
      m_size(_size)
{
}

ego::gui::GuiViewportID ego::gui::GuiViewport::getID() const
{
    return m_id;
}

ego::gui::GuiViewportRole ego::gui::GuiViewport::getRole() const
{
    return m_role;
}

const ego::gui::GuiSize& ego::gui::GuiViewport::getSize() const
{
    return m_size;
}

void ego::gui::GuiViewport::setSize(const GuiSize& _size)
{
    m_size = _size;
}

bool ego::gui::GuiViewport::addWindow(const GuiWindowPointer& _window)
{
    if (!_window)
    {
        return false;
    }

    const WindowCollection::const_iterator windowIt = std::find_if(
        m_windows.begin(),
        m_windows.end(),
        [&_window](const GuiWindowPointer& _currentWindow)
        {
            return _currentWindow.get() == _window.get();
        });
    if (windowIt != m_windows.end())
    {
        return false;
    }

    m_windows.push_back(_window);
    setActiveWindow(_window);
    return true;
}

ego::gui::GuiWindowPointer ego::gui::GuiViewport::removeWindow(const GuiWindowPointer& _window)
{
    if (!_window)
    {
        return nullptr;
    }

    const WindowCollection::iterator windowIt = std::find_if(
        m_windows.begin(),
        m_windows.end(),
        [&_window](const GuiWindowPointer& _currentWindow)
        {
            return _currentWindow.get() == _window.get();
        });
    if (windowIt == m_windows.end())
    {
        return nullptr;
    }

    GuiInputEvent focusLostEvent;
    focusLostEvent.m_type = GuiInputEventType::FocusLost;
    (*windowIt)->handleEvent(focusLostEvent);

    const GuiWindowPointer removedWindow = *windowIt;
    m_windows.erase(windowIt);

    const GuiWindowPointer activeWindow = m_activeWindow.lock();
    if (activeWindow.get() == removedWindow.get())
    {
        if (m_windows.empty())
        {
            m_activeWindow.reset();
        }
        else
        {
            m_activeWindow = m_windows.back();
        }
    }

    const GuiWindowPointer hoveredWindow = m_hoveredWindow.lock();
    if (hoveredWindow.get() == removedWindow.get())
    {
        m_hoveredWindow.reset();
    }

    const GuiWindowPointer capturedWindow = m_capturedWindow.lock();
    if (capturedWindow.get() == removedWindow.get())
    {
        m_capturedWindow.reset();
    }

    return removedWindow;
}

void ego::gui::GuiViewport::clearWindows()
{
    GuiInputEvent focusLostEvent;
    focusLostEvent.m_type = GuiInputEventType::FocusLost;
    for (const GuiWindowPointer& window : m_windows)
    {
        if (window)
        {
            window->handleEvent(focusLostEvent);
        }
    }

    m_windows.clear();
    m_activeWindow.reset();
    m_hoveredWindow.reset();
    m_capturedWindow.reset();
}

const ego::gui::GuiViewport::WindowCollection& ego::gui::GuiViewport::getWindows() const
{
    return m_windows;
}

ego::gui::GuiWindowPointer ego::gui::GuiViewport::getActiveWindow() const
{
    return m_activeWindow.lock();
}

ego::gui::GuiEventResult ego::gui::GuiViewport::processEvent(const GuiInputEvent& _event)
{
    switch (_event.m_type)
    {
    case GuiInputEventType::FocusLost:
        return processFocusLostEvent(_event);

    case GuiInputEventType::PointerLeave:
        updateHoveredWindow(nullptr, _event.m_position);
        return GuiEventResult::Unhandled;

    case GuiInputEventType::MouseMove:
    case GuiInputEventType::MouseButtonDown:
    case GuiInputEventType::MouseButtonUp:
    case GuiInputEventType::MouseWheel:
        return processPointerEvent(_event);

    case GuiInputEventType::KeyDown:
    case GuiInputEventType::KeyUp:
    case GuiInputEventType::TextInput:
        return dispatchToActiveWindow(_event);

    case GuiInputEventType::Undefined:
    default:
        return GuiEventResult::Unhandled;
    }
}

ego::gui::GuiEventResult ego::gui::GuiViewport::processFocusLostEvent(const GuiInputEvent& _event)
{
    for (const GuiWindowPointer& window : m_windows)
    {
        if (window)
        {
            window->handleEvent(_event);
        }
    }

    m_activeWindow.reset();
    m_hoveredWindow.reset();
    m_capturedWindow.reset();

    return GuiEventResult::Unhandled;
}

ego::gui::GuiEventResult ego::gui::GuiViewport::processPointerEvent(const GuiInputEvent& _event)
{
    const bool isInside = containsPosition(_event.m_position);
    const GuiWindowPointer hoveredWindow = isInside ? findWindowAtPosition(_event.m_position) : nullptr;
    updateHoveredWindow(hoveredWindow, _event.m_position);

    if (!isInside && _event.m_type != GuiInputEventType::MouseButtonUp)
    {
        return GuiEventResult::Unhandled;
    }

    const GuiWindowPointer capturedWindow = m_capturedWindow.lock();
    const GuiWindowPointer interactionWindow = capturedWindow ? capturedWindow : hoveredWindow;

    switch (_event.m_type)
    {
    case GuiInputEventType::MouseButtonDown:
    {
        setActiveWindow(hoveredWindow);
        if (!hoveredWindow)
        {
            return GuiEventResult::Unhandled;
        }

        const GuiEventResult result = hoveredWindow->handleEvent(_event);
        if (result == GuiEventResult::Handled)
        {
            m_capturedWindow = hoveredWindow;
            m_capturedMouseButton = _event.m_mouseButton;
        }

        return result;
    }

    case GuiInputEventType::MouseMove:
        return interactionWindow ? interactionWindow->handleEvent(_event) : GuiEventResult::Unhandled;

    case GuiInputEventType::MouseButtonUp:
    {
        const GuiEventResult result = interactionWindow ? interactionWindow->handleEvent(_event) : GuiEventResult::Unhandled;
        if (capturedWindow && _event.m_mouseButton == m_capturedMouseButton)
        {
            m_capturedWindow.reset();
        }

        return result;
    }

    case GuiInputEventType::MouseWheel:
        return hoveredWindow ? hoveredWindow->handleEvent(_event) : GuiEventResult::Unhandled;

    default:
        return GuiEventResult::Unhandled;
    }
}

ego::gui::GuiEventResult ego::gui::GuiViewport::dispatchToActiveWindow(const GuiInputEvent& _event)
{
    const GuiWindowPointer activeWindow = m_activeWindow.lock();
    return activeWindow ? activeWindow->handleEvent(_event) : GuiEventResult::Unhandled;
}

void ego::gui::GuiViewport::buildDrawData(const GuiLayoutContext& _layoutContext, GuiPaintContext& _paintContext)
{
    updateWindowLayouts(_layoutContext);
    paintWindows(_paintContext);
}

void ego::gui::GuiViewport::updateWindowLayouts(const GuiLayoutContext& _context)
{
    for (const GuiWindowPointer& window : m_windows)
    {
        if (!window)
        {
            continue;
        }

        window->measure(_context, window->getSize());
        window->arrange(_context, window->getWindowRect());
    }
}

void ego::gui::GuiViewport::paintWindows(GuiPaintContext& _context) const
{
    for (const GuiWindowPointer& window : m_windows)
    {
        if (window)
        {
            window->paint(_context);
        }
    }
}

bool ego::gui::GuiViewport::containsPosition(const GuiPosition& _position) const
{
    return _position.m_x >= 0.0f && _position.m_x < m_size.m_x && _position.m_y >= 0.0f && _position.m_y < m_size.m_y;
}

ego::gui::GuiWindowPointer ego::gui::GuiViewport::findWindowAtPosition(const GuiPosition& _position) const
{
    for (WindowCollection::const_reverse_iterator windowIt = m_windows.rbegin(); windowIt != m_windows.rend(); ++windowIt)
    {
        const GuiWindowPointer& window = *windowIt;
        if (window && window->isVisible() && window->getWindowRect().contains(_position))
        {
            return window;
        }
    }

    return nullptr;
}

void ego::gui::GuiViewport::setActiveWindow(const GuiWindowPointer& _window)
{
    const GuiWindowPointer activeWindow = m_activeWindow.lock();
    if (activeWindow.get() == _window.get())
    {
        bringWindowToFront(_window);
        return;
    }

    if (activeWindow)
    {
        GuiInputEvent focusLostEvent;
        focusLostEvent.m_type = GuiInputEventType::FocusLost;
        activeWindow->handleEvent(focusLostEvent);
    }

    m_activeWindow = _window;
    bringWindowToFront(_window);
}

void ego::gui::GuiViewport::updateHoveredWindow(const GuiWindowPointer& _window, const GuiPosition& _position)
{
    const GuiWindowPointer hoveredWindow = m_hoveredWindow.lock();
    if (hoveredWindow.get() == _window.get())
    {
        return;
    }

    if (hoveredWindow)
    {
        GuiInputEvent pointerLeaveEvent;
        pointerLeaveEvent.m_type = GuiInputEventType::PointerLeave;
        pointerLeaveEvent.m_position = _position;
        hoveredWindow->handleEvent(pointerLeaveEvent);
    }

    m_hoveredWindow = _window;
}

void ego::gui::GuiViewport::bringWindowToFront(const GuiWindowPointer& _window)
{
    if (!_window || (!m_windows.empty() && m_windows.back().get() == _window.get()))
    {
        return;
    }

    const WindowCollection::iterator windowIt = std::find_if(
        m_windows.begin(),
        m_windows.end(),
        [&_window](const GuiWindowPointer& _currentWindow)
        {
            return _currentWindow.get() == _window.get();
        });
    if (windowIt == m_windows.end())
    {
        return;
    }

    const GuiWindowPointer window = *windowIt;
    m_windows.erase(windowIt);
    m_windows.push_back(window);
}
