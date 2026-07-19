#include "ScrollView.h"

#include <algorithm>
#include <cstdint>
#include <utility>

#include "EgoGui/Theme/Theme.h"

ego::gui::ScrollView::ScrollView()
{
    const ScrollStyle& scroll = Theme::GetDefault().m_scroll;
    m_scrollBarThickness = (std::max)(0.0f, scroll.m_thickness);
    m_scrollBarMinimumThumbSize = (std::max)(0.0f, scroll.m_minimumThumbSize);
    m_wheelStep = (std::max)(0.0f, scroll.m_wheelStep);
}

ego::gui::ScrollViewPointer ego::gui::ScrollView::Create()
{
    return new ScrollView();
}

ego::gui::ScrollView::~ScrollView()
{
    setContent(nullptr);
}

ego::gui::ScrollViewPointer ego::gui::ScrollView::Create(ScrollViewDesc _desc)
{
    const ScrollViewPointer scrollBox = Create();
    scrollBox->setHorizontalScrollBarMode(_desc.m_horizontalMode);
    scrollBox->setVerticalScrollBarMode(_desc.m_verticalMode);
    scrollBox->onChange(std::move(_desc.m_onChanged));
    if (!scrollBox->setContent(_desc.m_content))
    {
        return nullptr;
    }

    return scrollBox;
}

bool ego::gui::ScrollView::setContent(const WidgetPointer& _content)
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

    m_scrollOffset = PositionZero;
    m_scrollRange = SizeZero;
    endScrollInteraction();

    if (previousContent)
    {
        detachChild(previousContent);
    }

    notifyTreeChanged();
    return true;
}

ego::gui::WidgetPointer ego::gui::ScrollView::getContent() const
{
    return m_content;
}

size_t ego::gui::ScrollView::getChildCount() const
{
    return m_content ? 1 : 0;
}

const ego::gui::WidgetPointer& ego::gui::ScrollView::getChild(size_t) const
{
    return m_content;
}

void ego::gui::ScrollView::setHorizontalScrollBarMode(ScrollBarMode _mode)
{
    if (m_horizontalScrollBarMode == _mode)
    {
        return;
    }

    m_horizontalScrollBarMode = _mode;
    if (_mode == ScrollBarMode::Disabled)
    {
        m_scrollOffset.m_x = 0.0f;
        m_scrollRange.m_x = 0.0f;
        m_isHorizontalScrollBarVisible = false;
    }
    invalidateLayout();
}

ego::gui::ScrollBarMode ego::gui::ScrollView::getHorizontalScrollBarMode() const
{
    return m_horizontalScrollBarMode;
}

void ego::gui::ScrollView::setVerticalScrollBarMode(ScrollBarMode _mode)
{
    if (m_verticalScrollBarMode == _mode)
    {
        return;
    }

    m_verticalScrollBarMode = _mode;
    if (_mode == ScrollBarMode::Disabled)
    {
        m_scrollOffset.m_y = 0.0f;
        m_scrollRange.m_y = 0.0f;
        m_isVerticalScrollBarVisible = false;
    }
    invalidateLayout();
}

ego::gui::ScrollBarMode ego::gui::ScrollView::getVerticalScrollBarMode() const
{
    return m_verticalScrollBarMode;
}

void ego::gui::ScrollView::setScrollOffset(const Position& _offset)
{
    const Position scrollOffset((std::clamp)(_offset.m_x, 0.0f, m_scrollRange.m_x), (std::clamp)(_offset.m_y, 0.0f, m_scrollRange.m_y));
    if (m_scrollOffset.m_x == scrollOffset.m_x && m_scrollOffset.m_y == scrollOffset.m_y)
    {
        return;
    }

    m_scrollOffset = scrollOffset;
    updateScrollBarRects();
    invalidateLayout();
}

const ego::gui::Position& ego::gui::ScrollView::getScrollOffset() const
{
    return m_scrollOffset;
}

const ego::gui::Size& ego::gui::ScrollView::getScrollRange() const
{
    return m_scrollRange;
}

void ego::gui::ScrollView::onChange(ScrollChangedHandler _handler)
{
    m_onScrollChanged.set(std::move(_handler));
}

ego::gui::InputReply ego::gui::ScrollView::onPointerMove(const PointerMoveEvent& _event)
{
    if (m_scrollInteraction != ScrollPart::None)
    {
        m_hoveredScrollPart = findScrollPart(_event.m_position);
        updateScrollInteraction(_event.m_position);
        return InputReply::Handled;
    }

    m_hoveredScrollPart = findScrollPart(_event.m_position);
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::ScrollView::onMouseButton(const MouseButtonEvent& _event)
{
    if (_event.m_action == InputButtonAction::Pressed && _event.m_key == MouseInputKey::ButtonLeft)
    {
        const ScrollPart scrollPart = findScrollPart(_event.m_position);
        m_hoveredScrollPart = scrollPart;
        if (scrollPart != ScrollPart::None && m_scrollInteraction == ScrollPart::None)
        {
            beginScrollInteraction(scrollPart, _event.m_position);
            return InputReply::FocusAndCapture;
        }
    }

    if (_event.m_action == InputButtonAction::Released && _event.m_key == MouseInputKey::ButtonLeft && m_scrollInteraction != ScrollPart::None)
    {
        updateScrollInteraction(_event.m_position);
        endScrollInteraction();
        m_hoveredScrollPart = findScrollPart(_event.m_position);
        return InputReply::Handled;
    }

    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::ScrollView::onMouseWheel(const MouseWheelEvent& _event)
{
    if (getLayoutBounds().contains(_event.m_position) && applyMouseWheel(_event.m_wheelDelta))
    {
        return InputReply::Handled;
    }

    return InputReply::Unhandled;
}

void ego::gui::ScrollView::onPointerEnter(const Position& _position, const InputModifiers&)
{
    m_hoveredScrollPart = findScrollPart(_position);
}

void ego::gui::ScrollView::onPointerLeave(const Position&, const InputModifiers&)
{
    m_hoveredScrollPart = ScrollPart::None;
}

void ego::gui::ScrollView::onPointerCaptureLost(const Position&)
{
    m_hoveredScrollPart = ScrollPart::None;
    endScrollInteraction();
}

ego::gui::Size ego::gui::ScrollView::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    const Size& maximumSize = _constraints.m_maximumSize;
    const float scrollBarThickness = (std::max)(0.0f, _context.getTheme().m_scroll.m_thickness);
    const WidgetPointer content = getContent();
    if (!content)
    {
        return SizeZero;
    }

    const Size contentAvailableSize(
        m_horizontalScrollBarMode == ScrollBarMode::Disabled ? maximumSize.m_x : UnboundedLayoutExtent,
        m_verticalScrollBarMode == ScrollBarMode::Disabled ? maximumSize.m_y : UnboundedLayoutExtent);
    const Size contentPreferredSize = content->updatePreferredSize(_context, LayoutConstraints(contentAvailableSize));

    bool isHorizontalScrollBarVisible = false;
    bool isVerticalScrollBarVisible = false;
    resolveScrollBarVisibility(maximumSize, contentPreferredSize, scrollBarThickness, isHorizontalScrollBarVisible, isVerticalScrollBarVisible);

    const float verticalScrollBarExtent = isVerticalScrollBarVisible ? scrollBarThickness : 0.0f;
    const float horizontalScrollBarExtent = isHorizontalScrollBarVisible ? scrollBarThickness : 0.0f;
    const float availableViewportWidth = (std::max)(0.0f, maximumSize.m_x - verticalScrollBarExtent);
    const float availableViewportHeight = (std::max)(0.0f, maximumSize.m_y - horizontalScrollBarExtent);
    const float desiredViewportWidth =
        m_horizontalScrollBarMode == ScrollBarMode::Disabled ? contentPreferredSize.m_x : (std::min)(contentPreferredSize.m_x, availableViewportWidth);
    const float desiredViewportHeight =
        m_verticalScrollBarMode == ScrollBarMode::Disabled ? contentPreferredSize.m_y : (std::min)(contentPreferredSize.m_y, availableViewportHeight);

    return Size(desiredViewportWidth + verticalScrollBarExtent, desiredViewportHeight + horizontalScrollBarExtent);
}

void ego::gui::ScrollView::updateGeometry(const LayoutContext& _context)
{
    const ScrollStyle& scroll = _context.getTheme().m_scroll;
    const float scrollBarThickness = (std::max)(0.0f, scroll.m_thickness);
    const float scrollBarMinimumThumbSize = (std::max)(0.0f, scroll.m_minimumThumbSize);
    const Rect previousHorizontalTrackRect = m_horizontalTrackRect;
    const Rect previousHorizontalThumbRect = m_horizontalThumbRect;
    const Rect previousVerticalTrackRect = m_verticalTrackRect;
    const Rect previousVerticalThumbRect = m_verticalThumbRect;
    const Size previousScrollRange = m_scrollRange;
    m_scrollBarThickness = scrollBarThickness;
    m_scrollBarMinimumThumbSize = scrollBarMinimumThumbSize;
    m_wheelStep = (std::max)(0.0f, scroll.m_wheelStep);
    updateScrollLayout(getLayoutBounds());
    const bool horizontalGeometryChanged = !AreRectsEqual(previousHorizontalTrackRect, m_horizontalTrackRect) ||
                                           !AreRectsEqual(previousHorizontalThumbRect, m_horizontalThumbRect) || previousScrollRange.m_x != m_scrollRange.m_x;
    const bool verticalGeometryChanged = !AreRectsEqual(previousVerticalTrackRect, m_verticalTrackRect) || !AreRectsEqual(previousVerticalThumbRect, m_verticalThumbRect) ||
                                         previousScrollRange.m_y != m_scrollRange.m_y;
    const bool activeThumbGeometryChanged =
        (m_scrollInteraction == ScrollPart::HorizontalThumb && horizontalGeometryChanged) || (m_scrollInteraction == ScrollPart::VerticalThumb && verticalGeometryChanged);
    if (activeThumbGeometryChanged)
    {
        m_interactionStartPosition = m_interactionCurrentPosition;
        m_interactionStartOffset = m_scrollOffset;
    }

    const WidgetPointer content = getContent();
    if (content)
    {
        const Rect contentRect(m_viewportRect.m_position.m_x - m_scrollOffset.m_x, m_viewportRect.m_position.m_y - m_scrollOffset.m_y, m_contentSize.m_x, m_contentSize.m_y);
        content->applyLayout(_context, contentRect);
    }
}

void ego::gui::ScrollView::drawOverlayLayer(PaintContext& _context) const
{
    const ScrollStyle& scroll = _context.getTheme().m_scroll;

    if (m_isHorizontalScrollBarVisible)
    {
        _context.drawBox(m_horizontalTrackRect, scroll.m_track);

        const NormalizedColorRGBA& thumbColor = scroll.m_thumb.resolve(m_hoveredScrollPart == ScrollPart::HorizontalThumb, m_scrollInteraction == ScrollPart::HorizontalThumb);
        _context.drawBox(m_horizontalThumbRect, thumbColor);
    }

    if (m_isVerticalScrollBarVisible)
    {
        _context.drawBox(m_verticalTrackRect, scroll.m_track);

        const NormalizedColorRGBA& thumbColor = scroll.m_thumb.resolve(m_hoveredScrollPart == ScrollPart::VerticalThumb, m_scrollInteraction == ScrollPart::VerticalThumb);
        _context.drawBox(m_verticalThumbRect, thumbColor);
    }

    if (m_isHorizontalScrollBarVisible && m_isVerticalScrollBarVisible)
    {
        _context.drawBox(Rect(m_viewportRect.getRight(), m_viewportRect.getBottom(), m_scrollBarThickness, m_scrollBarThickness), scroll.m_track);
    }
}

bool ego::gui::ScrollView::clipsChildren() const
{
    return true;
}

ego::gui::Rect ego::gui::ScrollView::getChildrenClipRect() const
{
    return m_viewportRect;
}

bool ego::gui::ScrollView::isChildHitTestVisible(const Position& _position) const
{
    return m_viewportRect.contains(_position);
}

bool ego::gui::ScrollView::AreRectsEqual(const Rect& _first, const Rect& _second)
{
    return _first.m_position.m_x == _second.m_position.m_x && _first.m_position.m_y == _second.m_position.m_y && _first.m_size.m_x == _second.m_size.m_x &&
           _first.m_size.m_y == _second.m_size.m_y;
}

ego::gui::Rect ego::gui::ScrollView::createScrollThumbRect(const Rect& _trackRect, float _viewportExtent, float _contentExtent, float _offset, float _range, Axis _axis) const
{
    const float trackExtent = _axis == Axis::Horizontal ? _trackRect.m_size.m_x : _trackRect.m_size.m_y;
    if (trackExtent <= 0.0f)
    {
        return Rect(_trackRect.m_position, SizeZero);
    }

    const float visibleRatio = _contentExtent > 0.0f ? (std::min)(1.0f, _viewportExtent / _contentExtent) : 1.0f;
    const float thumbExtent = (std::min)(trackExtent, (std::max)(m_scrollBarMinimumThumbSize, trackExtent * visibleRatio));
    const float thumbTravel = trackExtent - thumbExtent;
    const float thumbOffset = _range > 0.0f ? thumbTravel * _offset / _range : 0.0f;

    if (_axis == Axis::Horizontal)
    {
        return Rect(_trackRect.m_position.m_x + thumbOffset, _trackRect.m_position.m_y, thumbExtent, _trackRect.m_size.m_y);
    }

    return Rect(_trackRect.m_position.m_x, _trackRect.m_position.m_y + thumbOffset, _trackRect.m_size.m_x, thumbExtent);
}

void ego::gui::ScrollView::resolveScrollBarVisibility(
    const Size& _outerSize,
    const Size& _contentSize,
    float _scrollBarThickness,
    bool& _isHorizontalVisible,
    bool& _isVerticalVisible) const
{
    _isHorizontalVisible = m_horizontalScrollBarMode == ScrollBarMode::Always;
    _isVerticalVisible = m_verticalScrollBarMode == ScrollBarMode::Always;
    for (uint32_t passIndex = 0; passIndex < 2; ++passIndex)
    {
        const float viewportWidth = (std::max)(0.0f, _outerSize.m_x - (_isVerticalVisible ? _scrollBarThickness : 0.0f));
        const float viewportHeight = (std::max)(0.0f, _outerSize.m_y - (_isHorizontalVisible ? _scrollBarThickness : 0.0f));

        if (m_horizontalScrollBarMode == ScrollBarMode::Auto && _contentSize.m_x > viewportWidth)
        {
            _isHorizontalVisible = true;
        }
        if (m_verticalScrollBarMode == ScrollBarMode::Auto && _contentSize.m_y > viewportHeight)
        {
            _isVerticalVisible = true;
        }
    }
}

ego::gui::ScrollView::ScrollPart ego::gui::ScrollView::findScrollPart(const Position& _position) const
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

void ego::gui::ScrollView::beginScrollInteraction(ScrollPart _part, const Position& _position)
{
    m_scrollInteraction = _part;
    m_interactionStartPosition = _position;
    m_interactionCurrentPosition = _position;
    m_interactionStartOffset = m_scrollOffset;

    Position offset = m_scrollOffset;
    if (_part == ScrollPart::HorizontalTrack)
    {
        offset.m_x += _position.m_x < m_horizontalThumbRect.getLeft() ? -m_viewportRect.m_size.m_x : m_viewportRect.m_size.m_x;
    }
    else if (_part == ScrollPart::VerticalTrack)
    {
        offset.m_y += _position.m_y < m_verticalThumbRect.getTop() ? -m_viewportRect.m_size.m_y : m_viewportRect.m_size.m_y;
    }
    applyUserScrollOffset(offset);
}

void ego::gui::ScrollView::updateScrollInteraction(const Position& _position)
{
    m_interactionCurrentPosition = _position;
    if (m_scrollInteraction != ScrollPart::HorizontalThumb && m_scrollInteraction != ScrollPart::VerticalThumb)
    {
        return;
    }

    Position offset = m_interactionStartOffset;
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
    applyUserScrollOffset(offset);
}

void ego::gui::ScrollView::endScrollInteraction()
{
    m_scrollInteraction = ScrollPart::None;
    m_interactionStartPosition = PositionZero;
    m_interactionCurrentPosition = PositionZero;
    m_interactionStartOffset = PositionZero;
}

bool ego::gui::ScrollView::applyMouseWheel(float _delta)
{
    if (_delta == 0.0f)
    {
        return false;
    }

    Position offset = m_scrollOffset;
    if (m_scrollRange.m_y > 0.0f)
    {
        offset.m_y -= _delta * m_wheelStep;
    }
    else if (m_scrollRange.m_x > 0.0f)
    {
        offset.m_x -= _delta * m_wheelStep;
    }

    return applyUserScrollOffset(offset);
}

bool ego::gui::ScrollView::applyUserScrollOffset(const Position& _offset)
{
    const Position previousOffset = m_scrollOffset;
    setScrollOffset(_offset);
    const bool changed = previousOffset.m_x != m_scrollOffset.m_x || previousOffset.m_y != m_scrollOffset.m_y;
    if (changed)
    {
        notifyScrollChanged();
    }

    return changed;
}

void ego::gui::ScrollView::updateScrollLayout(const Rect& _rect)
{
    const WidgetPointer content = getContent();
    const Size contentPreferredSize = content ? content->getPreferredSize() : SizeZero;

    resolveScrollBarVisibility(_rect.m_size, contentPreferredSize, m_scrollBarThickness, m_isHorizontalScrollBarVisible, m_isVerticalScrollBarVisible);

    m_viewportRect = Rect(
        _rect.m_position,
        Size(
            (std::max)(0.0f, _rect.m_size.m_x - (m_isVerticalScrollBarVisible ? m_scrollBarThickness : 0.0f)),
            (std::max)(0.0f, _rect.m_size.m_y - (m_isHorizontalScrollBarVisible ? m_scrollBarThickness : 0.0f))));

    m_contentSize = Size(
        m_horizontalScrollBarMode == ScrollBarMode::Disabled ? m_viewportRect.m_size.m_x : (std::max)(m_viewportRect.m_size.m_x, contentPreferredSize.m_x),
        m_verticalScrollBarMode == ScrollBarMode::Disabled ? m_viewportRect.m_size.m_y : (std::max)(m_viewportRect.m_size.m_y, contentPreferredSize.m_y));
    m_scrollRange = Size((std::max)(0.0f, m_contentSize.m_x - m_viewportRect.m_size.m_x), (std::max)(0.0f, m_contentSize.m_y - m_viewportRect.m_size.m_y));
    m_scrollOffset = Position((std::clamp)(m_scrollOffset.m_x, 0.0f, m_scrollRange.m_x), (std::clamp)(m_scrollOffset.m_y, 0.0f, m_scrollRange.m_y));

    m_horizontalTrackRect =
        m_isHorizontalScrollBarVisible ? Rect(m_viewportRect.m_position.m_x, m_viewportRect.getBottom(), m_viewportRect.m_size.m_x, m_scrollBarThickness) : Rect();
    m_verticalTrackRect = m_isVerticalScrollBarVisible ? Rect(m_viewportRect.getRight(), m_viewportRect.m_position.m_y, m_scrollBarThickness, m_viewportRect.m_size.m_y) : Rect();
    updateScrollBarRects();
}

void ego::gui::ScrollView::updateScrollBarRects()
{
    m_horizontalThumbRect =
        m_isHorizontalScrollBarVisible ?
            createScrollThumbRect(m_horizontalTrackRect, m_viewportRect.m_size.m_x, m_contentSize.m_x, m_scrollOffset.m_x, m_scrollRange.m_x, Axis::Horizontal) :
            Rect();
    m_verticalThumbRect = m_isVerticalScrollBarVisible ?
                              createScrollThumbRect(m_verticalTrackRect, m_viewportRect.m_size.m_y, m_contentSize.m_y, m_scrollOffset.m_y, m_scrollRange.m_y, Axis::Vertical) :
                              Rect();
}

void ego::gui::ScrollView::notifyScrollChanged()
{
    const Position offset = m_scrollOffset;
    m_onScrollChanged.invoke(offset);
}
