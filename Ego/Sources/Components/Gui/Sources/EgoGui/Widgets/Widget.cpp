#include "Widget.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "EgoCore/Assert/Assert.h"

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

void ego::gui::Widget::completeLayout()
{
    const WidgetPointer root = sharedFromThis();
    if (!root)
    {
        return;
    }

    std::vector<WidgetPointer> widgets;
    widgets.push_back(root);
    for (size_t widgetIndex = 0; widgetIndex < widgets.size(); ++widgetIndex)
    {
        const WidgetPointer widget = widgets[widgetIndex];
        if (widget->isCollapsed())
        {
            continue;
        }

        const size_t childCount = widget->getChildCount();
        for (size_t childIndex = 0; childIndex < childCount; ++childIndex)
        {
            const WidgetPointer child = widget->getChild(childIndex);
            if (child && child->isDirectChildOf(*widget))
            {
                widgets.push_back(child);
            }
        }
    }

    for (const WidgetPointer& widget : widgets)
    {
        if (isLayoutInvalidated())
        {
            break;
        }

        if (!widget->isCollapsed() && (widget.get() == this || widget->isDescendantOf(*this)))
        {
            widget->onLayoutCompleted();
        }
    }
}

void ego::gui::Widget::emitDrawCommands(PaintContext& _context) const
{
    if (!isVisible() || m_layoutBounds.m_size.m_x <= 0.0f || m_layoutBounds.m_size.m_y <= 0.0f)
    {
        return;
    }

    drawBaseLayer(_context);

    const bool clipChildren = clipsChildren();
    if (clipChildren)
    {
        _context.pushClipRect(getChildrenClipRect());
    }

    const size_t childCount = getChildCount();
    for (size_t childIndex = 0; childIndex < childCount; ++childIndex)
    {
        const WidgetPointer child = getChild(childIndex);
        if (child && isChildActive(childIndex) && child->isDirectChildOf(*this))
        {
            child->emitDrawCommands(_context);
        }
    }

    if (clipChildren)
    {
        _context.popClipRect();
    }

    drawOverlayLayer(_context);
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

ego::gui::InputReply ego::gui::Widget::onPointerMove(WidgetUpdateContext&, const PointerMoveEvent&)
{
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Widget::onMouseButton(WidgetUpdateContext&, const MouseButtonEvent&)
{
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Widget::onMouseWheel(WidgetUpdateContext&, const MouseWheelEvent&)
{
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Widget::onKey(WidgetUpdateContext&, const KeyEvent&)
{
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Widget::onTextInput(WidgetUpdateContext&, const TextInputEvent&)
{
    return InputReply::Unhandled;
}

void ego::gui::Widget::onPointerEnter(WidgetUpdateContext&, const Position&, const InputModifiers&) {}

void ego::gui::Widget::onPointerLeave(WidgetUpdateContext&, const Position&, const InputModifiers&) {}

void ego::gui::Widget::onPointerCaptureLost(WidgetUpdateContext&, const Position&) {}

void ego::gui::Widget::onFocusChanged(WidgetUpdateContext&, FocusChange) {}

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
