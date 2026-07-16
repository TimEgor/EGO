#pragma once

#include "EgoGui/Widgets/GuiContainer.h"

namespace ego::gui
{
    enum class GuiScrollBarMode
    {
        Disabled,
        Auto,
        Always
    };

    class GuiScrollBox;
    EGO_POINTER(GuiScrollBox);

    class GuiScrollBox final : public GuiContainer
    {
    public:
        GuiScrollBox() = default;

        static GuiScrollBoxPointer Create();

        void setContent(const GuiWidgetPointer& _content);
        GuiWidgetPointer getContent() const;

        void setHorizontalScrollBarMode(GuiScrollBarMode _mode);
        GuiScrollBarMode getHorizontalScrollBarMode() const;
        void setVerticalScrollBarMode(GuiScrollBarMode _mode);
        GuiScrollBarMode getVerticalScrollBarMode() const;

        void setScrollOffset(const GuiPosition& _offset);
        const GuiPosition& getScrollOffset() const;
        const GuiSize& getScrollRange() const;

        EGO_RTTI_VIRTUAL(GuiScrollBox, GuiContainer);

    protected:
        GuiEventResult onEvent(const GuiInputEvent& _event) override;
        GuiSize onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize) override;
        void onArrange(const GuiLayoutContext& _context, const GuiRect& _rect) override;
        void onPaint(GuiPaintContext& _context) const override;
        bool isChildHitTestVisible(const GuiPosition& _position) const override;

    private:
        enum class Axis
        {
            Horizontal,
            Vertical
        };

        enum class ScrollPart
        {
            None,
            HorizontalTrack,
            HorizontalThumb,
            VerticalTrack,
            VerticalThumb
        };

        static GuiRect CreateScrollThumbRect(const GuiRect& _trackRect, float _viewportExtent, float _contentExtent, float _offset, float _range, Axis _axis);

        ScrollPart findScrollPart(const GuiPosition& _position) const;
        void beginScrollInteraction(ScrollPart _part, const GuiPosition& _position);
        void updateScrollInteraction(const GuiPosition& _position);
        void endScrollInteraction();
        bool applyMouseWheel(float _delta);

        void updateScrollLayout(const GuiRect& _rect);
        void updateScrollBarRects();

        GuiScrollBarMode m_horizontalScrollBarMode = GuiScrollBarMode::Disabled;
        GuiScrollBarMode m_verticalScrollBarMode = GuiScrollBarMode::Auto;
        GuiPosition m_scrollOffset = GuiPositionZero;
        GuiSize m_scrollRange = GuiSizeZero;
        GuiSize m_contentSize = GuiSizeZero;
        GuiRect m_viewportRect;
        GuiRect m_horizontalTrackRect;
        GuiRect m_horizontalThumbRect;
        GuiRect m_verticalTrackRect;
        GuiRect m_verticalThumbRect;
        GuiPosition m_interactionStartPosition = GuiPositionZero;
        GuiPosition m_interactionStartOffset = GuiPositionZero;
        ScrollPart m_hoveredScrollPart = ScrollPart::None;
        ScrollPart m_scrollInteraction = ScrollPart::None;
        bool m_isHorizontalScrollBarVisible = false;
        bool m_isVerticalScrollBarVisible = false;
    };
} // namespace ego::gui
