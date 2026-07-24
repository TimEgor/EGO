#pragma once

#include <cstddef>

#include "EgoGui/Widgets/Container.h"

namespace ego::gui
{
    class DockingSpace;
    class DockingTab;
    class Window;

    EGO_POINTER(DockingSpace);
    EGO_POINTER(DockingTab);
    EGO_POINTER(Window);

    class DockingTab final : public Container
    {
    public:
        static DockingTabPointer Create(const WindowPointer& _window);

        WindowPointer getWindow() const;
        WindowPointer releaseWindow();
        bool isSelected() const;
        void setArrangement(const Rect& _headerBounds, const Rect& _contentBounds);
        void clearInteraction();

        bool isChildActive(size_t _index) const override;
        bool hitTest(const Position& _position) const override;

        EGO_RTTI_VIRTUAL(DockingTab, Container);

    protected:
        InputReply onPointerMove(WidgetUpdateContext& _context, const PointerMoveEvent& _event) override;
        InputReply onMouseButton(WidgetUpdateContext& _context, const MouseButtonEvent& _event) override;
        void onPointerEnter(WidgetUpdateContext& _context, const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerLeave(WidgetUpdateContext& _context, const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerCaptureLost(WidgetUpdateContext& _context, const Position& _position) override;

        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;
        void drawBaseLayer(PaintContext& _context) const override;
        bool isChildHitTestVisible(const Position& _position) const override;

    private:
        DockingTab() = default;

        bool initialize(const WindowPointer& _window);
        DockingSpacePointer getSpace() const;

        size_t getChildCount() const override;
        WidgetPointer getChild(size_t _index) const override;

        WindowPointer m_window = nullptr;
        Rect m_headerBounds;
        Rect m_contentBounds;
        bool m_isHovered = false;
        bool m_isPressed = false;
    };
} // namespace ego::gui
