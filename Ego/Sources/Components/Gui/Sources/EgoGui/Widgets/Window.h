#pragma once

#include <functional>
#include <string>

#include "EgoCore/Callback/StableCallback.h"

#include "EgoGui/Widgets/Container.h"

namespace ego::gui
{
    struct WindowStyle;

    class Window;
    EGO_POINTER(Window);
    EGO_WEAK_POINTER(Window);

    using WindowSizeChangedHandler = std::function<void(const Size&)>;

    struct WindowDesc final
    {
        std::string m_title;
        Rect m_bounds;
        WidgetPointer m_content;
        WindowSizeChangedHandler m_onSizeChanged;
    };

    class Window final : public Container
    {
    public:
        ~Window() override = default;

        static WindowPointer Create();
        static WindowPointer Create(WindowDesc _desc);

        void setTitle(std::string _title);
        const std::string& getTitle() const;

        void setPosition(const Position& _position);
        const Position& getPosition() const;

        void setSize(const Size& _size);
        const Size& getSize() const;
        void onSizeChange(WindowSizeChangedHandler _handler);

        void setMinimumSize(const Size& _size);
        void clearMinimumSize();
        const Size& getMinimumSize() const;

        bool setContent(const WidgetPointer& _content);
        WidgetPointer getContent() const;

        void setBounds(const Rect& _bounds);
        const Rect& getBounds() const;
        const Rect& getFloatingBounds() const;
        bool isDocked() const;

        EGO_RTTI_VIRTUAL(Window, Container);

    protected:
        InputReply onPointerMove(WidgetUpdateContext& _context, const PointerMoveEvent& _event) override;
        InputReply onMouseButton(WidgetUpdateContext& _context, const MouseButtonEvent& _event) override;
        void onPointerEnter(WidgetUpdateContext& _context, const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerLeave(WidgetUpdateContext& _context, const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerCaptureLost(WidgetUpdateContext& _context, const Position& _position) override;

        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;
        void onLayoutCompleted() override;
        void drawBaseLayer(PaintContext& _context) const override;
        void drawOverlayLayer(PaintContext& _context) const override;
        bool clipsChildren() const override;
        Rect getChildrenClipRect() const override;
        bool isChildHitTestVisible(const Position& _position) const override;

    private:
        Window();

        enum class InteractionState
        {
            None,
            Dragging,
            Resizing
        };

        enum class PointerRegion
        {
            None,
            Content,
            TitleBar,
            ResizeGrip
        };

        PointerRegion resolvePointerRegion(const Position& _position) const;
        void updateHoveredRegion(const Position& _position);

        void beginInteraction(WidgetUpdateContext& _context, InteractionState _state, const Position& _position);
        void pauseInteraction();
        void updateInteraction(WidgetUpdateContext& _context, const Position& _position);
        void endInteraction(WidgetUpdateContext& _context, const Position& _position);
        void cancelInteraction(WidgetUpdateContext& _context, const Position& _position);
        void applyWindowStyle(const WindowStyle& _style);
        bool updateFloatingSize();
        void applyUserSize(const Size& _size);
        void updateWindowRects();
        void notifySizeChanged();

        size_t getChildCount() const override;
        WidgetPointer getChild(size_t _index) const override;

        std::string m_title;
        Rect m_floatingBounds;
        Size m_userSize = SizeZero;
        Size m_minimumSize = SizeZero;
        Size m_themeMinimumSize = SizeZero;
        Size m_reportedSize = SizeZero;
        WidgetPointer m_content = nullptr;
        Rect m_titleBarRect;
        Rect m_contentRect;
        Rect m_resizeGripRect;
        Position m_interactionStartPosition = PositionZero;
        Rect m_interactionStartBounds;
        InteractionState m_interactionState = InteractionState::None;
        PointerRegion m_hoveredRegion = PointerRegion::None;
        float m_titleBarHeight = 0.0f;
        float m_resizeGripSize = 0.0f;
        float m_windowBorderThickness = 0.0f;
        bool m_hasUserSize = false;
        bool m_hasMinimumSizeOverride = false;
        bool m_isInteractionPaused = false;
        StableCallback<const Size&> m_onSizeChanged;
    };
} // namespace ego::gui
