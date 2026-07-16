#include "GuiContainer.h"

#include <algorithm>

bool ego::gui::GuiContainer::addChild(const GuiWidgetPointer& _widget)
{
    if (!_widget)
    {
        return false;
    }

    const ChildCollection::const_iterator childIt = std::find_if(
        m_children.begin(),
        m_children.end(),
        [&_widget](const GuiWidgetPointer& _child)
        {
            return _child.get() == _widget.get();
        });
    if (childIt != m_children.end())
    {
        return false;
    }

    m_children.push_back(_widget);
    return true;
}

void ego::gui::GuiContainer::clearChildren()
{
    GuiInputEvent focusLostEvent;
    focusLostEvent.m_type = GuiInputEventType::FocusLost;
    processFocusLostEvent(focusLostEvent);
    m_children.clear();
}

void ego::gui::GuiContainer::clearHoveredChild(const GuiPosition& _position)
{
    updateHoveredChild(nullptr, _position);
}

const ego::gui::GuiContainer::ChildCollection& ego::gui::GuiContainer::getChildren() const
{
    return m_children;
}

ego::gui::GuiEventResult ego::gui::GuiContainer::onEvent(const GuiInputEvent& _event)
{
    switch (_event.m_type)
    {
    case GuiInputEventType::FocusLost:
        return processFocusLostEvent(_event);

    case GuiInputEventType::PointerLeave:
        updateHoveredChild(nullptr, _event.m_position);
        return GuiEventResult::Unhandled;

    case GuiInputEventType::MouseMove:
    case GuiInputEventType::MouseButtonDown:
    case GuiInputEventType::MouseButtonUp:
    case GuiInputEventType::MouseWheel:
        return processPointerEvent(_event);

    case GuiInputEventType::KeyDown:
    case GuiInputEventType::KeyUp:
    case GuiInputEventType::TextInput:
        return dispatchToFocusedChild(_event);

    case GuiInputEventType::Undefined:
    default:
        return GuiEventResult::Unhandled;
    }
}

void ego::gui::GuiContainer::onPaint(GuiPaintContext& _context) const
{
    for (const GuiWidgetPointer& child : m_children)
    {
        if (child)
        {
            child->paint(_context);
        }
    }
}

bool ego::gui::GuiContainer::isChildHitTestVisible(const GuiPosition& _position) const
{
    return getRect().contains(_position);
}

ego::gui::GuiEventResult ego::gui::GuiContainer::processFocusLostEvent(const GuiInputEvent& _event)
{
    for (const GuiWidgetPointer& child : m_children)
    {
        if (child)
        {
            child->handleEvent(_event);
        }
    }

    m_focusedChild.reset();
    m_hoveredChild.reset();
    m_capturedChild.reset();
    return GuiEventResult::Unhandled;
}

ego::gui::GuiEventResult ego::gui::GuiContainer::processPointerEvent(const GuiInputEvent& _event)
{
    const GuiWidgetPointer hoveredChild = findChildAtPosition(_event.m_position);

    switch (_event.m_type)
    {
    case GuiInputEventType::MouseButtonDown:
    {
        setFocusedChild(hoveredChild);
        updateHoveredChild(hoveredChild, _event.m_position);

        const GuiEventResult result = hoveredChild ? hoveredChild->handleEvent(_event) : GuiEventResult::Unhandled;
        if (result == GuiEventResult::Handled)
        {
            m_capturedChild = hoveredChild;
            m_capturedMouseButton = _event.m_mouseButton;
        }

        return result;
    }

    case GuiInputEventType::MouseMove:
    {
        updateHoveredChild(hoveredChild, _event.m_position);
        const GuiWidgetPointer capturedChild = m_capturedChild.lock();
        const GuiWidgetPointer targetChild = capturedChild ? capturedChild : hoveredChild;
        return targetChild ? targetChild->handleEvent(_event) : GuiEventResult::Unhandled;
    }

    case GuiInputEventType::MouseButtonUp:
    {
        updateHoveredChild(hoveredChild, _event.m_position);
        const GuiWidgetPointer capturedChild = m_capturedChild.lock();
        const GuiWidgetPointer targetChild = capturedChild ? capturedChild : hoveredChild;
        const GuiEventResult result = targetChild ? targetChild->handleEvent(_event) : GuiEventResult::Unhandled;
        if (capturedChild && _event.m_mouseButton == m_capturedMouseButton)
        {
            m_capturedChild.reset();
        }

        return result;
    }

    case GuiInputEventType::MouseWheel:
        return hoveredChild ? hoveredChild->handleEvent(_event) : GuiEventResult::Unhandled;

    default:
        return GuiEventResult::Unhandled;
    }
}

ego::gui::GuiEventResult ego::gui::GuiContainer::dispatchToFocusedChild(const GuiInputEvent& _event)
{
    const GuiWidgetPointer focusedChild = m_focusedChild.lock();
    return focusedChild ? focusedChild->handleEvent(_event) : GuiEventResult::Unhandled;
}

ego::gui::GuiWidgetPointer ego::gui::GuiContainer::findChildAtPosition(const GuiPosition& _position) const
{
    if (!isChildHitTestVisible(_position))
    {
        return nullptr;
    }

    for (ChildCollection::const_reverse_iterator childIt = m_children.rbegin(); childIt != m_children.rend(); ++childIt)
    {
        const GuiWidgetPointer& child = *childIt;
        if (child && child->isVisible() && child->getRect().contains(_position))
        {
            return child;
        }
    }

    return nullptr;
}

void ego::gui::GuiContainer::setFocusedChild(const GuiWidgetPointer& _widget)
{
    const GuiWidgetPointer focusedChild = m_focusedChild.lock();
    if (focusedChild.get() == _widget.get())
    {
        return;
    }

    if (focusedChild)
    {
        GuiInputEvent focusLostEvent;
        focusLostEvent.m_type = GuiInputEventType::FocusLost;
        focusedChild->handleEvent(focusLostEvent);
    }

    m_focusedChild = _widget;
}

void ego::gui::GuiContainer::updateHoveredChild(const GuiWidgetPointer& _widget, const GuiPosition& _position)
{
    const GuiWidgetPointer hoveredChild = m_hoveredChild.lock();
    if (hoveredChild.get() == _widget.get())
    {
        return;
    }

    if (hoveredChild)
    {
        GuiInputEvent pointerLeaveEvent;
        pointerLeaveEvent.m_type = GuiInputEventType::PointerLeave;
        pointerLeaveEvent.m_position = _position;
        hoveredChild->handleEvent(pointerLeaveEvent);
    }

    m_hoveredChild = _widget;
}
