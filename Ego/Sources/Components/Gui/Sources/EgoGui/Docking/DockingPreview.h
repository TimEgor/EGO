#pragma once

#include <array>
#include <cstddef>

#include "EgoGui/Core/Geometry.h"

#include "DockingTypes.h"

namespace ego::gui
{
    struct DockingStyle;

    enum class DockingTargetLevel
    {
        Space,
        Root
    };

    struct DockingTarget final
    {
        DockingTargetLevel m_level = DockingTargetLevel::Space;
        DockingPlacement m_placement = DockingPlacement::Center;
        Rect m_bounds;
        Rect m_hitBounds;
        bool m_isVisible = false;
        bool m_isAvailable = false;
    };

    struct DockingPreview final
    {
        bool m_isTargetHovered = false;
        bool m_isDropAllowed = false;
        DockingSpaceID m_targetSpaceID = InvalidDockingSpaceID;
        DockingTargetLevel m_targetLevel = DockingTargetLevel::Space;
        DockingPlacement m_placement = DockingPlacement::Center;
        float m_splitRatio = 0.5f;
        Rect m_previewBounds;
        std::array<DockingTarget, 9> m_targets;
    };

    class DockingPreviewBuilder final
    {
    public:
        static DockingPreview Build(
            const Position& _pointerPosition,
            DockingSpaceID _targetSpaceID,
            const Rect& _targetSpaceBounds,
            const Rect& _rootBounds,
            const Size& _rootMinimumSize,
            bool _showRootTargets,
            bool _allowHorizontalSpaceSplitAfterOriginRemoval,
            bool _allowVerticalSpaceSplitAfterOriginRemoval,
            const DockingStyle& _style);

    private:
        static DockingPlacement GetSpacePlacement(size_t _targetIndex);
        static DockingPlacement GetRootPlacement(size_t _targetIndex);
        static float CalculateTargetSize(const Rect& _bounds, const DockingStyle& _style);
        static bool IsPlacementAvailable(const Rect& _spaceBounds, DockingPlacement _placement, const DockingStyle& _style);
        static bool IsRootPlacementAvailable(const Rect& _rootBounds, const Size& _rootMinimumSize, DockingPlacement _placement, const DockingStyle& _style);
        static float CalculateRootSplitRatio(const Rect& _rootBounds, const Size& _rootMinimumSize, DockingPlacement _placement, const DockingStyle& _style);
        static Rect CalculatePreviewBounds(const Rect& _spaceBounds, DockingPlacement _placement, float _splitRatio, const DockingStyle& _style);
    };
} // namespace ego::gui
