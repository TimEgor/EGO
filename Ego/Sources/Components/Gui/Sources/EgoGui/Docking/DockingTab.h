#pragma once

#include <cstddef>

#include "EgoCore/Patterns/NonInstanceable.h"

#include "EgoGui/Widgets/Container.h"

namespace ego::gui
{
    class DockingSpace;
    class DockingTab;
    class Window;

    EGO_POINTER(DockingSpace);
    EGO_WEAK_POINTER(DockingSpace);
    EGO_POINTER(DockingTab);
    EGO_POINTER(Window);

    class DockingTab final : public Container
    {
    public:
        class HierarchyAccessor final : public NonInstanceable
        {
            friend class DockingSpace;

            static bool AttachToSpace(DockingTab& _tab, const DockingSpacePointer& _space);
            static bool IsAttachedToSpace(const DockingTab& _tab, const DockingSpace& _space);
            static void DetachFromSpace(DockingTab& _tab);
        };

        ~DockingTab() override;

        static DockingTabPointer Create(const WindowPointer& _window);

        WindowPointer getWindow() const;
        WindowPointer releaseWindow();
        bool isSelected() const;
        void setArrangement(const Rect& _headerBounds, const Rect& _contentBounds);
        void clearInteraction();

        EGO_RTTI_VIRTUAL(DockingTab, Container);

    protected:
        bool isChildActive(size_t _index) const override;
        bool hitTest(const Position& _position) const override;

        InputReply onPointerMove(InputContext& _context, const PointerMoveEvent& _event) override;
        InputReply onMouseButton(InputContext& _context, const MouseButtonEvent& _event) override;
        void onPointerEnter(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerLeave(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerCaptureLost(const Position& _position) override;

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

        DockingSpaceWeakPointer m_space;
        WindowPointer m_window = nullptr;
        Rect m_headerBounds;
        Rect m_contentBounds;
        bool m_isHovered = false;
        bool m_isPressed = false;
    };
} // namespace ego::gui
