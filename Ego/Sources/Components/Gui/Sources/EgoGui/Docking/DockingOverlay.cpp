#include "DockingOverlay.h"

#include <algorithm>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Docking/DockingArea.h"
#include "EgoGui/Docking/DockingNode.h"
#include "EgoGui/Docking/DockingSpace.h"
#include "EgoGui/Input/WidgetUpdateContext.h"
#include "EgoGui/Theme/Theme.h"
#include "EgoGui/Viewport/SurfaceRoot.h"
#include "EgoGui/Widgets/Window.h"

ego::gui::DockingTargetWidgetPointer ego::gui::DockingTargetWidget::Create()
{
    return new DockingTargetWidget();
}

void ego::gui::DockingTargetWidget::setTarget(const DockingTarget& _target, bool _isHovered)
{
    m_target = _target;
    m_isHovered = _isHovered;
}

const ego::gui::DockingTarget& ego::gui::DockingTargetWidget::getTarget() const
{
    return m_target;
}

bool ego::gui::DockingTargetWidget::isHovered() const
{
    return m_isHovered;
}

bool ego::gui::DockingTargetWidget::hitTest(const Position& _position) const
{
    return m_target.m_isVisible && m_target.m_isAvailable && m_target.m_hitBounds.contains(_position);
}

void ego::gui::DockingTargetWidget::drawBaseLayer(PaintContext& _context) const
{
    if (!m_target.m_isVisible || !m_target.m_isAvailable)
    {
        return;
    }

    constexpr float BorderThickness = 2.0f;

    const DockingStyle& style = _context.getTheme().m_docking;
    const NormalizedColorRGBA& color = style.m_target.resolve(m_isHovered, false);
    const Rect& bounds = getLayoutBounds();
    _context.drawBox(bounds, color);
    _context.drawBorder(bounds, BorderThickness, style.m_targetBorder);

    const float centerX = bounds.m_position.m_x + bounds.m_size.m_x * 0.5f;
    const float centerY = bounds.m_position.m_y + bounds.m_size.m_y * 0.5f;
    if (m_target.m_placement == DockingPlacement::Left || m_target.m_placement == DockingPlacement::Right)
    {
        _context.drawBox(Rect(centerX - BorderThickness * 0.5f, bounds.getTop(), BorderThickness, bounds.m_size.m_y), style.m_targetBorder);
    }
    else if (m_target.m_placement == DockingPlacement::Top || m_target.m_placement == DockingPlacement::Bottom)
    {
        _context.drawBox(Rect(bounds.getLeft(), centerY - BorderThickness * 0.5f, bounds.m_size.m_x, BorderThickness), style.m_targetBorder);
    }
}

ego::gui::DockingOverlayPointer ego::gui::DockingOverlay::Create()
{
    const DockingOverlayPointer overlay = new DockingOverlay();

    return overlay->initialize() ? overlay : nullptr;
}

ego::gui::DockingOverlay::DockingOverlay()
    : m_style(Theme::GetDefault().m_docking)
{
}

bool ego::gui::DockingOverlay::initialize()
{
    for (DockingTargetWidgetPointer& target : m_targets)
    {
        target = DockingTargetWidget::Create();
        if (!target || !attachChild(target))
        {
            return false;
        }
    }

    return true;
}

void ego::gui::DockingOverlay::beginFloatingDrag(WidgetUpdateContext& _context, const WindowPointer& _window, const Position& _position)
{
    const DockingAreaPointer dockingArea = _context.getDockingArea();
    if (!dockingArea || !dockingArea->getRootNode() || !_window || !_window->isDirectChildOf(_context.getRoot()))
    {
        return;
    }

    beginDrag(_window, DragOrigin::FloatingWindow, _position, _window->getFloatingBounds());
}

void ego::gui::DockingOverlay::beginDockedDrag(WidgetUpdateContext& _context, const WindowPointer& _window, const Position& _position)
{
    const DockingAreaPointer dockingArea = _context.getDockingArea();
    if (!dockingArea || !dockingArea->getRootNode() || !dockingArea->findWindowSpace(_window))
    {
        return;
    }

    const Size requestedSize = _window->getFloatingBounds().m_size;
    const Size floatingSize(
        requestedSize.m_x > 0.0f ? requestedSize.m_x : m_style.m_minimumSpaceSize.m_x,
        requestedSize.m_y > 0.0f ? requestedSize.m_y : m_style.m_minimumSpaceSize.m_y);
    const Position grabOffset(floatingSize.m_x * 0.5f, (std::min)(floatingSize.m_y, m_style.m_tabHeight) * 0.5f);
    const Rect floatingBounds(Position(_position.m_x - grabOffset.m_x, _position.m_y - grabOffset.m_y), floatingSize);
    beginDrag(_window, DragOrigin::DockingTab, _position, floatingBounds);
}

bool ego::gui::DockingOverlay::makeDockedWindowFloating(WidgetUpdateContext& _context, const WindowPointer& _window)
{
    const WidgetPointer captureWidget = _window ? _window->getParent() : nullptr;
    const DockingOverlayPointer self = ego::StaticPointerCast<DockingOverlay>(sharedFromThis());
    if (!captureWidget || !self || !_context.transferPointerCapture(captureWidget, self))
    {
        return false;
    }

    if (!_context.getRoot().makeWindowFloating(_window, m_drag.m_floatingBounds))
    {
        const bool captureRestored = _context.transferPointerCapture(self, captureWidget);
        EGO_ASSERT(captureRestored);

        return false;
    }

    m_drag.m_origin = DragOrigin::FloatingWindow;

    return true;
}

void ego::gui::DockingOverlay::beginDrag(const WindowPointer& _window, DragOrigin _origin, const Position& _position, const Rect& _floatingBounds)
{
    m_drag = DragState();
    m_drag.m_origin = _origin;
    m_drag.m_window = _window;
    m_drag.m_startPosition = _position;
    m_drag.m_startBounds = _floatingBounds;
    m_drag.m_floatingBounds = _floatingBounds;
    applyPreview(DockingPreview());
}

void ego::gui::DockingOverlay::updateDrag(WidgetUpdateContext& _context, const WindowPointer& _window, const Position& _position)
{
    const WindowPointer window = _window;
    const DockingAreaPointer dockingArea = _context.getDockingArea();
    if (!dockingArea || !dockingArea->getRootNode() || !isDraggingWindow(window))
    {
        return;
    }

    if (!getLayoutBounds().contains(_position))
    {
        clearPreview();

        return;
    }

    const Position offset(_position.m_x - m_drag.m_startPosition.m_x, _position.m_y - m_drag.m_startPosition.m_y);
    m_drag.m_floatingBounds.m_position = Position(m_drag.m_startBounds.m_position.m_x + offset.m_x, m_drag.m_startBounds.m_position.m_y + offset.m_y);
    if (m_drag.m_origin == DragOrigin::FloatingWindow)
    {
        window->setPosition(m_drag.m_floatingBounds.m_position);
    }

    if (!m_drag.m_hasPassedThreshold)
    {
        const float distanceSquared = offset.m_x * offset.m_x + offset.m_y * offset.m_y;
        if (distanceSquared < m_style.m_dragThreshold * m_style.m_dragThreshold)
        {
            return;
        }

        m_drag.m_hasPassedThreshold = true;
    }

    if (m_drag.m_origin == DragOrigin::DockingTab && !makeDockedWindowFloating(_context, window))
    {
        clear();

        return;
    }

    updatePreview(*dockingArea, _position);
    invalidateLayout();
}

void ego::gui::DockingOverlay::finishDrag(WidgetUpdateContext& _context, const WindowPointer& _window, const Position& _position)
{
    const WindowPointer window = _window;
    if (!isDraggingWindow(window))
    {
        return;
    }

    updateDrag(_context, window, _position);
    if (!isDraggingWindow(window))
    {
        return;
    }

    SurfaceRoot& root = _context.getRoot();
    const DockingTargetWidgetPointer target = findTargetAt(_position, true);
    if (m_drag.m_hasPassedThreshold && target)
    {
        const DockingTarget& targetData = target->getTarget();
        const bool docked = targetData.m_level == DockingTargetLevel::Root ? root.dockWindowToRoot(window, targetData.m_placement, m_splitRatio) :
                                                                             root.dockWindow(
                                                                                 window,
                                                                                 WindowPlacement{
                                                                                     .m_spaceID = m_targetSpaceID,
                                                                                     .m_placement = targetData.m_placement,
                                                                                     .m_splitRatio = m_splitRatio,
                                                                                 });
        EGO_ASSERT(docked);
    }

    clear();
}

void ego::gui::DockingOverlay::cancelDrag(const WindowPointer& _window)
{
    if (isDraggingWindow(_window))
    {
        clear();
    }
}

void ego::gui::DockingOverlay::clearPreview()
{
    applyPreview(DockingPreview());
    invalidateLayout();
}

void ego::gui::DockingOverlay::clear()
{
    m_drag = DragState();
    clearPreview();
}

bool ego::gui::DockingOverlay::isDraggingWindow(const WindowPointer& _window) const
{
    const WindowPointer dragWindow = m_drag.m_window.lock();

    return _window && m_drag.m_origin != DragOrigin::None && dragWindow.get() == _window.get();
}

bool ego::gui::DockingOverlay::hitTest(const Position& _position) const
{
    return static_cast<bool>(findTargetAt(_position, false));
}

ego::gui::InputReply ego::gui::DockingOverlay::onPointerMove(WidgetUpdateContext& _context, const PointerMoveEvent& _event)
{
    const WindowPointer dragWindow = m_drag.m_window.lock();
    if (!isDraggingWindow(dragWindow))
    {
        return InputReply::Unhandled;
    }

    updateDrag(_context, dragWindow, _event.m_position);

    return InputReply::Handled;
}

ego::gui::InputReply ego::gui::DockingOverlay::onMouseButton(WidgetUpdateContext& _context, const MouseButtonEvent& _event)
{
    const WindowPointer dragWindow = m_drag.m_window.lock();
    if (_event.m_key != MouseInputKey::ButtonLeft || _event.m_action != InputButtonAction::Released || !isDraggingWindow(dragWindow))
    {
        return InputReply::Unhandled;
    }

    finishDrag(_context, dragWindow, _event.m_position);

    return InputReply::Handled;
}

void ego::gui::DockingOverlay::onPointerCaptureLost(WidgetUpdateContext&, const Position&)
{
    clear();
}

void ego::gui::DockingOverlay::updatePreview(const DockingArea& _dockingArea, const Position& _position)
{
    const DockingNodePointer dockingRoot = _dockingArea.getRootNode();
    const DockingSpacePointer targetSpace = _dockingArea.findSpaceAt(_position);
    const bool showRootTargets = dockingRoot && dockingRoot->getSpaceCount() > 1;
    if (!targetSpace && (!showRootTargets || !getLayoutBounds().contains(_position)))
    {
        applyPreview(DockingPreview());

        return;
    }

    const WindowPointer dragWindow = m_drag.m_window.lock();
    const DockingSpacePointer dragSpace = _dockingArea.findWindowSpace(dragWindow);
    const bool removesOriginSpace = dragSpace && dragWindow && !dragSpace->hasNonCollapsedWindowExcept(*dragWindow);
    const DockingSpaceID targetSpaceID = targetSpace ? targetSpace->getID() : InvalidDockingSpaceID;
    const Rect targetSpaceBounds = _dockingArea.getDockingBounds(targetSpace);
    const Size rootMinimumSize = !dockingRoot ? SizeZero :
                                 removesOriginSpace ?
                                                dockingRoot->getMinimumSizeExcluding(*dragSpace, m_style.m_minimumSpaceSize, m_style.m_separatorThickness) :
                                                dockingRoot->getMinimumSize(m_style.m_minimumSpaceSize, m_style.m_separatorThickness);
    const bool canReuseOriginSpaceExtent = removesOriginSpace && targetSpace && dragSpace.get() != targetSpace.get();
    const bool allowHorizontalSpaceSplitAfterOriginRemoval = canReuseOriginSpaceExtent && _dockingArea.isSplitAvailableAfterRemoving(
                                                                                              dragSpace,
                                                                                              targetSpace,
                                                                                              DockingAxis::Horizontal,
                                                                                              getLayoutBounds().m_size,
                                                                                              m_style.m_minimumSpaceSize,
                                                                                              m_style.m_separatorThickness);
    const bool allowVerticalSpaceSplitAfterOriginRemoval = canReuseOriginSpaceExtent && _dockingArea.isSplitAvailableAfterRemoving(
                                                                                            dragSpace,
                                                                                            targetSpace,
                                                                                            DockingAxis::Vertical,
                                                                                            getLayoutBounds().m_size,
                                                                                            m_style.m_minimumSpaceSize,
                                                                                            m_style.m_separatorThickness);
    DockingPreview preview = DockingPreviewBuilder::Build(
        _position,
        targetSpaceID,
        targetSpaceBounds,
        getLayoutBounds(),
        rootMinimumSize,
        showRootTargets,
        allowHorizontalSpaceSplitAfterOriginRemoval,
        allowVerticalSpaceSplitAfterOriginRemoval,
        m_style);

    const bool isOnlyTabInTarget =
        dragSpace && dragWindow && targetSpace && dragSpace.get() == targetSpace.get() && !targetSpace->hasNonCollapsedWindowExcept(*dragWindow);
    if (targetSpace && (targetSpace->getSpaceCount() == 0 || isOnlyTabInTarget))
    {
        for (DockingTarget& target : preview.m_targets)
        {
            if (target.m_level == DockingTargetLevel::Space && target.m_placement != DockingPlacement::Center)
            {
                target.m_isAvailable = false;
            }
        }

        if (preview.m_targetLevel == DockingTargetLevel::Space && preview.m_placement != DockingPlacement::Center)
        {
            preview.m_isDropAllowed = false;
        }
    }
    applyPreview(preview);
}

void ego::gui::DockingOverlay::applyPreview(const DockingPreview& _preview)
{
    m_previewBounds = _preview.m_previewBounds;
    m_targetSpaceID = _preview.m_targetSpaceID;
    m_splitRatio = _preview.m_splitRatio;
    m_isPreviewVisible = _preview.m_isDropAllowed;
    m_hasHoveredTarget = _preview.m_isTargetHovered;
    for (size_t targetIndex = 0; targetIndex < m_targets.size(); ++targetIndex)
    {
        const DockingTarget& target = _preview.m_targets[targetIndex];
        const bool isHovered = _preview.m_isDropAllowed && target.m_level == _preview.m_targetLevel && target.m_placement == _preview.m_placement;
        m_targets[targetIndex]->setTarget(target, isHovered);
    }
}

ego::gui::DockingTargetWidgetPointer ego::gui::DockingOverlay::findTargetAt(const Position& _position, bool _onlyAvailable) const
{
    for (size_t targetIndex = m_targets.size(); targetIndex > 0; --targetIndex)
    {
        const DockingTargetWidgetPointer& target = m_targets[targetIndex - 1];
        if (!target || !target->hitTest(_position))
        {
            continue;
        }

        if (!_onlyAvailable || (target->getTarget().m_isAvailable && target->isHovered()))
        {
            return target;
        }
    }

    return nullptr;
}

ego::gui::Size ego::gui::DockingOverlay::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    m_style = _context.getTheme().m_docking;
    m_style.m_targetSize = (std::max)(0.0f, m_style.m_targetSize);
    m_style.m_targetSpacing = (std::max)(0.0f, m_style.m_targetSpacing);
    m_style.m_separatorThickness = (std::max)(0.0f, m_style.m_separatorThickness);
    m_style.m_tabHeight = (std::max)(0.0f, m_style.m_tabHeight);
    m_style.m_dragThreshold = (std::max)(0.0f, m_style.m_dragThreshold);
    m_style.m_minimumSpaceSize = Size((std::max)(0.0f, m_style.m_minimumSpaceSize.m_x), (std::max)(0.0f, m_style.m_minimumSpaceSize.m_y));

    for (const DockingTargetWidgetPointer& target : m_targets)
    {
        if (target)
        {
            target->updatePreferredSize(_context, LayoutConstraints(target->getTarget().m_bounds.m_size));
        }
    }

    return _constraints.m_maximumSize;
}

void ego::gui::DockingOverlay::updateGeometry(const LayoutContext& _context)
{
    for (const DockingTargetWidgetPointer& target : m_targets)
    {
        if (target)
        {
            const DockingTarget& targetData = target->getTarget();
            target->applyLayout(_context, targetData.m_isVisible ? targetData.m_bounds : Rect(targetData.m_bounds.m_position, SizeZero));
        }
    }
}

void ego::gui::DockingOverlay::drawBaseLayer(PaintContext& _context) const
{
    if (m_isPreviewVisible)
    {
        _context.drawBox(m_previewBounds, m_style.m_previewFill);
    }
}

size_t ego::gui::DockingOverlay::getChildCount() const
{
    return m_targets.size();
}

ego::gui::WidgetPointer ego::gui::DockingOverlay::getChild(size_t _index) const
{
    return _index < m_targets.size() ? m_targets[_index] : nullptr;
}
