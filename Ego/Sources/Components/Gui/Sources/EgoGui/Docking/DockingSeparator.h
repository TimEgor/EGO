#pragma once

#include "EgoGui/Docking/DockingTypes.h"
#include "EgoGui/Widgets/Widget.h"

namespace ego::gui
{
    class DockingSplit;
    EGO_POINTER(DockingSplit);

    class DockingSeparator;
    EGO_POINTER(DockingSeparator);

    class DockingSeparator final : public Widget
    {
    public:
        static DockingSeparatorPointer Create(DockingAxis _axis);

        void clearInteraction();

        EGO_RTTI_VIRTUAL(DockingSeparator, Widget);

    protected:
        InputReply onPointerMove(WidgetUpdateContext& _context, const PointerMoveEvent& _event) override;
        InputReply onMouseButton(WidgetUpdateContext& _context, const MouseButtonEvent& _event) override;
        void onPointerEnter(WidgetUpdateContext& _context, const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerLeave(WidgetUpdateContext& _context, const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerCaptureLost(WidgetUpdateContext& _context, const Position& _position) override;

        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void drawBaseLayer(PaintContext& _context) const override;

    private:
        explicit DockingSeparator(DockingAxis _axis);

        DockingSplitPointer getSplit() const;

        DockingAxis m_axis;
        Size m_minimumSpaceSize = SizeZero;
        float m_separatorThickness = 0.0f;
        bool m_isHovered = false;
        bool m_isPressed = false;
    };
} // namespace ego::gui
