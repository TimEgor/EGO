#pragma once

#include <cstddef>

#include "EgoGui/Widgets/Container.h"

namespace ego::gui
{
    class DockingSpace;

    class DockingNode;
    EGO_POINTER(DockingNode);

    class DockingNode : public Container
    {
    public:
        ~DockingNode() override;

        virtual size_t getSpaceCount() const = 0;
        virtual size_t getSpaceCountExcluding(const DockingSpace& _space) const = 0;
        virtual Size getMinimumSize(const Size& _minimumSpaceSize, float _separatorThickness) const = 0;
        virtual Size getMinimumSizeExcluding(const DockingSpace& _space, const Size& _minimumSpaceSize, float _separatorThickness) const = 0;
        virtual void clearInteraction() = 0;

        EGO_RTTI_VIRTUAL(DockingNode, Container);

    protected:
        DockingNode() = default;
    };
} // namespace ego::gui
