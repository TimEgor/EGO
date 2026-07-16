#include "GuiWindow.h"

#include <algorithm>
#include <utility>

namespace
{
    constexpr float TitleBarHeight = 28.0f;
    constexpr float TitlePaddingX = 8.0f;
    constexpr float TitlePaddingY = 6.0f;
    constexpr float ResizeGripSize = 16.0f;
    constexpr float WindowBorderThickness = 1.0f;
} // namespace

ego::gui::GuiWindowPointer ego::gui::GuiWindow::Create()
{
    return new GuiWindow();
}

void ego::gui::GuiWindow::setTitle(std::string _title)
{
    m_title = std::move(_title);
}

const std::string& ego::gui::GuiWindow::getTitle() const
{
    return m_title;
}

void ego::gui::GuiWindow::setPosition(const GuiPosition& _position)
{
    m_bounds.m_position = _position;
    updateWindowRects();
}

const ego::gui::GuiPosition& ego::gui::GuiWindow::getPosition() const
{
    return m_bounds.m_position;
}

void ego::gui::GuiWindow::setSize(const GuiSize& _size)
{
    m_bounds.m_size = GuiSize((std::max)(m_minimumSize.m_x, _size.m_x), (std::max)(m_minimumSize.m_y, _size.m_y));
    updateWindowRects();
}

const ego::gui::GuiSize& ego::gui::GuiWindow::getSize() const
{
    return m_bounds.m_size;
}

void ego::gui::GuiWindow::setMinimumSize(const GuiSize& _size)
{
    m_minimumSize = GuiSize((std::max)(0.0f, _size.m_x), (std::max)(0.0f, _size.m_y));
    setSize(m_bounds.m_size);
}

const ego::gui::GuiSize& ego::gui::GuiWindow::getMinimumSize() const
{
    return m_minimumSize;
}

void ego::gui::GuiWindow::setContent(const GuiWidgetPointer& _content)
{
    if (m_content.get() == _content.get())
    {
        return;
    }

    GuiInputEvent focusLostEvent;
    focusLostEvent.m_type = GuiInputEventType::FocusLost;
    dispatchToContent(focusLostEvent);

    m_content = _content;
}

ego::gui::GuiWidgetPointer ego::gui::GuiWindow::getContent() const
{
    return m_content;
}

const ego::gui::GuiRect& ego::gui::GuiWindow::getWindowRect() const
{
    return m_bounds;
}

ego::gui::GuiEventResult ego::gui::GuiWindow::onEvent(const GuiInputEvent& _event)
{
    switch (_event.m_type)
    {
    case GuiInputEventType::FocusLost:
        m_hoveredRegion = PointerRegion::None;
        endInteraction();
        dispatchToContent(_event);
        return GuiEventResult::Unhandled;

    case GuiInputEventType::PointerLeave:
        m_hoveredRegion = PointerRegion::None;
        pauseInteraction();
        dispatchToContent(_event);
        return GuiEventResult::Unhandled;

    case GuiInputEventType::MouseMove:
    case GuiInputEventType::MouseButtonDown:
    case GuiInputEventType::MouseButtonUp:
    case GuiInputEventType::MouseWheel:
        return processPointerEvent(_event);

    case GuiInputEventType::KeyDown:
    case GuiInputEventType::KeyUp:
    case GuiInputEventType::TextInput:
        return dispatchToContent(_event);

    case GuiInputEventType::Undefined:
    default:
        return GuiEventResult::Unhandled;
    }
}

ego::gui::GuiEventResult ego::gui::GuiWindow::processPointerEvent(const GuiInputEvent& _event)
{
    const PointerRegion pointerRegion = hitTest(_event.m_position);

    switch (_event.m_type)
    {
    case GuiInputEventType::MouseMove:
        if (m_interactionState != InteractionState::None)
        {
            m_isInteractionPaused = false;
            updateInteraction(_event.m_position);
            updateHoveredRegion(_event.m_position);
            return GuiEventResult::Handled;
        }

        m_hoveredRegion = pointerRegion;
        if (pointerRegion == PointerRegion::ResizeGrip)
        {
            GuiInputEvent pointerLeaveEvent;
            pointerLeaveEvent.m_type = GuiInputEventType::PointerLeave;
            pointerLeaveEvent.m_position = _event.m_position;
            dispatchToContent(pointerLeaveEvent);
            return GuiEventResult::Handled;
        }
        break;

    case GuiInputEventType::MouseButtonDown:
        if (_event.m_mouseButton == GuiMouseButton::Left && (pointerRegion == PointerRegion::TitleBar || pointerRegion == PointerRegion::ResizeGrip))
        {
            GuiInputEvent focusLostEvent;
            focusLostEvent.m_type = GuiInputEventType::FocusLost;
            dispatchToContent(focusLostEvent);

            const InteractionState interactionState = pointerRegion == PointerRegion::ResizeGrip ? InteractionState::Resizing : InteractionState::Dragging;
            beginInteraction(interactionState, _event.m_position);
            m_hoveredRegion = pointerRegion;
            return GuiEventResult::Handled;
        }
        break;

    case GuiInputEventType::MouseButtonUp:
        if (_event.m_mouseButton == GuiMouseButton::Left && m_interactionState != InteractionState::None)
        {
            if (!m_isInteractionPaused)
            {
                updateInteraction(_event.m_position);
            }

            endInteraction();
            updateHoveredRegion(_event.m_position);
            return GuiEventResult::Handled;
        }
        break;

    case GuiInputEventType::MouseWheel:
        break;

    default:
        return GuiEventResult::Unhandled;
    }

    const GuiEventResult contentResult = dispatchToContent(_event);
    if (contentResult == GuiEventResult::Handled)
    {
        return contentResult;
    }

    if (pointerRegion != PointerRegion::None &&
        (_event.m_type == GuiInputEventType::MouseButtonDown || _event.m_type == GuiInputEventType::MouseButtonUp || _event.m_type == GuiInputEventType::MouseWheel))
    {
        return GuiEventResult::Handled;
    }

    return GuiEventResult::Unhandled;
}

ego::gui::GuiEventResult ego::gui::GuiWindow::dispatchToContent(const GuiInputEvent& _event)
{
    return m_content ? m_content->handleEvent(_event) : GuiEventResult::Unhandled;
}

ego::gui::GuiWindow::PointerRegion ego::gui::GuiWindow::hitTest(const GuiPosition& _position) const
{
    if (!m_bounds.contains(_position))
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

void ego::gui::GuiWindow::updateHoveredRegion(const GuiPosition& _position)
{
    m_hoveredRegion = hitTest(_position);
}

ego::gui::GuiSize ego::gui::GuiWindow::onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize)
{
    const GuiSize contentAvailableSize(_availableSize.m_x, (std::max)(0.0f, _availableSize.m_y - TitleBarHeight));
    if (m_content)
    {
        m_content->measure(_context, contentAvailableSize);
    }

    return _availableSize;
}

void ego::gui::GuiWindow::onArrange(const GuiLayoutContext& _context, const GuiRect& _rect)
{
    m_bounds = _rect;
    updateWindowRects();
    if (m_content)
    {
        m_content->arrange(_context, m_contentRect);
    }
}

void ego::gui::GuiWindow::onPaint(GuiPaintContext& _context) const
{
    _context.drawBox(m_bounds, GuiColor(0.075f, 0.080f, 0.090f, 1.0f));

    GuiColor titleBarColor(0.13f, 0.14f, 0.16f, 1.0f);
    if (m_hoveredRegion == PointerRegion::TitleBar)
    {
        titleBarColor = GuiColor(0.17f, 0.19f, 0.22f, 1.0f);
    }
    if (m_interactionState == InteractionState::Dragging)
    {
        titleBarColor = GuiColor(0.20f, 0.23f, 0.27f, 1.0f);
    }

    _context.drawBox(m_titleBarRect, titleBarColor);

    const GuiRect titleRect(
        m_titleBarRect.m_position.m_x + TitlePaddingX,
        m_titleBarRect.m_position.m_y + TitlePaddingY,
        (std::max)(0.0f, m_titleBarRect.m_size.m_x - TitlePaddingX * 2.0f),
        (std::max)(0.0f, m_titleBarRect.m_size.m_y - TitlePaddingY));
    _context.drawText(m_title, titleRect, GuiColorWhite);

    if (m_content)
    {
        _context.pushClipRect(m_contentRect);
        m_content->paint(_context);
        _context.popClipRect();
    }

    GuiColor resizeGripColor(0.32f, 0.35f, 0.40f, 1.0f);
    if (m_hoveredRegion == PointerRegion::ResizeGrip)
    {
        resizeGripColor = GuiColor(0.45f, 0.50f, 0.58f, 1.0f);
    }
    if (m_interactionState == InteractionState::Resizing)
    {
        resizeGripColor = GuiColor(0.58f, 0.65f, 0.76f, 1.0f);
    }

    _context.drawTriangle(
        GuiPosition(m_resizeGripRect.getRight(), m_resizeGripRect.getBottom()),
        GuiPosition(m_resizeGripRect.getLeft(), m_resizeGripRect.getBottom()),
        GuiPosition(m_resizeGripRect.getRight(), m_resizeGripRect.getTop()),
        resizeGripColor);

    const GuiColor borderColor(0.30f, 0.32f, 0.36f, 1.0f);
    _context.drawBox(GuiRect(m_bounds.getLeft(), m_bounds.getTop(), m_bounds.m_size.m_x, WindowBorderThickness), borderColor);
    _context.drawBox(GuiRect(m_bounds.getLeft(), m_bounds.getBottom() - WindowBorderThickness, m_bounds.m_size.m_x, WindowBorderThickness), borderColor);
    _context.drawBox(GuiRect(m_bounds.getLeft(), m_bounds.getTop(), WindowBorderThickness, m_bounds.m_size.m_y), borderColor);
    _context.drawBox(GuiRect(m_bounds.getRight() - WindowBorderThickness, m_bounds.getTop(), WindowBorderThickness, m_bounds.m_size.m_y), borderColor);
}

void ego::gui::GuiWindow::beginInteraction(InteractionState _state, const GuiPosition& _position)
{
    m_interactionState = _state;
    m_isInteractionPaused = false;
    m_interactionStartPosition = _position;
    m_interactionStartBounds = m_bounds;
}

void ego::gui::GuiWindow::pauseInteraction()
{
    m_isInteractionPaused = m_interactionState != InteractionState::None;
}

void ego::gui::GuiWindow::updateInteraction(const GuiPosition& _position)
{
    const GuiPosition offset(_position.m_x - m_interactionStartPosition.m_x, _position.m_y - m_interactionStartPosition.m_y);

    if (m_interactionState == InteractionState::Dragging)
    {
        setPosition(GuiPosition(m_interactionStartBounds.m_position.m_x + offset.m_x, m_interactionStartBounds.m_position.m_y + offset.m_y));
    }
    else if (m_interactionState == InteractionState::Resizing)
    {
        setSize(GuiSize(m_interactionStartBounds.m_size.m_x + offset.m_x, m_interactionStartBounds.m_size.m_y + offset.m_y));
    }
}

void ego::gui::GuiWindow::endInteraction()
{
    m_interactionState = InteractionState::None;
    m_isInteractionPaused = false;
}

void ego::gui::GuiWindow::updateWindowRects()
{
    const float titleBarHeight = (std::min)(TitleBarHeight, m_bounds.m_size.m_y);
    m_titleBarRect = GuiRect(m_bounds.m_position.m_x, m_bounds.m_position.m_y, m_bounds.m_size.m_x, titleBarHeight);
    m_contentRect = GuiRect(m_bounds.m_position.m_x, m_bounds.m_position.m_y + titleBarHeight, m_bounds.m_size.m_x, (std::max)(0.0f, m_bounds.m_size.m_y - titleBarHeight));

    const float gripWidth = (std::min)(ResizeGripSize, m_bounds.m_size.m_x);
    const float gripHeight = (std::min)(ResizeGripSize, m_bounds.m_size.m_y);
    m_resizeGripRect = GuiRect(m_bounds.getRight() - gripWidth, m_bounds.getBottom() - gripHeight, gripWidth, gripHeight);
}
