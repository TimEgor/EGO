#include "Widget.h"

#include <algorithm>
#include <cmath>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Input/Input.h"
#include "EgoGui/Layout/Layout.h"
#include "EgoGui/Rendering/PaintContext.h"

ego::gui::InputReply ego::gui::Widget::WidgetAccessor::OnPointerMove(Widget& _widget, InputContext& _context, const PointerMoveEvent& _event)
{
    return _widget.onPointerMove(_context, _event);
}

ego::gui::InputReply ego::gui::Widget::WidgetAccessor::OnMouseButton(Widget& _widget, InputContext& _context, const MouseButtonEvent& _event)
{
    return _widget.onMouseButton(_context, _event);
}

ego::gui::InputReply ego::gui::Widget::WidgetAccessor::OnMouseWheel(Widget& _widget, InputContext& _context, const MouseWheelEvent& _event)
{
    return _widget.onMouseWheel(_context, _event);
}

ego::gui::InputReply ego::gui::Widget::WidgetAccessor::OnKey(Widget& _widget, InputContext& _context, const KeyEvent& _event)
{
    return _widget.onKey(_context, _event);
}

ego::gui::InputReply ego::gui::Widget::WidgetAccessor::OnTextInput(Widget& _widget, InputContext& _context, const TextInputEvent& _event)
{
    return _widget.onTextInput(_context, _event);
}

void ego::gui::Widget::WidgetAccessor::OnPointerEnter(Widget& _widget, const Position& _position, const InputModifiers& _modifiers)
{
    _widget.onPointerEnter(_position, _modifiers);
}

void ego::gui::Widget::WidgetAccessor::OnPointerLeave(Widget& _widget, const Position& _position, const InputModifiers& _modifiers)
{
    _widget.onPointerLeave(_position, _modifiers);
}

void ego::gui::Widget::WidgetAccessor::OnPointerCaptureLost(Widget& _widget, const Position& _position)
{
    _widget.onPointerCaptureLost(_position);
}

void ego::gui::Widget::WidgetAccessor::OnFocusChanged(Widget& _widget, FocusChange _change)
{
    _widget.onFocusChanged(_change);
}

ego::gui::Size ego::gui::Widget::WidgetAccessor::UpdatePreferredSize(Widget& _widget, const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    return _widget.updatePreferredSize(_context, _constraints);
}

void ego::gui::Widget::WidgetAccessor::ApplyLayout(Widget& _widget, const LayoutContext& _context, const Rect& _bounds)
{
    _widget.applyLayout(_context, _bounds);
}

bool ego::gui::Widget::WidgetAccessor::IsLayoutInvalidated(const Widget& _widget)
{
    return _widget.isLayoutInvalidated();
}

void ego::gui::Widget::WidgetAccessor::OnLayoutCompleted(Widget& _widget)
{
    _widget.onLayoutCompleted();
}

void ego::gui::Widget::WidgetAccessor::DrawBaseLayer(const Widget& _widget, PaintContext& _context)
{
    _widget.drawBaseLayer(_context);
}

void ego::gui::Widget::WidgetAccessor::DrawOverlayLayer(const Widget& _widget, PaintContext& _context)
{
    _widget.drawOverlayLayer(_context);
}

bool ego::gui::Widget::WidgetAccessor::ClipsChildren(const Widget& _widget)
{
    return _widget.clipsChildren();
}

ego::gui::Rect ego::gui::Widget::WidgetAccessor::GetChildrenClipRect(const Widget& _widget)
{
    return _widget.getChildrenClipRect();
}

size_t ego::gui::Widget::WidgetAccessor::GetChildCount(const Widget& _widget)
{
    return _widget.getChildCount();
}

ego::gui::WidgetPointer ego::gui::Widget::WidgetAccessor::GetChild(const Widget& _widget, size_t _index)
{
    return _widget.getChild(_index);
}

bool ego::gui::Widget::WidgetAccessor::IsChildActive(const Widget& _widget, size_t _index)
{
    return _widget.isChildActive(_index);
}

bool ego::gui::Widget::WidgetAccessor::HitTest(const Widget& _widget, const Position& _position)
{
    return _widget.hitTest(_position);
}

bool ego::gui::Widget::WidgetAccessor::IsChildHitTestVisible(const Widget& _widget, const Position& _position)
{
    return _widget.isChildHitTestVisible(_position);
}

ego::gui::Widget::Widget() = default;

ego::gui::Size ego::gui::Widget::updatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    if (isCollapsed())
    {
        m_preferredSize = SizeZero;

        return m_preferredSize;
    }

    const Size preferredSize = calculatePreferredSize(_context, _constraints);
    m_preferredSize = Size(
        std::isnan(preferredSize.m_x) ? 0.0f : (std::clamp)(preferredSize.m_x, 0.0f, UnboundedLayoutExtent),
        std::isnan(preferredSize.m_y) ? 0.0f : (std::clamp)(preferredSize.m_y, 0.0f, UnboundedLayoutExtent));

    return m_preferredSize;
}

void ego::gui::Widget::applyLayout(const LayoutContext& _context, const Rect& _bounds)
{
    m_isLayoutInvalidated = false;
    if (isCollapsed())
    {
        m_layoutBounds = Rect(_bounds.m_position, SizeZero);

        return;
    }

    m_layoutBounds = Rect(_bounds.m_position, Size((std::max)(0.0f, _bounds.m_size.m_x), (std::max)(0.0f, _bounds.m_size.m_y)));
    updateGeometry(_context);
}

bool ego::gui::Widget::attachChild(const WidgetPointer& _child)
{
    if (!_child || _child->m_parent.lock())
    {
        return false;
    }

    const WidgetPointer self = sharedFromThis();
    if (!self)
    {
        EGO_ASSERT_FAIL_MESSAGE("Widget must have a shared owner before attaching children.");

        return false;
    }

    WidgetPointer ancestor = self;
    while (ancestor)
    {
        if (ancestor.get() == _child.get())
        {
            return false;
        }

        ancestor = ancestor->m_parent.lock();
    }

    _child->m_parent = self;
    invalidateLayout();

    return true;
}

bool ego::gui::Widget::detachChild(const WidgetPointer& _child)
{
    if (!_child || !_child->isDirectChildOf(*this))
    {
        return false;
    }

    _child->m_parent.reset();
    invalidateLayout();

    return true;
}

void ego::gui::Widget::invalidateLayout() const
{
    m_isLayoutInvalidated = true;

    const WidgetPointer parent = m_parent.lock();
    if (parent)
    {
        parent->invalidateLayout();
    }
}

bool ego::gui::Widget::isLayoutInvalidated() const
{
    return m_isLayoutInvalidated;
}

void ego::gui::Widget::notifyTreeChanged() const
{
    invalidateLayout();
}

void ego::gui::Widget::setVisibility(Visibility _visibility)
{
    if (m_visibility == _visibility)
    {
        return;
    }

    m_visibility = _visibility;
    notifyTreeChanged();
}

ego::gui::Visibility ego::gui::Widget::getVisibility() const
{
    return m_visibility;
}

bool ego::gui::Widget::isVisible() const
{
    return m_visibility == Visibility::Visible;
}

bool ego::gui::Widget::isCollapsed() const
{
    return m_visibility == Visibility::Collapsed;
}

const ego::gui::Size& ego::gui::Widget::getPreferredSize() const
{
    return m_preferredSize;
}

const ego::gui::Rect& ego::gui::Widget::getLayoutBounds() const
{
    return m_layoutBounds;
}

ego::gui::InputReply ego::gui::Widget::onPointerMove(InputContext&, const PointerMoveEvent&)
{
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Widget::onMouseButton(InputContext&, const MouseButtonEvent&)
{
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Widget::onMouseWheel(InputContext&, const MouseWheelEvent&)
{
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Widget::onKey(InputContext&, const KeyEvent&)
{
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Widget::onTextInput(InputContext&, const TextInputEvent&)
{
    return InputReply::Unhandled;
}

void ego::gui::Widget::onPointerEnter(const Position&, const InputModifiers&) {}

void ego::gui::Widget::onPointerLeave(const Position&, const InputModifiers&) {}

void ego::gui::Widget::onPointerCaptureLost(const Position&) {}

void ego::gui::Widget::onFocusChanged(FocusChange) {}

ego::gui::Size ego::gui::Widget::calculatePreferredSize(const LayoutContext&, const LayoutConstraints&)
{
    return SizeZero;
}

void ego::gui::Widget::updateGeometry(const LayoutContext&) {}

void ego::gui::Widget::onLayoutCompleted() {}

void ego::gui::Widget::drawBaseLayer(PaintContext&) const {}

void ego::gui::Widget::drawOverlayLayer(PaintContext&) const {}

bool ego::gui::Widget::clipsChildren() const
{
    return false;
}

ego::gui::Rect ego::gui::Widget::getChildrenClipRect() const
{
    return getLayoutBounds();
}

size_t ego::gui::Widget::getChildCount() const
{
    return 0;
}

ego::gui::WidgetPointer ego::gui::Widget::getChild(size_t) const
{
    return nullptr;
}

bool ego::gui::Widget::isChildActive(size_t) const
{
    return true;
}

bool ego::gui::Widget::hitTest(const Position& _position) const
{
    return isVisible() && getLayoutBounds().contains(_position);
}

bool ego::gui::Widget::isChildHitTestVisible(const Position& _position) const
{
    return getLayoutBounds().contains(_position);
}

bool ego::gui::Widget::isDescendantOf(const Widget& _ancestor) const
{
    WidgetPointer widget = m_parent.lock();
    while (widget)
    {
        if (widget.get() == &_ancestor)
        {
            return true;
        }

        widget = widget->m_parent.lock();
    }

    return false;
}

bool ego::gui::Widget::isDirectChildOf(const Widget& _parent) const
{
    const WidgetPointer parent = m_parent.lock();

    return parent.get() == &_parent;
}

ego::gui::WidgetPointer ego::gui::Widget::getParent() const
{
    return m_parent.lock();
}
