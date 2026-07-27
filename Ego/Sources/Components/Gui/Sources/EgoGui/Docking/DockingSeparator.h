#pragma once

#include "EgoGui/Docking/DockingTypes.h"
#include "EgoGui/Widgets/Widget.h"

namespace ego::gui
{
    class DockingSplit;
    EGO_POINTER(DockingSplit);
    EGO_WEAK_POINTER(DockingSplit);

    class DockingSeparator;
    EGO_POINTER(DockingSeparator);

    class DockingSeparator final : public Widget
    {
    public:
        static DockingSeparatorPointer Create(const DockingSplitPointer& _split, DockingAxis _axis);

        void clearInteraction();

        EGO_RTTI_VIRTUAL(DockingSeparator, Widget);

    protected:
        InputReply onPointerMove(InputContext& _context, const PointerMoveEvent& _event) override;
        InputReply onMouseButton(InputContext& _context, const MouseButtonEvent& _event) override;
        void onPointerEnter(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerLeave(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerCaptureLost(const Position& _position) override;

        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void drawBaseLayer(PaintContext& _context) const override;

    private:
        DockingSeparator(const DockingSplitPointer& _split, DockingAxis _axis);

        DockingSplitPointer getSplit() const;

        DockingSplitWeakPointer m_split;
        DockingAxis m_axis;
        Size m_minimumSpaceSize = SizeZero;
        float m_separatorThickness = 0.0f;
        bool m_isHovered = false;
        bool m_isPressed = false;
    };
} // namespace ego::gui
