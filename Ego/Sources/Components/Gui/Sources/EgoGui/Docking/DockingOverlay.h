#pragma once

#include <array>
#include <cstddef>

#include "EgoGui/Docking/DockingPreview.h"
#include "EgoGui/Theme/Theme.h"
#include "EgoGui/Widgets/Container.h"

namespace ego::gui
{
    class DockingArea;
    class WidgetUpdateContext;
    class Window;

    EGO_POINTER(Window);
    EGO_WEAK_POINTER(Window);

    class DockingTargetWidget;
    class DockingOverlay;

    EGO_POINTER(DockingTargetWidget);
    EGO_POINTER(DockingOverlay);

    class DockingTargetWidget final
        : public Widget
    {
    public:
        static DockingTargetWidgetPointer Create();

        void setTarget(const DockingTarget& _target, bool _isHovered);
        const DockingTarget& getTarget() const;
        bool isHovered() const;

        bool hitTest(const Position& _position) const override;

        EGO_RTTI_VIRTUAL(DockingTargetWidget, Widget);

    protected:
        void drawBaseLayer(PaintContext& _context) const override;

    private:
        DockingTargetWidget() = default;

        DockingTarget m_target;
        bool m_isHovered = false;
    };

    class DockingOverlay final
        : public Container
    {
    public:
        static DockingOverlayPointer Create();

        void beginFloatingDrag(WidgetUpdateContext& _context, const WindowPointer& _window, const Position& _position);
        void beginDockedDrag(WidgetUpdateContext& _context, const WindowPointer& _window, const Position& _position);
        void updateDrag(WidgetUpdateContext& _context, const WindowPointer& _window, const Position& _position);
        void finishDrag(WidgetUpdateContext& _context, const WindowPointer& _window, const Position& _position);
        void cancelDrag(const WindowPointer& _window);
        void clearPreview();
        void clear();

        bool isDraggingWindow(const WindowPointer& _window) const;
        bool hitTest(const Position& _position) const override;

        EGO_RTTI_VIRTUAL(DockingOverlay, Container);

    protected:
        InputReply onPointerMove(WidgetUpdateContext& _context, const PointerMoveEvent& _event) override;
        InputReply onMouseButton(WidgetUpdateContext& _context, const MouseButtonEvent& _event) override;
        void onPointerCaptureLost(WidgetUpdateContext& _context, const Position& _position) override;

        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;
        void drawBaseLayer(PaintContext& _context) const override;

    private:
        enum class DragOrigin
        {
            None,
            FloatingWindow,
            DockingTab
        };

        struct DragState final
        {
            DragOrigin m_origin = DragOrigin::None;
            WindowWeakPointer m_window;
            Position m_startPosition = PositionZero;
            Rect m_startBounds;
            Rect m_floatingBounds;
            bool m_hasPassedThreshold = false;
        };

        DockingOverlay();

        bool initialize();
        bool makeDockedWindowFloating(WidgetUpdateContext& _context, const WindowPointer& _window);
        void beginDrag(const WindowPointer& _window, DragOrigin _origin, const Position& _position, const Rect& _floatingBounds);
        void updatePreview(const DockingArea& _dockingArea, const Position& _position);
        void applyPreview(const DockingPreview& _preview);
        DockingTargetWidgetPointer findTargetAt(const Position& _position, bool _onlyAvailable) const;

        size_t getChildCount() const override;
        WidgetPointer getChild(size_t _index) const override;

        std::array<DockingTargetWidgetPointer, 9> m_targets;
        DockingStyle m_style;
        DragState m_drag;
        Rect m_previewBounds;
        DockingSpaceID m_targetSpaceID = InvalidDockingSpaceID;
        float m_splitRatio = 0.5f;
        bool m_isPreviewVisible = false;
        bool m_hasHoveredTarget = false;
    };
} // namespace ego::gui
