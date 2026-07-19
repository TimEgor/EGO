#include "Widget.h"

#include <algorithm>
#include <cmath>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Viewport/SurfaceRoot.h"

class ego::gui::Widget::AttachmentIdentity final
{
public:
    ego::WeakPointer<AttachmentIdentity> m_parent;
    ego::gui::WidgetWeakPointer m_self;
    ego::gui::WidgetWeakPointer m_parentWidget;
    ego::WeakPointer<ego::gui::SurfaceRoot> m_surfaceRoot;
};

ego::gui::Widget::Widget()
    : m_attachmentIdentity(new AttachmentIdentity())
{
}

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
    if (isCollapsed())
    {
        m_layoutBounds = Rect(_bounds.m_position, SizeZero);
        return;
    }

    m_layoutBounds = Rect(_bounds.m_position, Size((std::max)(0.0f, _bounds.m_size.m_x), (std::max)(0.0f, _bounds.m_size.m_y)));
    updateGeometry(_context);
}

void ego::gui::Widget::emitDrawCommands(PaintContext& _context) const
{
    if (!isVisible())
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
        const WidgetPointer& child = getChild(childIndex);
        if (child)
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
    if (!canMutateTree() || !_child || !m_attachmentIdentity || !_child->m_attachmentIdentity || _child->m_attachmentIdentity->m_parent.lock())
    {
        return false;
    }

    ego::SharedPointer<AttachmentIdentity> ancestorIdentity = m_attachmentIdentity;
    while (ancestorIdentity)
    {
        if (ancestorIdentity.get() == _child->m_attachmentIdentity.get())
        {
            return false;
        }

        ancestorIdentity = ancestorIdentity->m_parent.lock();
    }

    _child->m_attachmentIdentity->m_parent = m_attachmentIdentity;
    const WidgetPointer self = m_attachmentIdentity->m_self.lock();
    if (self)
    {
        _child->bindSurfaceRoot(_child, self, m_attachmentIdentity->m_surfaceRoot);
    }
    return true;
}

bool ego::gui::Widget::detachChild(const WidgetPointer& _child)
{
    if (!canMutateTree() || !_child || !_child->isDirectChildOf(*this))
    {
        return false;
    }

    _child->m_attachmentIdentity->m_parent.reset();
    _child->bindSurfaceRoot(_child, nullptr, ego::WeakPointer<SurfaceRoot>());
    return true;
}

bool ego::gui::Widget::canMutateTree() const
{
    const ego::SharedPointer<SurfaceRoot> surfaceRoot = m_attachmentIdentity ? m_attachmentIdentity->m_surfaceRoot.lock() : nullptr;
    const bool canMutate = !surfaceRoot || surfaceRoot->canMutateTree();
    EGO_ASSERT(canMutate);
    return canMutate;
}

void ego::gui::Widget::invalidateLayout() const
{
    const ego::SharedPointer<SurfaceRoot> surfaceRoot = m_attachmentIdentity ? m_attachmentIdentity->m_surfaceRoot.lock() : nullptr;
    if (surfaceRoot)
    {
        surfaceRoot->invalidateLayout();
    }
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

    if (!canMutateTree())
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

ego::gui::InputReply ego::gui::Widget::onPointerMove(const PointerMoveEvent&)
{
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Widget::onMouseButton(const MouseButtonEvent&)
{
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Widget::onMouseWheel(const MouseWheelEvent&)
{
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Widget::onKey(const KeyEvent&)
{
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Widget::onTextInput(const TextInputEvent&)
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

const ego::gui::WidgetPointer& ego::gui::Widget::getChild(size_t) const
{
    static const WidgetPointer NullWidget = nullptr;
    return NullWidget;
}

bool ego::gui::Widget::isChildHitTestVisible(const Position& _position) const
{
    return getLayoutBounds().contains(_position);
}

ego::gui::Rect ego::gui::Widget::resolveTopLevelBounds(const Rect& _surfaceBounds) const
{
    return _surfaceBounds;
}

bool ego::gui::Widget::isDirectChildOf(const Widget& _parent) const
{
    if (!m_attachmentIdentity || !_parent.m_attachmentIdentity)
    {
        return false;
    }

    const ego::SharedPointer<AttachmentIdentity> parentIdentity = m_attachmentIdentity->m_parent.lock();
    return parentIdentity.get() == _parent.m_attachmentIdentity.get();
}

ego::gui::WidgetPointer ego::gui::Widget::getParent() const
{
    return m_attachmentIdentity ? m_attachmentIdentity->m_parentWidget.lock() : nullptr;
}

bool ego::gui::Widget::isAttachedTo(const SurfaceRoot& _surfaceRoot) const
{
    const ego::SharedPointer<SurfaceRoot> surfaceRoot = m_attachmentIdentity ? m_attachmentIdentity->m_surfaceRoot.lock() : nullptr;
    return surfaceRoot.get() == &_surfaceRoot;
}

void ego::gui::Widget::bindSurfaceRoot(const WidgetPointer& _self, const WidgetPointer& _parent, const ego::WeakPointer<SurfaceRoot>& _surfaceRoot)
{
    if (!m_attachmentIdentity)
    {
        return;
    }

    m_attachmentIdentity->m_self = _self;
    m_attachmentIdentity->m_parentWidget = _parent;
    m_attachmentIdentity->m_surfaceRoot = _surfaceRoot;
    const size_t childCount = getChildCount();
    for (size_t childIndex = 0; childIndex < childCount; ++childIndex)
    {
        const WidgetPointer& child = getChild(childIndex);
        if (child && child->isDirectChildOf(*this))
        {
            child->bindSurfaceRoot(child, _self, _surfaceRoot);
        }
    }
}
