#pragma once

#include <cstddef>
#include <functional>

#include "EgoCore/Callback/StableCallback.h"

#include "EgoGui/Widgets/Container.h"

namespace ego::gui
{
    struct ScrollStyle;

    enum class ScrollBarMode
    {
        Disabled,
        Auto,
        Always
    };

    class ScrollView;
    EGO_POINTER(ScrollView);

    using ScrollChangedHandler = std::function<void(const Position&)>;

    struct ScrollViewDesc final
    {
        WidgetPointer m_content;
        ScrollBarMode m_horizontalMode = ScrollBarMode::Disabled;
        ScrollBarMode m_verticalMode = ScrollBarMode::Auto;
        ScrollChangedHandler m_onChanged;
    };

    class ScrollView final : public Container
    {
    public:
        ~ScrollView() override = default;

        static ScrollViewPointer Create();
        static ScrollViewPointer Create(ScrollViewDesc _desc);

        bool setContent(const WidgetPointer& _content);
        WidgetPointer getContent() const;

        void setHorizontalScrollBarMode(ScrollBarMode _mode);
        ScrollBarMode getHorizontalScrollBarMode() const;
        void setVerticalScrollBarMode(ScrollBarMode _mode);
        ScrollBarMode getVerticalScrollBarMode() const;

        void setScrollOffset(const Position& _offset);
        const Position& getScrollOffset() const;
        const Size& getScrollRange() const;
        void onChange(ScrollChangedHandler _handler);

        EGO_RTTI_VIRTUAL(ScrollView, Container);

    protected:
        InputReply onPointerMove(InputContext& _context, const PointerMoveEvent& _event) override;
        InputReply onMouseButton(InputContext& _context, const MouseButtonEvent& _event) override;
        InputReply onMouseWheel(InputContext& _context, const MouseWheelEvent& _event) override;
        void onPointerEnter(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerLeave(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerCaptureLost(const Position& _position) override;
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;
        void drawOverlayLayer(PaintContext& _context) const override;
        bool clipsChildren() const override;
        Rect getChildrenClipRect() const override;
        bool isChildHitTestVisible(const Position& _position) const override;

    private:
        ScrollView();

        size_t getChildCount() const override;
        WidgetPointer getChild(size_t _index) const override;

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

        Rect createScrollThumbRect(const Rect& _trackRect, float _viewportExtent, float _contentExtent, float _offset, float _range, Axis _axis) const;
        void resolveScrollBarVisibility(
            const Size& _outerSize,
            const Size& _contentSize,
            float _scrollBarThickness,
            bool& _isHorizontalVisible,
            bool& _isVerticalVisible) const;

        ScrollPart findScrollPart(const Position& _position) const;
        void beginScrollInteraction(ScrollPart _part, const Position& _position);
        void updateScrollInteraction(const Position& _position);
        void endScrollInteraction();
        bool applyMouseWheel(float _delta);
        bool applyUserScrollOffset(const Position& _offset);

        void updateScrollLayout(const Rect& _rect);
        void updateScrollBarRects();
        void notifyScrollChanged();

        WidgetPointer m_content;
        ScrollBarMode m_horizontalScrollBarMode = ScrollBarMode::Disabled;
        ScrollBarMode m_verticalScrollBarMode = ScrollBarMode::Auto;
        Position m_scrollOffset = PositionZero;
        Size m_scrollRange = SizeZero;
        Size m_contentSize = SizeZero;
        Rect m_viewportRect;
        Rect m_horizontalTrackRect;
        Rect m_horizontalThumbRect;
        Rect m_verticalTrackRect;
        Rect m_verticalThumbRect;
        Position m_interactionStartPosition = PositionZero;
        Position m_interactionCurrentPosition = PositionZero;
        Position m_interactionStartOffset = PositionZero;
        ScrollPart m_hoveredScrollPart = ScrollPart::None;
        ScrollPart m_scrollInteraction = ScrollPart::None;
        float m_scrollBarThickness = 0.0f;
        float m_scrollBarMinimumThumbSize = 0.0f;
        float m_wheelStep = 0.0f;
        bool m_isHorizontalScrollBarVisible = false;
        bool m_isVerticalScrollBarVisible = false;
        StableCallback<const Position&> m_onScrollChanged;
    };
} // namespace ego::gui
