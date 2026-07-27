#pragma once

#include <cstddef>
#include <functional>
#include <optional>

#include "EgoGui/Widgets/Container.h"

namespace ego::gui
{
    class DockingSpace;

    struct DockingMeasureContext final
    {
        Size m_minimumSpaceSize = SizeZero;
        float m_separatorThickness = 0.0f;
        std::optional<std::reference_wrapper<const DockingSpace>> m_excludedSpace;
    };

    struct DockingMetrics final
    {
        size_t m_spaceCount = 0;
        Size m_minimumSize = SizeZero;
    };

    class DockingNode;
    EGO_POINTER(DockingNode);

    class DockingNode : public Container
    {
    public:
        ~DockingNode() override;

        virtual DockingMetrics measure(const DockingMeasureContext& _context) const = 0;
        virtual void clearInteraction() = 0;

        EGO_RTTI_VIRTUAL(DockingNode, Container);

    protected:
        DockingNode() = default;
    };
} // namespace ego::gui
