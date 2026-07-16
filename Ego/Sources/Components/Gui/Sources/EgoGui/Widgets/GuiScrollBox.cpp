#include "GuiScrollBox.h"

#include <algorithm>
#include <cstdint>
#include <limits>

namespace
{
    constexpr float ScrollBarThickness = 12.0f;
    constexpr float MinimumThumbSize = 18.0f;
    constexpr float MouseWheelStep = 40.0f;

    constexpr ego::gui::GuiColor ScrollBarTrackColor(0.08f, 0.09f, 0.10f, 1.0f);
    constexpr ego::gui::GuiColor ScrollBarThumbColor(0.28f, 0.31f, 0.35f, 1.0f);
    constexpr ego::gui::GuiColor ScrollBarThumbHoveredColor(0.38f, 0.42f, 0.48f, 1.0f);
    constexpr ego::gui::GuiColor ScrollBarThumbPressedColor(0.48f, 0.54f, 0.62f, 1.0f);
} // namespace

ego::gui::GuiScrollBoxPointer ego::gui::GuiScrollBox::Create()
{
    return new GuiScrollBox();
}

void ego::gui::GuiScrollBox::setContent(const GuiWidgetPointer& _content)
{
    if (getContent().get() == _content.get())
    {
        return;
    }

    GuiContainer::clearChildren();
    if (_content)
    {
        GuiContainer::addChild(_content);
    }

    m_scrollOffset = GuiPositionZero;
    m_scrollRange = GuiSizeZero;
    endScrollInteraction();
}

ego::gui::GuiWidgetPointer ego::gui::GuiScrollBox::getContent() const
{
    const ChildCollection& children = getChildren();
    return children.empty() ? nullptr : children.front();
}

void ego::gui::GuiScrollBox::setHorizontalScrollBarMode(GuiScrollBarMode _mode)
{
    m_horizontalScrollBarMode = _mode;
    if (_mode == GuiScrollBarMode::Disabled)
    {
        m_scrollOffset.m_x = 0.0f;
        m_scrollRange.m_x = 0.0f;
        m_isHorizontalScrollBarVisible = false;
    }
}

ego::gui::GuiScrollBarMode ego::gui::GuiScrollBox::getHorizontalScrollBarMode() const
{
    return m_horizontalScrollBarMode;
}

void ego::gui::GuiScrollBox::setVerticalScrollBarMode(GuiScrollBarMode _mode)
{
    m_verticalScrollBarMode = _mode;
    if (_mode == GuiScrollBarMode::Disabled)
    {
        m_scrollOffset.m_y = 0.0f;
        m_scrollRange.m_y = 0.0f;
        m_isVerticalScrollBarVisible = false;
    }
}

ego::gui::GuiScrollBarMode ego::gui::GuiScrollBox::getVerticalScrollBarMode() const
{
    return m_verticalScrollBarMode;
}

void ego::gui::GuiScrollBox::setScrollOffset(const GuiPosition& _offset)
{
    m_scrollOffset = GuiPosition((std::clamp)(_offset.m_x, 0.0f, m_scrollRange.m_x), (std::clamp)(_offset.m_y, 0.0f, m_scrollRange.m_y));
    updateScrollBarRects();
}

const ego::gui::GuiPosition& ego::gui::GuiScrollBox::getScrollOffset() const
{
    return m_scrollOffset;
}

const ego::gui::GuiSize& ego::gui::GuiScrollBox::getScrollRange() const
{
    return m_scrollRange;
}

ego::gui::GuiEventResult ego::gui::GuiScrollBox::onEvent(const GuiInputEvent& _event)
{
    if (_event.m_type == GuiInputEventType::FocusLost)
    {
        m_hoveredScrollPart = ScrollPart::None;
        endScrollInteraction();
        return GuiContainer::onEvent(_event);
    }

    if (_event.m_type == GuiInputEventType::PointerLeave)
    {
        m_hoveredScrollPart = ScrollPart::None;
        return GuiContainer::onEvent(_event);
    }

    if (_event.m_type == GuiInputEventType::MouseWheel)
    {
        const GuiEventResult childResult = GuiContainer::onEvent(_event);
        if (childResult == GuiEventResult::Handled)
        {
            return childResult;
        }

        if (getRect().contains(_event.m_position) && applyMouseWheel(_event.m_wheelDelta))
        {
            clearHoveredChild(_event.m_position);
            return GuiEventResult::Handled;
        }

        return GuiEventResult::Unhandled;
    }

    if (_event.m_type == GuiInputEventType::MouseMove)
    {
        m_hoveredScrollPart = findScrollPart(_event.m_position);
        if (m_scrollInteraction != ScrollPart::None)
        {
            updateScrollInteraction(_event.m_position);
            return GuiEventResult::Handled;
        }
    }

    if (_event.m_type == GuiInputEventType::MouseButtonDown && _event.m_mouseButton == GuiMouseButton::Left)
    {
        const ScrollPart scrollPart = findScrollPart(_event.m_position);
        m_hoveredScrollPart = scrollPart;
        if (scrollPart != ScrollPart::None)
        {
            clearHoveredChild(_event.m_position);
            beginScrollInteraction(scrollPart, _event.m_position);
            return GuiEventResult::Handled;
        }
    }

    if (_event.m_type == GuiInputEventType::MouseButtonUp && _event.m_mouseButton == GuiMouseButton::Left && m_scrollInteraction != ScrollPart::None)
    {
        updateScrollInteraction(_event.m_position);
        endScrollInteraction();
        m_hoveredScrollPart = findScrollPart(_event.m_position);
        return GuiEventResult::Handled;
    }

    return GuiContainer::onEvent(_event);
}

ego::gui::GuiSize ego::gui::GuiScrollBox::onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize)
{
    const GuiWidgetPointer content = getContent();
    if (!content)
    {
        return GuiSizeZero;
    }

    constexpr float unboundedExtent = (std::numeric_limits<float>::max)();
    const GuiSize contentAvailableSize(
        m_horizontalScrollBarMode == GuiScrollBarMode::Disabled ? _availableSize.m_x : unboundedExtent,
        m_verticalScrollBarMode == GuiScrollBarMode::Disabled ? _availableSize.m_y : unboundedExtent);
    const GuiSize contentDesiredSize = content->measure(_context, contentAvailableSize);

    return GuiSize(
        m_horizontalScrollBarMode == GuiScrollBarMode::Disabled ? contentDesiredSize.m_x : (std::min)(contentDesiredSize.m_x, _availableSize.m_x),
        m_verticalScrollBarMode == GuiScrollBarMode::Disabled ? contentDesiredSize.m_y : (std::min)(contentDesiredSize.m_y, _availableSize.m_y));
}

void ego::gui::GuiScrollBox::onArrange(const GuiLayoutContext& _context, const GuiRect& _rect)
{
    updateScrollLayout(_rect);

    const GuiWidgetPointer content = getContent();
    if (content)
    {
        const GuiRect contentRect(m_viewportRect.m_position.m_x - m_scrollOffset.m_x, m_viewportRect.m_position.m_y - m_scrollOffset.m_y, m_contentSize.m_x, m_contentSize.m_y);
        content->arrange(_context, contentRect);
    }
}

void ego::gui::GuiScrollBox::onPaint(GuiPaintContext& _context) const
{
    _context.pushClipRect(m_viewportRect);
    GuiContainer::onPaint(_context);
    _context.popClipRect();

    if (m_isHorizontalScrollBarVisible)
    {
        _context.drawBox(m_horizontalTrackRect, ScrollBarTrackColor);

        GuiColor thumbColor = ScrollBarThumbColor;
        if (m_hoveredScrollPart == ScrollPart::HorizontalThumb)
        {
            thumbColor = ScrollBarThumbHoveredColor;
        }
        if (m_scrollInteraction == ScrollPart::HorizontalThumb)
        {
            thumbColor = ScrollBarThumbPressedColor;
        }
        _context.drawBox(m_horizontalThumbRect, thumbColor);
    }

    if (m_isVerticalScrollBarVisible)
    {
        _context.drawBox(m_verticalTrackRect, ScrollBarTrackColor);

        GuiColor thumbColor = ScrollBarThumbColor;
        if (m_hoveredScrollPart == ScrollPart::VerticalThumb)
        {
            thumbColor = ScrollBarThumbHoveredColor;
        }
        if (m_scrollInteraction == ScrollPart::VerticalThumb)
        {
            thumbColor = ScrollBarThumbPressedColor;
        }
        _context.drawBox(m_verticalThumbRect, thumbColor);
    }

    if (m_isHorizontalScrollBarVisible && m_isVerticalScrollBarVisible)
    {
        _context.drawBox(GuiRect(m_viewportRect.getRight(), m_viewportRect.getBottom(), ScrollBarThickness, ScrollBarThickness), ScrollBarTrackColor);
    }
}

bool ego::gui::GuiScrollBox::isChildHitTestVisible(const GuiPosition& _position) const
{
    return m_viewportRect.contains(_position);
}

ego::gui::GuiRect ego::gui::GuiScrollBox::CreateScrollThumbRect(const GuiRect& _trackRect, float _viewportExtent, float _contentExtent, float _offset, float _range, Axis _axis)
{
    const float trackExtent = _axis == Axis::Horizontal ? _trackRect.m_size.m_x : _trackRect.m_size.m_y;
    if (trackExtent <= 0.0f)
    {
        return GuiRect(_trackRect.m_position, GuiSizeZero);
    }

    const float visibleRatio = _contentExtent > 0.0f ? (std::min)(1.0f, _viewportExtent / _contentExtent) : 1.0f;
    const float thumbExtent = (std::min)(trackExtent, (std::max)(MinimumThumbSize, trackExtent * visibleRatio));
    const float thumbTravel = trackExtent - thumbExtent;
    const float thumbOffset = _range > 0.0f ? thumbTravel * _offset / _range : 0.0f;

    if (_axis == Axis::Horizontal)
    {
        return GuiRect(_trackRect.m_position.m_x + thumbOffset, _trackRect.m_position.m_y, thumbExtent, _trackRect.m_size.m_y);
    }

    return GuiRect(_trackRect.m_position.m_x, _trackRect.m_position.m_y + thumbOffset, _trackRect.m_size.m_x, thumbExtent);
}

ego::gui::GuiScrollBox::ScrollPart ego::gui::GuiScrollBox::findScrollPart(const GuiPosition& _position) const
{
    if (m_isHorizontalScrollBarVisible)
    {
        if (m_horizontalThumbRect.contains(_position))
        {
            return ScrollPart::HorizontalThumb;
        }
        if (m_horizontalTrackRect.contains(_position))
        {
            return ScrollPart::HorizontalTrack;
        }
    }

    if (m_isVerticalScrollBarVisible)
    {
        if (m_verticalThumbRect.contains(_position))
        {
            return ScrollPart::VerticalThumb;
        }
        if (m_verticalTrackRect.contains(_position))
        {
            return ScrollPart::VerticalTrack;
        }
    }

    return ScrollPart::None;
}

void ego::gui::GuiScrollBox::beginScrollInteraction(ScrollPart _part, const GuiPosition& _position)
{
    m_scrollInteraction = _part;
    m_interactionStartPosition = _position;
    m_interactionStartOffset = m_scrollOffset;

    GuiPosition offset = m_scrollOffset;
    if (_part == ScrollPart::HorizontalTrack)
    {
        offset.m_x += _position.m_x < m_horizontalThumbRect.getLeft() ? -m_viewportRect.m_size.m_x : m_viewportRect.m_size.m_x;
    }
    else if (_part == ScrollPart::VerticalTrack)
    {
        offset.m_y += _position.m_y < m_verticalThumbRect.getTop() ? -m_viewportRect.m_size.m_y : m_viewportRect.m_size.m_y;
    }
    setScrollOffset(offset);
}

void ego::gui::GuiScrollBox::updateScrollInteraction(const GuiPosition& _position)
{
    if (m_scrollInteraction != ScrollPart::HorizontalThumb && m_scrollInteraction != ScrollPart::VerticalThumb)
    {
        return;
    }

    GuiPosition offset = m_interactionStartOffset;
    if (m_scrollInteraction == ScrollPart::HorizontalThumb)
    {
        const float thumbTravel = m_horizontalTrackRect.m_size.m_x - m_horizontalThumbRect.m_size.m_x;
        if (thumbTravel > 0.0f)
        {
            offset.m_x += (_position.m_x - m_interactionStartPosition.m_x) * m_scrollRange.m_x / thumbTravel;
        }
    }
    else if (m_scrollInteraction == ScrollPart::VerticalThumb)
    {
        const float thumbTravel = m_verticalTrackRect.m_size.m_y - m_verticalThumbRect.m_size.m_y;
        if (thumbTravel > 0.0f)
        {
            offset.m_y += (_position.m_y - m_interactionStartPosition.m_y) * m_scrollRange.m_y / thumbTravel;
        }
    }
    setScrollOffset(offset);
}

void ego::gui::GuiScrollBox::endScrollInteraction()
{
    m_scrollInteraction = ScrollPart::None;
}

bool ego::gui::GuiScrollBox::applyMouseWheel(float _delta)
{
    if (_delta == 0.0f)
    {
        return false;
    }

    const GuiPosition previousOffset = m_scrollOffset;
    GuiPosition offset = m_scrollOffset;
    if (m_scrollRange.m_y > 0.0f)
    {
        offset.m_y -= _delta * MouseWheelStep;
    }
    else if (m_scrollRange.m_x > 0.0f)
    {
        offset.m_x -= _delta * MouseWheelStep;
    }

    setScrollOffset(offset);
    return previousOffset.m_x != m_scrollOffset.m_x || previousOffset.m_y != m_scrollOffset.m_y;
}

void ego::gui::GuiScrollBox::updateScrollLayout(const GuiRect& _rect)
{
    const GuiWidgetPointer content = getContent();
    const GuiSize contentDesiredSize = content ? content->getDesiredSize() : GuiSizeZero;

    m_isHorizontalScrollBarVisible = m_horizontalScrollBarMode == GuiScrollBarMode::Always;
    m_isVerticalScrollBarVisible = m_verticalScrollBarMode == GuiScrollBarMode::Always;
    for (uint32_t passIndex = 0; passIndex < 2; ++passIndex)
    {
        const float viewportWidth = (std::max)(0.0f, _rect.m_size.m_x - (m_isVerticalScrollBarVisible ? ScrollBarThickness : 0.0f));
        const float viewportHeight = (std::max)(0.0f, _rect.m_size.m_y - (m_isHorizontalScrollBarVisible ? ScrollBarThickness : 0.0f));

        if (m_horizontalScrollBarMode == GuiScrollBarMode::Auto && contentDesiredSize.m_x > viewportWidth)
        {
            m_isHorizontalScrollBarVisible = true;
        }
        if (m_verticalScrollBarMode == GuiScrollBarMode::Auto && contentDesiredSize.m_y > viewportHeight)
        {
            m_isVerticalScrollBarVisible = true;
        }
    }

    m_viewportRect = GuiRect(
        _rect.m_position,
        GuiSize(
            (std::max)(0.0f, _rect.m_size.m_x - (m_isVerticalScrollBarVisible ? ScrollBarThickness : 0.0f)),
            (std::max)(0.0f, _rect.m_size.m_y - (m_isHorizontalScrollBarVisible ? ScrollBarThickness : 0.0f))));

    m_contentSize = GuiSize(
        m_horizontalScrollBarMode == GuiScrollBarMode::Disabled ? m_viewportRect.m_size.m_x : (std::max)(m_viewportRect.m_size.m_x, contentDesiredSize.m_x),
        m_verticalScrollBarMode == GuiScrollBarMode::Disabled ? m_viewportRect.m_size.m_y : (std::max)(m_viewportRect.m_size.m_y, contentDesiredSize.m_y));
    m_scrollRange = GuiSize((std::max)(0.0f, m_contentSize.m_x - m_viewportRect.m_size.m_x), (std::max)(0.0f, m_contentSize.m_y - m_viewportRect.m_size.m_y));
    m_scrollOffset = GuiPosition((std::clamp)(m_scrollOffset.m_x, 0.0f, m_scrollRange.m_x), (std::clamp)(m_scrollOffset.m_y, 0.0f, m_scrollRange.m_y));

    m_horizontalTrackRect =
        m_isHorizontalScrollBarVisible ? GuiRect(m_viewportRect.m_position.m_x, m_viewportRect.getBottom(), m_viewportRect.m_size.m_x, ScrollBarThickness) : GuiRect();
    m_verticalTrackRect =
        m_isVerticalScrollBarVisible ? GuiRect(m_viewportRect.getRight(), m_viewportRect.m_position.m_y, ScrollBarThickness, m_viewportRect.m_size.m_y) : GuiRect();
    updateScrollBarRects();
}

void ego::gui::GuiScrollBox::updateScrollBarRects()
{
    m_horizontalThumbRect =
        m_isHorizontalScrollBarVisible ?
            CreateScrollThumbRect(m_horizontalTrackRect, m_viewportRect.m_size.m_x, m_contentSize.m_x, m_scrollOffset.m_x, m_scrollRange.m_x, Axis::Horizontal) :
            GuiRect();
    m_verticalThumbRect = m_isVerticalScrollBarVisible ?
                              CreateScrollThumbRect(m_verticalTrackRect, m_viewportRect.m_size.m_y, m_contentSize.m_y, m_scrollOffset.m_y, m_scrollRange.m_y, Axis::Vertical) :
                              GuiRect();
}
