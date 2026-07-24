#pragma once

#include <cstddef>

#include "EgoGui/Docking/DockingNode.h"
#include "EgoGui/Docking/DockingTypes.h"

namespace ego::gui
{
    struct DockingStyle;
    class DockingSeparator;
    class DockingSplit;

    EGO_POINTER(DockingSeparator);
    EGO_POINTER(DockingSplit);

    class DockingSplit final : public DockingNode
    {
    public:
        static DockingSplitPointer Create(DockingAxis _axis, float _ratio);

        DockingAxis getAxis() const;

        DockingNodePointer getFirst() const;
        DockingNodePointer getSecond() const;

        bool setChildren(const DockingNodePointer& _first, const DockingNodePointer& _second);
        bool replaceChildNode(const DockingNodePointer& _node, const DockingNodePointer& _replacement);
        DockingNodePointer releaseSibling(const DockingNodePointer& _node);
        void updateRatio(const Position& _position, const Size& _minimumSpaceSize, float _separatorThickness);

        size_t getSpaceCount() const override;
        size_t getSpaceCountExcluding(const DockingSpace& _space) const override;
        Size getMinimumSize(const Size& _minimumSpaceSize, float _separatorThickness) const override;
        Size getMinimumSizeExcluding(const DockingSpace& _space, const Size& _minimumSpaceSize, float _separatorThickness) const override;
        void clearInteraction() override;

        EGO_RTTI_VIRTUAL(DockingSplit, DockingNode);

    protected:
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;

    private:
        explicit DockingSplit(DockingAxis _axis, float _ratio);

        bool initialize();
        void setRatio(float _ratio);
        void calculateChildBounds(const Rect& _bounds, const DockingStyle& _style, Rect& _firstBounds, Rect& _separatorBounds, Rect& _secondBounds) const;

        size_t getChildCount() const override;
        WidgetPointer getChild(size_t _index) const override;

        DockingAxis m_axis;
        float m_ratio;
        DockingNodePointer m_first = nullptr;
        DockingNodePointer m_second = nullptr;
        DockingSeparatorPointer m_separator = nullptr;
    };
} // namespace ego::gui
