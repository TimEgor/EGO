#include "Window.h"

#include <algorithm>
#include <utility>

#include "EgoGui/Input/Input.h"
#include "EgoGui/Input/InputContext.h"
#include "EgoGui/Layout/Layout.h"
#include "EgoGui/Rendering/PaintContext.h"
#include "EgoGui/Theme/Theme.h"

void ego::gui::Window::HierarchyAccessor::SetDocked(Window& _window, bool _isDocked)
{
    _window.m_isDocked = _isDocked;
}

ego::gui::Window::Window()
{
    applyWindowStyle(Theme::GetDefault().m_window);
}

ego::gui::WindowPointer ego::gui::Window::Create()
{
    return new Window();
}

ego::gui::WindowPointer ego::gui::Window::Create(WindowDesc _desc)
{
    const WindowPointer window = Create();
    window->setTitle(std::move(_desc.m_title));
    window->setBounds(_desc.m_bounds);
    window->onSizeChange(std::move(_desc.m_onSizeChanged));
    if (!window->setContent(_desc.m_content))
    {
        return nullptr;
    }

    return window;
}

void ego::gui::Window::setTitle(std::string _title)
{
    m_title = std::move(_title);
}

const std::string& ego::gui::Window::getTitle() const
{
    return m_title;
}

void ego::gui::Window::setPosition(const Position& _position)
{
    if (AreEqual(m_floatingBounds.m_position, _position))
    {
        return;
    }

    m_floatingBounds.m_position = _position;
    invalidateLayout();
}

const ego::gui::Position& ego::gui::Window::getPosition() const
{
    return isDocked() ? getLayoutBounds().m_position : m_floatingBounds.m_position;
}

void ego::gui::Window::setSize(const Size& _size)
{
    m_userSize = Size((std::max)(0.0f, _size.m_x), (std::max)(0.0f, _size.m_y));
    m_hasUserSize = true;
    updateFloatingSize();
}

const ego::gui::Size& ego::gui::Window::getSize() const
{
    return isDocked() ? getLayoutBounds().m_size : m_floatingBounds.m_size;
}

void ego::gui::Window::onSizeChange(WindowSizeChangedHandler _handler)
{
    m_onSizeChanged.set(std::move(_handler));
}

void ego::gui::Window::setMinimumSize(const Size& _size)
{
    m_hasMinimumSizeOverride = true;
    m_minimumSize = Size((std::max)(0.0f, _size.m_x), (std::max)(0.0f, _size.m_y));
    updateFloatingSize();
}

void ego::gui::Window::clearMinimumSize()
{
    m_hasMinimumSizeOverride = false;
    m_minimumSize = m_themeMinimumSize;
    updateFloatingSize();
}

const ego::gui::Size& ego::gui::Window::getMinimumSize() const
{
    return m_minimumSize;
}

bool ego::gui::Window::setContent(const WidgetPointer& _content)
{
    if (m_content.get() == _content.get())
    {
        return true;
    }

    if (_content && !attachChild(_content))
    {
        return false;
    }

    const WidgetPointer previousContent = m_content;
    m_content = _content;
    if (previousContent)
    {
        detachChild(previousContent);
    }

    notifyTreeChanged();

    return true;
}

ego::gui::WidgetPointer ego::gui::Window::getContent() const
{
    return m_content;
}

void ego::gui::Window::setBounds(const Rect& _bounds)
{
    setPosition(_bounds.m_position);
    setSize(_bounds.m_size);
}

const ego::gui::Rect& ego::gui::Window::getBounds() const
{
    return isDocked() ? getLayoutBounds() : m_floatingBounds;
}

const ego::gui::Rect& ego::gui::Window::getFloatingBounds() const
{
    return m_floatingBounds;
}

bool ego::gui::Window::isDocked() const
{
    return m_isDocked;
}

void ego::gui::Window::clearInteraction()
{
    m_interactionState = InteractionState::None;
    m_isInteractionPaused = false;
}

ego::gui::InputReply ego::gui::Window::onPointerMove(InputContext& _context, const PointerMoveEvent& _event)
{
    const PointerRegion pointerRegion = resolvePointerRegion(_event.m_position);
    if (m_interactionState != InteractionState::None)
    {
        m_isInteractionPaused = false;
        updateInteraction(_context, _event.m_position);
        updateHoveredRegion(_event.m_position);

        return InputReply::Handled;
    }

    m_hoveredRegion = pointerRegion;

    return pointerRegion == PointerRegion::ResizeGrip ? InputReply::Handled : InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Window::onMouseButton(InputContext& _context, const MouseButtonEvent& _event)
{
    const PointerRegion pointerRegion = resolvePointerRegion(_event.m_position);
    if (_event.m_action == InputButtonAction::Pressed)
    {
        if (_event.m_key == MouseInputKey::ButtonLeft && m_interactionState == InteractionState::None &&
            (pointerRegion == PointerRegion::TitleBar || pointerRegion == PointerRegion::ResizeGrip))
        {
            const InteractionState interactionState = pointerRegion == PointerRegion::ResizeGrip ? InteractionState::Resizing : InteractionState::Dragging;
            beginInteraction(_context, interactionState, _event.m_position);
            m_hoveredRegion = pointerRegion;

            return InputReply::FocusAndCapture;
        }

        if (isDocked() && pointerRegion == PointerRegion::Content)
        {
            return InputReply::Unhandled;
        }

        return pointerRegion != PointerRegion::None ? InputReply::Focus : InputReply::Unhandled;
    }

    if (_event.m_key == MouseInputKey::ButtonLeft && m_interactionState != InteractionState::None)
    {
        if (!m_isInteractionPaused)
        {
            updateInteraction(_context, _event.m_position);
        }

        endInteraction(_context, _event.m_position);
        updateHoveredRegion(_event.m_position);

        return InputReply::Handled;
    }

    return InputReply::Unhandled;
}

void ego::gui::Window::onPointerEnter(const Position& _position, const InputModifiers&)
{
    m_hoveredRegion = resolvePointerRegion(_position);
}

void ego::gui::Window::onPointerLeave(const Position&, const InputModifiers&)
{
    m_hoveredRegion = PointerRegion::None;
    pauseInteraction();
}

void ego::gui::Window::onPointerCaptureLost(const Position& _position)
{
    m_hoveredRegion = PointerRegion::None;
    cancelInteraction(_position);
}

ego::gui::Window::PointerRegion ego::gui::Window::resolvePointerRegion(const Position& _position) const
{
    if (!getLayoutBounds().contains(_position))
    {
        return PointerRegion::None;
    }

    if (isDocked())
    {
        return PointerRegion::Content;
    }

    if (m_resizeGripRect.contains(_position))
    {
        return PointerRegion::ResizeGrip;
    }

    if (m_titleBarRect.contains(_position))
    {
        return PointerRegion::TitleBar;
    }

    return PointerRegion::Content;
}

void ego::gui::Window::updateHoveredRegion(const Position& _position)
{
    m_hoveredRegion = resolvePointerRegion(_position);
}

ego::gui::Size ego::gui::Window::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    applyWindowStyle(_context.getTheme().m_window);

    const Size& maximumSize = _constraints.m_maximumSize;
    const Size effectiveMaximumSize =
        isDocked() ? maximumSize : Size((std::max)(maximumSize.m_x, m_floatingBounds.m_size.m_x), (std::max)(maximumSize.m_y, m_floatingBounds.m_size.m_y));
    const float titleBarHeight = isDocked() ? 0.0f : m_titleBarHeight;
    const Size contentMaximumSize(effectiveMaximumSize.m_x, (std::max)(0.0f, effectiveMaximumSize.m_y - titleBarHeight));
    if (m_content)
    {
        _context.measure(*m_content, LayoutConstraints(contentMaximumSize));
    }

    return effectiveMaximumSize;
}

void ego::gui::Window::updateGeometry(const LayoutContext& _context)
{
    applyWindowStyle(_context.getTheme().m_window);
    updateWindowRects();
    if (m_content)
    {
        _context.arrange(*m_content, m_contentRect);
    }
}

void ego::gui::Window::onLayoutCompleted()
{
    if (!AreEqual(m_reportedSize, getSize()))
    {
        notifySizeChanged();
    }
}

void ego::gui::Window::drawBaseLayer(PaintContext& _context) const
{
    const Theme& theme = _context.getTheme();
    const WindowStyle& window = theme.m_window;
    const Rect& layoutBounds = getLayoutBounds();

    _context.drawBox(layoutBounds, window.m_surface);
    if (isDocked())
    {
        return;
    }

    const NormalizedColorRGBA& titleBarColor =
        window.m_title.resolve(m_hoveredRegion == PointerRegion::TitleBar, m_interactionState == InteractionState::Dragging);
    _context.drawBox(m_titleBarRect, titleBarColor);

    const float titleWidth = (std::max)(0.0f, m_titleBarRect.m_size.m_x);
    const float titleHeight = (std::max)(0.0f, m_titleBarRect.m_size.m_y);
    const float titlePaddingLeft = (std::clamp)(window.m_titlePadding.m_left, 0.0f, titleWidth);
    const float titlePaddingTop = (std::clamp)(window.m_titlePadding.m_top, 0.0f, titleHeight);
    const float titlePaddingRight = (std::clamp)(window.m_titlePadding.m_right, 0.0f, titleWidth - titlePaddingLeft);
    const float titlePaddingBottom = (std::clamp)(window.m_titlePadding.m_bottom, 0.0f, titleHeight - titlePaddingTop);
    const Rect titleRect(
        m_titleBarRect.m_position.m_x + titlePaddingLeft,
        m_titleBarRect.m_position.m_y + titlePaddingTop,
        titleWidth - titlePaddingLeft - titlePaddingRight,
        titleHeight - titlePaddingTop - titlePaddingBottom);
    _context.drawText(m_title, titleRect, theme.m_typography.m_primary);
}

void ego::gui::Window::drawOverlayLayer(PaintContext& _context) const
{
    if (isDocked())
    {
        return;
    }

    const WindowStyle& window = _context.getTheme().m_window;
    const Rect& layoutBounds = getLayoutBounds();
    const NormalizedColorRGBA& resizeGripColor =
        window.m_resizeGrip.resolve(m_hoveredRegion == PointerRegion::ResizeGrip, m_interactionState == InteractionState::Resizing);

    _context.drawTriangle(
        Position(m_resizeGripRect.getRight(), m_resizeGripRect.getBottom()),
        Position(m_resizeGripRect.getLeft(), m_resizeGripRect.getBottom()),
        Position(m_resizeGripRect.getRight(), m_resizeGripRect.getTop()),
        resizeGripColor);
    _context.drawBorder(layoutBounds, m_windowBorderThickness, window.m_border);
}

bool ego::gui::Window::clipsChildren() const
{
    return true;
}

ego::gui::Rect ego::gui::Window::getChildrenClipRect() const
{
    return m_contentRect;
}

bool ego::gui::Window::isChildHitTestVisible(const Position& _position) const
{
    return m_contentRect.contains(_position) && !m_resizeGripRect.contains(_position);
}

size_t ego::gui::Window::getChildCount() const
{
    return m_content ? 1 : 0;
}

ego::gui::WidgetPointer ego::gui::Window::getChild(size_t) const
{
    return m_content;
}

void ego::gui::Window::beginInteraction(InputContext& _context, InteractionState _state, const Position& _position)
{
    m_interactionState = _state;
    m_isInteractionPaused = false;
    m_interactionStartPosition = _position;
    m_interactionStartBounds = m_floatingBounds;

    if (_state == InteractionState::Dragging)
    {
        const WindowPointer self = ego::StaticPointerCast<Window>(sharedFromThis());
        _context.beginFloatingWindowDrag(self, _position);
    }
}

void ego::gui::Window::pauseInteraction()
{
    m_isInteractionPaused = m_interactionState != InteractionState::None;
}

void ego::gui::Window::updateInteraction(InputContext& _context, const Position& _position)
{
    const Position offset(_position.m_x - m_interactionStartPosition.m_x, _position.m_y - m_interactionStartPosition.m_y);
    if (m_interactionState == InteractionState::Dragging)
    {
        const WindowPointer self = ego::StaticPointerCast<Window>(sharedFromThis());
        if (!_context.updateWindowDrag(self, _position))
        {
            setPosition(Position(m_interactionStartBounds.m_position.m_x + offset.m_x, m_interactionStartBounds.m_position.m_y + offset.m_y));
        }
    }
    else if (m_interactionState == InteractionState::Resizing)
    {
        const bool allowsInteractionOutsideSurface = _context.allowsInteractionOutsideSurface();
        if (!allowsInteractionOutsideSurface && !_context.getSurfaceBounds().contains(_position))
        {
            return;
        }

        Size size(m_interactionStartBounds.m_size.m_x + offset.m_x, m_interactionStartBounds.m_size.m_y + offset.m_y);
        if (!allowsInteractionOutsideSurface)
        {
            const Rect& surfaceBounds = _context.getSurfaceBounds();
            const Size maximumSize(
                (std::max)(0.0f, surfaceBounds.getRight() - m_interactionStartBounds.m_position.m_x),
                (std::max)(0.0f, surfaceBounds.getBottom() - m_interactionStartBounds.m_position.m_y));
            size.m_x = (std::min)(size.m_x, maximumSize.m_x);
            size.m_y = (std::min)(size.m_y, maximumSize.m_y);
        }

        applyUserSize(size);
    }
}

void ego::gui::Window::endInteraction(InputContext& _context, const Position& _position)
{
    const bool wasDragging = m_interactionState == InteractionState::Dragging;
    m_interactionState = InteractionState::None;
    m_isInteractionPaused = false;
    if (wasDragging)
    {
        const WindowPointer self = ego::StaticPointerCast<Window>(sharedFromThis());
        _context.finishWindowDrag(self, _position);
    }
}

void ego::gui::Window::cancelInteraction(const Position&)
{
    const bool wasResizing = m_interactionState == InteractionState::Resizing;
    const bool wasInteracting = m_interactionState != InteractionState::None;
    m_interactionState = InteractionState::None;
    m_isInteractionPaused = false;
    if (!wasInteracting)
    {
        return;
    }

    const Size currentSize = m_floatingBounds.m_size;
    m_floatingBounds = m_interactionStartBounds;
    m_userSize = m_interactionStartBounds.m_size;
    m_hasUserSize = true;
    invalidateLayout();
    if (wasResizing && !AreEqual(currentSize, m_floatingBounds.m_size))
    {
        notifySizeChanged();
    }
}

void ego::gui::Window::applyWindowStyle(const WindowStyle& _style)
{
    m_titleBarHeight = (std::max)(0.0f, _style.m_titleHeight);
    m_resizeGripSize = (std::max)(0.0f, _style.m_resizeGripSize);
    m_windowBorderThickness = (std::max)(0.0f, _style.m_borderThickness);

    const Size themeMinimumSize((std::max)(0.0f, _style.m_minimumSize.m_x), (std::max)(0.0f, _style.m_minimumSize.m_y));
    const bool themeMinimumSizeChanged = !AreEqual(m_themeMinimumSize, themeMinimumSize);
    m_themeMinimumSize = themeMinimumSize;
    if (!m_hasMinimumSizeOverride)
    {
        m_minimumSize = m_themeMinimumSize;
        if (themeMinimumSizeChanged)
        {
            updateFloatingSize();
        }
    }
}

bool ego::gui::Window::updateFloatingSize()
{
    const Size sourceSize = m_hasUserSize ? m_userSize : m_floatingBounds.m_size;
    if (!m_hasUserSize && sourceSize.m_x <= 0.0f && sourceSize.m_y <= 0.0f)
    {
        return false;
    }

    const Size targetSize((std::max)(m_minimumSize.m_x, sourceSize.m_x), (std::max)(m_minimumSize.m_y, sourceSize.m_y));
    if (AreEqual(m_floatingBounds.m_size, targetSize))
    {
        return false;
    }

    m_floatingBounds.m_size = targetSize;
    invalidateLayout();

    return true;
}

void ego::gui::Window::applyUserSize(const Size& _size)
{
    const Size previousSize = m_floatingBounds.m_size;
    setSize(_size);
    if (!AreEqual(previousSize, m_floatingBounds.m_size))
    {
        notifySizeChanged();
    }
}

void ego::gui::Window::updateWindowRects()
{
    const Rect& layoutBounds = getLayoutBounds();
    if (isDocked())
    {
        m_titleBarRect = Rect(layoutBounds.m_position, SizeZero);
        m_contentRect = layoutBounds;
        m_resizeGripRect = Rect(Position(layoutBounds.getRight(), layoutBounds.getBottom()), SizeZero);

        return;
    }

    const float titleBarHeight = (std::min)(m_titleBarHeight, layoutBounds.m_size.m_y);
    m_titleBarRect = Rect(layoutBounds.m_position.m_x, layoutBounds.m_position.m_y, layoutBounds.m_size.m_x, titleBarHeight);
    m_contentRect = Rect(
        layoutBounds.m_position.m_x,
        layoutBounds.m_position.m_y + titleBarHeight,
        layoutBounds.m_size.m_x,
        (std::max)(0.0f, layoutBounds.m_size.m_y - titleBarHeight));

    const float gripWidth = (std::min)(m_resizeGripSize, layoutBounds.m_size.m_x);
    const float gripHeight = (std::min)(m_resizeGripSize, layoutBounds.m_size.m_y);
    m_resizeGripRect = Rect(layoutBounds.getRight() - gripWidth, layoutBounds.getBottom() - gripHeight, gripWidth, gripHeight);
}

void ego::gui::Window::notifySizeChanged()
{
    m_reportedSize = getSize();
    const Size size = m_reportedSize;
    m_onSizeChanged.invoke(size);
}
