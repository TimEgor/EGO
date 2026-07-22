#pragma once

#include <functional>
#include <string>

#include "EgoCore/Callback/StableCallback.h"
#include "EgoCore/Patterns/NonInstanceable.h"

#include "EgoGui/Widgets/Container.h"

namespace ego::gui
{
    struct WindowStyle;

    class Window;
    EGO_POINTER(Window);
    EGO_WEAK_POINTER(Window);

    class WindowDragDelegate
    {
    public:
        virtual ~WindowDragDelegate();

        virtual void onWindowDragBegin(Window& _window, const Position& _position) = 0;
        virtual void onWindowDragMove(Window& _window, const Position& _position) = 0;
        virtual void onWindowDragEnd(Window& _window, const Position& _position) = 0;
        virtual void onWindowDragCancel(Window& _window, const Position& _position) = 0;
    };

    EGO_POINTER(WindowDragDelegate);
    EGO_WEAK_POINTER(WindowDragDelegate);

    using WindowSizeChangedHandler = std::function<void(const Size&)>;

    struct WindowDesc final
    {
        std::string m_title;
        Rect m_bounds;
        WidgetPointer m_content;
        WindowSizeChangedHandler m_onSizeChanged;
    };

    class Window final
        : public Container
    {
    public:
        class WindowAccessor final
            : public NonInstanceable
        {
        public:
            static void BindDragDelegate(Window& _window, const WindowDragDelegateWeakPointer& _delegate);
            static void SetDockedBounds(Window& _window, const Rect& _bounds, bool _isVisible);
            static void ClearDockedBounds(Window& _window);
            static const Rect& GetFloatingBounds(const Window& _window);
            static bool FlushSizeChanged(Window& _window);
        };

        ~Window() override;

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

        EGO_RTTI_VIRTUAL(Window, Container);

    protected:
        InputReply onPointerMove(const PointerMoveEvent& _event) override;
        InputReply onMouseButton(const MouseButtonEvent& _event) override;
        void onPointerEnter(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerLeave(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerCaptureLost(const Position& _position) override;
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;
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

        enum class Presentation
        {
            Floating,
            Docked
        };

        PointerRegion hitTest(const Position& _position) const;
        void updateHoveredRegion(const Position& _position);

        void beginInteraction(InteractionState _state, const Position& _position);
        void pauseInteraction();
        void updateInteraction(const Position& _position);
        void endInteraction(const Position& _position);
        void cancelInteraction(const Position& _position);
        void bindDragDelegate(const WindowDragDelegateWeakPointer& _delegate);
        void setDockedBounds(const Rect& _bounds, bool _isVisible);
        void clearDockedBounds();
        bool isDocked() const;
        void applyWindowStyle(const WindowStyle& _style);
        bool updateFloatingSize();
        void applyUserSize(const Size& _size);
        void updateWindowRects();
        void notifySizeChanged();
        bool flushSizeChanged();

        size_t getChildCount() const override;
        const WidgetPointer& getChild(size_t _index) const override;
        Rect resolveTopLevelBounds(const Rect& _surfaceBounds) const override;

        std::string m_title;
        Rect m_floatingBounds;
        Rect m_dockedBounds;
        Size m_userSize = SizeZero;
        Size m_minimumSize = SizeZero;
        Size m_themeMinimumSize = SizeZero;
        WidgetPointer m_content = nullptr;
        Rect m_titleBarRect;
        Rect m_contentRect;
        Rect m_resizeGripRect;
        Position m_interactionStartPosition = PositionZero;
        Rect m_interactionStartBounds;
        InteractionState m_interactionState = InteractionState::None;
        PointerRegion m_hoveredRegion = PointerRegion::None;
        Presentation m_presentation = Presentation::Floating;
        float m_titleBarHeight = 0.0f;
        float m_resizeGripSize = 0.0f;
        float m_windowBorderThickness = 0.0f;
        bool m_hasUserSize = false;
        bool m_hasMinimumSizeOverride = false;
        bool m_isInteractionPaused = false;
        bool m_isDockedVisible = false;
        bool m_isSizeChangedPending = false;
        StableCallback<const Size&> m_onSizeChanged;
        WindowDragDelegateWeakPointer m_dragDelegate;
        WindowDragDelegateWeakPointer m_activeDragDelegate;
    };
} // namespace ego::gui
