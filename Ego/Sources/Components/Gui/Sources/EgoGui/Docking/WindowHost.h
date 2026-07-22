#pragma once

#include <cstddef>
#include <vector>

#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Theme/Theme.h"
#include "EgoGui/Widgets/Window.h"

#include "DockingLayout.h"
#include "DockingPreview.h"
#include "DockingTypes.h"

namespace ego::gui
{
    class SurfaceRoot;
    EGO_POINTER(SurfaceRoot);
    EGO_WEAK_POINTER(SurfaceRoot);

    class WindowHost;
    EGO_POINTER(WindowHost);

    class WindowHost final : public Container, public WindowDragDelegate
    {
    public:
        class WindowHostAccessor final : public NonInstanceable
        {
        public:
            static bool IsInteractionActive(const WindowHost& _host);
        };

        using WidgetCollection = std::vector<WidgetPointer>;

        ~WindowHost() override;

        static WindowHostPointer Create(const SurfaceRootPointer& _root);

        bool addWidget(const WidgetPointer& _widget);
        WidgetPointer removeWidget(const WidgetPointer& _widget);
        void clearWidgets();
        void bringWidgetToFront(const WidgetPointer& _widget);
        const WidgetCollection& getWidgets() const;
        bool flushWindowNotifications();

        bool setDockingEnabled(bool _isEnabled);
        bool isDockingEnabled() const;
        DockingSpaceID getDefaultSpaceID() const;
        DockingSpaceID getWindowSpaceID(const WindowPointer& _window) const;
        bool placeWindow(const WindowPointer& _window, const WindowPlacement& _placement);
        bool isInteractionAffectedByPlacement(const WindowPointer& _window, const WindowPlacement& _placement) const;
        void clearInteraction();

        EGO_RTTI_VIRTUAL(WindowHost, Container);

    protected:
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;

        InputReply onPointerMove(const PointerMoveEvent& _event) override;
        InputReply onMouseButton(const MouseButtonEvent& _event) override;
        void onPointerLeave(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerCaptureLost(const Position& _position) override;

        void drawBaseLayer(PaintContext& _context) const override;
        void drawOverlayLayer(PaintContext& _context) const override;

    private:
        enum class DragOrigin
        {
            None,
            FloatingWindow,
            Tab
        };

        struct DragState final
        {
            DragOrigin m_origin = DragOrigin::None;
            WindowWeakPointer m_window;
            Position m_startPosition = PositionZero;
            Position m_position = PositionZero;
            Rect m_startBounds;
            Rect m_floatingBounds;
            bool m_hasPassedThreshold = false;
        };

        struct ArrangedTab final
        {
            WindowWeakPointer m_window;
            Rect m_bounds;
            bool m_isSelected = false;
        };

        explicit WindowHost(const SurfaceRootPointer& _root);

        static WindowPointer GetWindow(const WidgetPointer& _widget);
        WindowPointer findHostedWindow(const Window& _window) const;
        void moveWidgetToFront(const WidgetPointer& _widget);
        void moveDockedWindowsToBack();
        void queueWindowNotification(const WindowPointer& _window);

        void bindWindowDrag(const WindowPointer& _window);
        void releaseWindow(const WindowPointer& _window);
        void releaseDocking();

        void updateDockingLayout(const Rect& _bounds, const DockingStyle& _style);
        void updateDockingSpace(const DockingLayout::Space& _space);

        bool isInteractionActive() const;
        bool isDraggingWindow(const Window& _window, DragOrigin _origin) const;
        bool isDockChromeUnoccluded(const Position& _position) const;
        DockingSplitID findSplitAt(const Position& _position) const;
        WindowPointer findTabAt(const Position& _position) const;
        void updateActiveSplit(const Position& _position);

        void beginTabInteraction(const WindowPointer& _window, const Position& _position);
        void beginDrag(const WindowPointer& _window, DragOrigin _origin, const Position& _position, const Rect& _floatingBounds);
        void updateDrag(const Position& _position);
        void finishDrag(const Position& _position);
        void updatePreview(const Position& _position);

        void onWindowDragBegin(Window& _window, const Position& _position) override;
        void onWindowDragMove(Window& _window, const Position& _position) override;
        void onWindowDragEnd(Window& _window, const Position& _position) override;
        void onWindowDragCancel(Window& _window, const Position& _position) override;

        static void DrawTarget(
            PaintContext& _context,
            const Rect& _bounds,
            DockingPlacement _placement,
            const NormalizedColorRGBA& _fillColor,
            const NormalizedColorRGBA& _borderColor);

        size_t getChildCount() const override;
        const WidgetPointer& getChild(size_t _index) const override;

        SurfaceRootWeakPointer m_root;
        WidgetCollection m_widgets;
        std::vector<WindowPointer> m_pendingWindowNotifications;

        DockingLayout m_layout;
        DockingLayout::Arrangement m_arrangement;
        Rect m_dockingBounds;
        std::vector<ArrangedTab> m_tabs;
        DockingStyle m_style;

        WindowWeakPointer m_hoveredTabWindow;
        DockingSplitID m_hoveredSplitID = InvalidDockingSplitID;
        DockingSplitID m_activeSplitID = InvalidDockingSplitID;
        DragState m_drag;
        DockingPreview m_preview;

        bool m_isDockingEnabled = false;
    };
} // namespace ego::gui
