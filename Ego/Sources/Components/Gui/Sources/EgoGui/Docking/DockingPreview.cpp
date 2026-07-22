#include "DockingPreview.h"

#include <algorithm>
#include <array>
#include <cmath>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Theme/Theme.h"

ego::gui::DockingPreview ego::gui::DockingPreviewBuilder::Build(
    const Position& _pointerPosition,
    DockingSpaceID _targetSpaceID,
    const Rect& _targetSpaceBounds,
    const Rect& _rootBounds,
    const Size& _rootMinimumSize,
    bool _showRootTargets,
    const DockingStyle& _style)
{
    DockingPreview preview;
    preview.m_isVisible = true;
    preview.m_targetSpaceID = _targetSpaceID;

    const bool showSpaceTargets = _targetSpaceID != InvalidDockingSpaceID;
    if (showSpaceTargets)
    {
        const float targetSize = CalculateTargetSize(_targetSpaceBounds, _style);
        const float targetOffset = targetSize + (std::max)(0.0f, _style.m_targetSpacing);
        const Position center(
            _targetSpaceBounds.m_position.m_x + _targetSpaceBounds.m_size.m_x * 0.5f,
            _targetSpaceBounds.m_position.m_y + _targetSpaceBounds.m_size.m_y * 0.5f);
        preview.m_targets[0].m_bounds = Rect(center.m_x - targetSize * 0.5f, center.m_y - targetSize * 0.5f, targetSize, targetSize);
        preview.m_targets[1].m_bounds = Rect(center.m_x - targetOffset - targetSize * 0.5f, center.m_y - targetSize * 0.5f, targetSize, targetSize);
        preview.m_targets[2].m_bounds = Rect(center.m_x + targetOffset - targetSize * 0.5f, center.m_y - targetSize * 0.5f, targetSize, targetSize);
        preview.m_targets[3].m_bounds = Rect(center.m_x - targetSize * 0.5f, center.m_y - targetOffset - targetSize * 0.5f, targetSize, targetSize);
        preview.m_targets[4].m_bounds = Rect(center.m_x - targetSize * 0.5f, center.m_y + targetOffset - targetSize * 0.5f, targetSize, targetSize);
    }

    if (_showRootTargets)
    {
        const float targetSize = CalculateTargetSize(_rootBounds, _style);
        const float targetInset = (std::max)(0.0f, _style.m_targetSpacing);
        const Position center(_rootBounds.m_position.m_x + _rootBounds.m_size.m_x * 0.5f, _rootBounds.m_position.m_y + _rootBounds.m_size.m_y * 0.5f);
        preview.m_targets[5].m_bounds = Rect(_rootBounds.getLeft() + targetInset, center.m_y - targetSize * 0.5f, targetSize, targetSize);
        preview.m_targets[6].m_bounds = Rect(_rootBounds.getRight() - targetInset - targetSize, center.m_y - targetSize * 0.5f, targetSize, targetSize);
        preview.m_targets[7].m_bounds = Rect(center.m_x - targetSize * 0.5f, _rootBounds.getTop() + targetInset, targetSize, targetSize);
        preview.m_targets[8].m_bounds = Rect(center.m_x - targetSize * 0.5f, _rootBounds.getBottom() - targetInset - targetSize, targetSize, targetSize);
    }

    const float targetHitPadding = (std::max)(2.0f, _style.m_targetSpacing * 0.5f);
    for (size_t targetIndex = 0; targetIndex < preview.m_targets.size(); ++targetIndex)
    {
        DockingTarget& target = preview.m_targets[targetIndex];
        target.m_level = GetLevel(targetIndex);
        target.m_placement = GetPlacement(targetIndex);
        target.m_isVisible = target.m_level == DockingTargetLevel::Space ? showSpaceTargets : _showRootTargets;
        if (!target.m_isVisible)
        {
            continue;
        }

        target.m_isAvailable = target.m_level == DockingTargetLevel::Space ?
                                   IsPlacementAvailable(_targetSpaceBounds, target.m_placement, _style) :
                                   IsRootPlacementAvailable(_rootBounds, _rootMinimumSize, target.m_placement, _style);
        target.m_hitBounds = Rect(
            target.m_bounds.m_position.m_x - targetHitPadding,
            target.m_bounds.m_position.m_y - targetHitPadding,
            target.m_bounds.m_size.m_x + targetHitPadding * 2.0f,
            target.m_bounds.m_size.m_y + targetHitPadding * 2.0f);
    }

    for (const DockingTarget& target : preview.m_targets)
    {
        if (!target.m_isVisible || !target.m_hitBounds.contains(_pointerPosition))
        {
            continue;
        }

        preview.m_isTargetHovered = true;
        preview.m_targetLevel = target.m_level;
        preview.m_placement = target.m_placement;
        preview.m_isDropAllowed = target.m_isAvailable;
        if (target.m_level == DockingTargetLevel::Root && target.m_isAvailable)
        {
            preview.m_splitRatio = CalculateRootSplitRatio(_rootBounds, _rootMinimumSize, target.m_placement, _style);
        }
        break;
    }

    if (preview.m_isDropAllowed)
    {
        const Rect& splitBounds = preview.m_targetLevel == DockingTargetLevel::Space ? _targetSpaceBounds : _rootBounds;
        preview.m_previewBounds = CalculatePreviewBounds(splitBounds, preview.m_placement, preview.m_splitRatio, _style);
    }
    return preview;
}

ego::gui::DockingTargetLevel ego::gui::DockingPreviewBuilder::GetLevel(size_t _targetIndex)
{
    EGO_ASSERT(_targetIndex < 9);
    return _targetIndex < 5 ? DockingTargetLevel::Space : DockingTargetLevel::Root;
}

ego::gui::DockingPlacement ego::gui::DockingPreviewBuilder::GetPlacement(size_t _targetIndex)
{
    static constexpr std::array<DockingPlacement, 9> Placements = {DockingPlacement::Center,
        DockingPlacement::Left,
        DockingPlacement::Right,
        DockingPlacement::Top,
        DockingPlacement::Bottom,
        DockingPlacement::Left,
        DockingPlacement::Right,
        DockingPlacement::Top,
        DockingPlacement::Bottom};
    EGO_ASSERT(_targetIndex < Placements.size());
    return Placements[_targetIndex];
}

float ego::gui::DockingPreviewBuilder::CalculateTargetSize(const Rect& _bounds, const DockingStyle& _style)
{
    const float smallerExtent = (std::min)(_bounds.m_size.m_x, _bounds.m_size.m_y);
    return (std::min)((std::max)(16.0f, _style.m_targetSize), (std::max)(16.0f, smallerExtent / 5.0f));
}

bool ego::gui::DockingPreviewBuilder::IsPlacementAvailable(const Rect& _spaceBounds, DockingPlacement _placement, const DockingStyle& _style)
{
    if (_placement == DockingPlacement::Center)
    {
        return true;
    }
    if (_placement == DockingPlacement::Left || _placement == DockingPlacement::Right)
    {
        return _spaceBounds.m_size.m_x >= _style.m_minimumSpaceSize.m_x * 2.0f + _style.m_separatorThickness;
    }
    return _spaceBounds.m_size.m_y >= _style.m_minimumSpaceSize.m_y * 2.0f + _style.m_separatorThickness;
}

bool ego::gui::DockingPreviewBuilder::IsRootPlacementAvailable(
    const Rect& _rootBounds,
    const Size& _rootMinimumSize,
    DockingPlacement _placement,
    const DockingStyle& _style)
{
    if (_placement == DockingPlacement::Left || _placement == DockingPlacement::Right)
    {
        return _rootBounds.m_size.m_x >= _rootMinimumSize.m_x + _style.m_minimumSpaceSize.m_x + _style.m_separatorThickness;
    }

    return _rootBounds.m_size.m_y >= _rootMinimumSize.m_y + _style.m_minimumSpaceSize.m_y + _style.m_separatorThickness;
}

float ego::gui::DockingPreviewBuilder::CalculateRootSplitRatio(
    const Rect& _rootBounds,
    const Size& _rootMinimumSize,
    DockingPlacement _placement,
    const DockingStyle& _style)
{
    const bool horizontal = _placement == DockingPlacement::Left || _placement == DockingPlacement::Right;
    const float totalExtent = horizontal ? _rootBounds.m_size.m_x : _rootBounds.m_size.m_y;
    const float availableExtent = (std::max)(0.0f, totalExtent - _style.m_separatorThickness);
    if (availableExtent <= 0.0f)
    {
        return 0.5f;
    }

    const float payloadMinimumExtent = horizontal ? _style.m_minimumSpaceSize.m_x : _style.m_minimumSpaceSize.m_y;
    const float rootMinimumExtent = horizontal ? _rootMinimumSize.m_x : _rootMinimumSize.m_y;
    const float minimumRatio = payloadMinimumExtent / availableExtent;
    const float maximumRatio = 1.0f - rootMinimumExtent / availableExtent;

    return (std::clamp)(0.5f, minimumRatio, maximumRatio);
}

ego::gui::Rect ego::gui::DockingPreviewBuilder::CalculatePreviewBounds(
    const Rect& _spaceBounds,
    DockingPlacement _placement,
    float _splitRatio,
    const DockingStyle& _style)
{
    if (_placement == DockingPlacement::Center)
    {
        return _spaceBounds;
    }

    const float ratio = (std::clamp)(_splitRatio, 0.05f, 0.95f);
    if (_placement == DockingPlacement::Left || _placement == DockingPlacement::Right)
    {
        const float availableWidth = (std::max)(0.0f, _spaceBounds.m_size.m_x - _style.m_separatorThickness);
        const float previewWidth = std::floor(availableWidth * ratio);
        const float previewX = _placement == DockingPlacement::Left ? _spaceBounds.m_position.m_x : _spaceBounds.getRight() - previewWidth;
        return Rect(previewX, _spaceBounds.m_position.m_y, previewWidth, _spaceBounds.m_size.m_y);
    }

    const float availableHeight = (std::max)(0.0f, _spaceBounds.m_size.m_y - _style.m_separatorThickness);
    const float previewHeight = std::floor(availableHeight * ratio);
    const float previewY = _placement == DockingPlacement::Top ? _spaceBounds.m_position.m_y : _spaceBounds.getBottom() - previewHeight;
    return Rect(_spaceBounds.m_position.m_x, previewY, _spaceBounds.m_size.m_x, previewHeight);
}
