#include "Window.h"

#include <algorithm>
#include <utility>

#include "EgoGui/Theme/Theme.h"

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

ego::gui::Window::~Window()
{
    setContent(nullptr);
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
    if (m_requestedBounds.m_position.m_x == _position.m_x && m_requestedBounds.m_position.m_y == _position.m_y)
    {
        return;
    }

    m_requestedBounds.m_position = _position;
    invalidateLayout();
}

const ego::gui::Position& ego::gui::Window::getPosition() const
{
    return m_requestedBounds.m_position;
}

void ego::gui::Window::setSize(const Size& _size)
{
    m_requestedSize = Size((std::max)(0.0f, _size.m_x), (std::max)(0.0f, _size.m_y));
    m_hasRequestedSize = true;
    const Size requestedSize((std::max)(m_minimumSize.m_x, m_requestedSize.m_x), (std::max)(m_minimumSize.m_y, m_requestedSize.m_y));
    if (m_requestedBounds.m_size.m_x == requestedSize.m_x && m_requestedBounds.m_size.m_y == requestedSize.m_y)
    {
        return;
    }

    m_requestedBounds.m_size = requestedSize;
    invalidateLayout();
}

const ego::gui::Size& ego::gui::Window::getSize() const
{
    return m_requestedBounds.m_size;
}

void ego::gui::Window::onSizeChange(WindowSizeChangedHandler _handler)
{
    m_onSizeChanged.set(std::move(_handler));
}

void ego::gui::Window::setMinimumSize(const Size& _size)
{
    m_hasMinimumSizeOverride = true;
    m_minimumSize = Size((std::max)(0.0f, _size.m_x), (std::max)(0.0f, _size.m_y));
    setSize(m_requestedBounds.m_size);
}

void ego::gui::Window::clearMinimumSize()
{
    m_hasMinimumSizeOverride = false;
    m_minimumSize = m_themeMinimumSize;
    setSize(m_requestedBounds.m_size);
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

    if (!canMutateTree())
    {
        return false;
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
    return m_requestedBounds;
}

ego::gui::InputReply ego::gui::Window::onPointerMove(const PointerMoveEvent& _event)
{
    const PointerRegion pointerRegion = hitTest(_event.m_position);
    if (m_interactionState != InteractionState::None)
    {
        m_isInteractionPaused = false;
        updateInteraction(_event.m_position);
        updateHoveredRegion(_event.m_position);
        return InputReply::Handled;
    }

    m_hoveredRegion = pointerRegion;
    return pointerRegion == PointerRegion::ResizeGrip ? InputReply::Handled : InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Window::onMouseButton(const MouseButtonEvent& _event)
{
    const PointerRegion pointerRegion = hitTest(_event.m_position);
    if (_event.m_action == InputButtonAction::Pressed)
    {
        if (_event.m_key == MouseInputKey::ButtonLeft && m_interactionState == InteractionState::None &&
            (pointerRegion == PointerRegion::TitleBar || pointerRegion == PointerRegion::ResizeGrip))
        {
            const InteractionState interactionState = pointerRegion == PointerRegion::ResizeGrip ? InteractionState::Resizing : InteractionState::Dragging;
            beginInteraction(interactionState, _event.m_position);
            m_hoveredRegion = pointerRegion;
            return InputReply::FocusAndCapture;
        }

        return pointerRegion != PointerRegion::None ? InputReply::Focus : InputReply::Unhandled;
    }

    if (_event.m_key == MouseInputKey::ButtonLeft && m_interactionState != InteractionState::None)
    {
        if (!m_isInteractionPaused)
        {
            updateInteraction(_event.m_position);
        }

        endInteraction();
        updateHoveredRegion(_event.m_position);
        return InputReply::Handled;
    }

    return InputReply::Unhandled;
}

void ego::gui::Window::onPointerEnter(const Position& _position, const InputModifiers&)
{
    m_hoveredRegion = hitTest(_position);
}

void ego::gui::Window::onPointerLeave(const Position&, const InputModifiers&)
{
    m_hoveredRegion = PointerRegion::None;
    pauseInteraction();
}

void ego::gui::Window::onPointerCaptureLost(const Position&)
{
    m_hoveredRegion = PointerRegion::None;
    endInteraction();
}

ego::gui::Window::PointerRegion ego::gui::Window::hitTest(const Position& _position) const
{
    if (!getLayoutBounds().contains(_position))
    {
        return PointerRegion::None;
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
    m_hoveredRegion = hitTest(_position);
}

ego::gui::Size ego::gui::Window::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    applyWindowStyle(_context.getTheme().m_window);
    const Size& maximumSize = _constraints.m_maximumSize;
    const Size effectiveMaximumSize((std::max)(maximumSize.m_x, m_requestedBounds.m_size.m_x), (std::max)(maximumSize.m_y, m_requestedBounds.m_size.m_y));
    const Size contentMaximumSize(effectiveMaximumSize.m_x, (std::max)(0.0f, effectiveMaximumSize.m_y - m_titleBarHeight));
    const WidgetPointer content = m_content;
    if (content)
    {
        content->updatePreferredSize(_context, LayoutConstraints(contentMaximumSize));
    }

    return effectiveMaximumSize;
}

void ego::gui::Window::updateGeometry(const LayoutContext& _context)
{
    applyWindowStyle(_context.getTheme().m_window);
    updateWindowRects();
    const WidgetPointer content = m_content;
    if (content)
    {
        content->applyLayout(_context, m_contentRect);
    }
}

void ego::gui::Window::drawBaseLayer(PaintContext& _context) const
{
    const Theme& theme = _context.getTheme();
    const WindowStyle& window = theme.m_window;

    const Rect& layoutBounds = getLayoutBounds();
    _context.drawBox(layoutBounds, window.m_surface);

    const NormalizedColorRGBA& titleBarColor = window.m_title.resolve(m_hoveredRegion == PointerRegion::TitleBar, m_interactionState == InteractionState::Dragging);

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
    const WindowStyle& window = _context.getTheme().m_window;
    const Rect& layoutBounds = getLayoutBounds();

    const NormalizedColorRGBA& resizeGripColor = window.m_resizeGrip.resolve(m_hoveredRegion == PointerRegion::ResizeGrip, m_interactionState == InteractionState::Resizing);

    _context.drawTriangle(
        Position(m_resizeGripRect.getRight(), m_resizeGripRect.getBottom()),
        Position(m_resizeGripRect.getLeft(), m_resizeGripRect.getBottom()),
        Position(m_resizeGripRect.getRight(), m_resizeGripRect.getTop()),
        resizeGripColor);

    _context.drawBox(Rect(layoutBounds.getLeft(), layoutBounds.getTop(), layoutBounds.m_size.m_x, m_windowBorderThickness), window.m_border);
    _context.drawBox(Rect(layoutBounds.getLeft(), layoutBounds.getBottom() - m_windowBorderThickness, layoutBounds.m_size.m_x, m_windowBorderThickness), window.m_border);
    _context.drawBox(Rect(layoutBounds.getLeft(), layoutBounds.getTop(), m_windowBorderThickness, layoutBounds.m_size.m_y), window.m_border);
    _context.drawBox(Rect(layoutBounds.getRight() - m_windowBorderThickness, layoutBounds.getTop(), m_windowBorderThickness, layoutBounds.m_size.m_y), window.m_border);
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

const ego::gui::WidgetPointer& ego::gui::Window::getChild(size_t) const
{
    return m_content;
}

ego::gui::Rect ego::gui::Window::resolveTopLevelBounds(const Rect&) const
{
    return m_requestedBounds;
}

void ego::gui::Window::beginInteraction(InteractionState _state, const Position& _position)
{
    m_interactionState = _state;
    m_isInteractionPaused = false;
    m_interactionStartPosition = _position;
    m_interactionStartBounds = m_requestedBounds;
}

void ego::gui::Window::pauseInteraction()
{
    m_isInteractionPaused = m_interactionState != InteractionState::None;
}

void ego::gui::Window::updateInteraction(const Position& _position)
{
    const Position offset(_position.m_x - m_interactionStartPosition.m_x, _position.m_y - m_interactionStartPosition.m_y);

    if (m_interactionState == InteractionState::Dragging)
    {
        setPosition(Position(m_interactionStartBounds.m_position.m_x + offset.m_x, m_interactionStartBounds.m_position.m_y + offset.m_y));
    }
    else if (m_interactionState == InteractionState::Resizing)
    {
        applyUserSize(Size(m_interactionStartBounds.m_size.m_x + offset.m_x, m_interactionStartBounds.m_size.m_y + offset.m_y));
    }
}

void ego::gui::Window::endInteraction()
{
    m_interactionState = InteractionState::None;
    m_isInteractionPaused = false;
}

void ego::gui::Window::applyWindowStyle(const WindowStyle& _style)
{
    m_titleBarHeight = (std::max)(0.0f, _style.m_titleHeight);
    m_resizeGripSize = (std::max)(0.0f, _style.m_resizeGripSize);
    m_windowBorderThickness = (std::max)(0.0f, _style.m_borderThickness);
    const Size themeMinimumSize((std::max)(0.0f, _style.m_minimumSize.m_x), (std::max)(0.0f, _style.m_minimumSize.m_y));
    const bool themeMinimumSizeChanged = m_themeMinimumSize.m_x != themeMinimumSize.m_x || m_themeMinimumSize.m_y != themeMinimumSize.m_y;
    m_themeMinimumSize = themeMinimumSize;
    if (!m_hasMinimumSizeOverride)
    {
        m_minimumSize = m_themeMinimumSize;
        const Size targetSize = themeMinimumSizeChanged && m_hasRequestedSize ? m_requestedSize : m_requestedBounds.m_size;
        if (m_hasRequestedSize || targetSize.m_x > 0.0f || targetSize.m_y > 0.0f)
        {
            m_requestedBounds.m_size = Size((std::max)(m_minimumSize.m_x, targetSize.m_x), (std::max)(m_minimumSize.m_y, targetSize.m_y));
        }
    }
}

void ego::gui::Window::applyUserSize(const Size& _size)
{
    const Size previousSize = m_requestedBounds.m_size;
    setSize(_size);
    if (previousSize.m_x != m_requestedBounds.m_size.m_x || previousSize.m_y != m_requestedBounds.m_size.m_y)
    {
        notifySizeChanged();
    }
}

void ego::gui::Window::updateWindowRects()
{
    const Rect& layoutBounds = getLayoutBounds();
    const float titleBarHeight = (std::min)(m_titleBarHeight, layoutBounds.m_size.m_y);
    m_titleBarRect = Rect(layoutBounds.m_position.m_x, layoutBounds.m_position.m_y, layoutBounds.m_size.m_x, titleBarHeight);
    m_contentRect =
        Rect(layoutBounds.m_position.m_x, layoutBounds.m_position.m_y + titleBarHeight, layoutBounds.m_size.m_x, (std::max)(0.0f, layoutBounds.m_size.m_y - titleBarHeight));

    const float gripWidth = (std::min)(m_resizeGripSize, layoutBounds.m_size.m_x);
    const float gripHeight = (std::min)(m_resizeGripSize, layoutBounds.m_size.m_y);
    m_resizeGripRect = Rect(layoutBounds.getRight() - gripWidth, layoutBounds.getBottom() - gripHeight, gripWidth, gripHeight);
}

void ego::gui::Window::notifySizeChanged()
{
    const Size size = m_requestedBounds.m_size;
    m_onSizeChanged.invoke(size);
}
