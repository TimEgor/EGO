#include "SurfaceRoot.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoGui/Docking/DockingArea.h"
#include "EgoGui/Docking/DockingNode.h"
#include "EgoGui/Docking/DockingOverlay.h"
#include "EgoGui/Docking/DockingPreview.h"
#include "EgoGui/Docking/DockingSpace.h"
#include "EgoGui/Layout/Layout.h"
#include "EgoGui/Widgets/Window.h"

ego::gui::SurfaceRoot::~SurfaceRoot() = default;

ego::gui::SurfaceRootPointer ego::gui::SurfaceRoot::ViewportAccessor::Create()
{
    return SurfaceRoot::Create();
}

bool ego::gui::SurfaceRoot::ViewportAccessor::AttachWindow(SurfaceRoot& _root, const WindowPointer& _window, bool _isBound)
{
    return _root.attachWindow(_window, _isBound);
}

ego::gui::WindowPointer ego::gui::SurfaceRoot::ViewportAccessor::ExtractWindow(SurfaceRoot& _root, const WindowPointer& _window)
{
    return _root.extractWindow(_window);
}

bool ego::gui::SurfaceRoot::ViewportAccessor::TransferWindowTo(SurfaceRoot& _root, SurfaceRoot& _target, const WindowPointer& _window)
{
    return _root.transferWindowTo(_target, _window);
}

void ego::gui::SurfaceRoot::ViewportAccessor::ClearWindows(SurfaceRoot& _root)
{
    _root.clearWindows();
}

bool ego::gui::SurfaceRoot::ViewportAccessor::ContainsWindow(const SurfaceRoot& _root, const WindowPointer& _window)
{
    return _root.containsWindow(_window);
}

size_t ego::gui::SurfaceRoot::ViewportAccessor::GetWindowCount(const SurfaceRoot& _root)
{
    return _root.getWindowCount();
}

ego::gui::WindowPointer ego::gui::SurfaceRoot::ViewportAccessor::GetWindow(const SurfaceRoot& _root, size_t _index)
{
    return _root.getWindow(_index);
}

size_t ego::gui::SurfaceRoot::ViewportAccessor::GetFloatingWindowCount(const SurfaceRoot& _root)
{
    return _root.getFloatingWindowCount();
}

ego::gui::WindowPointer ego::gui::SurfaceRoot::ViewportAccessor::GetFloatingWindow(const SurfaceRoot& _root, size_t _index)
{
    return _root.getFloatingWindow(_index);
}

bool ego::gui::SurfaceRoot::ViewportAccessor::IsWindowBound(const SurfaceRoot& _root, const WindowPointer& _window)
{
    return _root.isWindowBound(_window);
}

bool ego::gui::SurfaceRoot::ViewportAccessor::IsWindowFloating(const SurfaceRoot& _root, const WindowPointer& _window)
{
    return _root.isWindowFloating(_window);
}

bool ego::gui::SurfaceRoot::ViewportAccessor::SetDockingEnabled(SurfaceRoot& _root, bool _isEnabled)
{
    return _root.setDockingEnabled(_isEnabled);
}

bool ego::gui::SurfaceRoot::ViewportAccessor::IsDockingEnabled(const SurfaceRoot& _root)
{
    return _root.isDockingEnabled();
}

ego::gui::DockingSpaceID ego::gui::SurfaceRoot::ViewportAccessor::GetDefaultDockingSpaceID(const SurfaceRoot& _root)
{
    return _root.getDefaultDockingSpaceID();
}

ego::gui::DockingSpaceID ego::gui::SurfaceRoot::ViewportAccessor::GetWindowDockingSpaceID(const SurfaceRoot& _root, const WindowPointer& _window)
{
    return _root.getWindowDockingSpaceID(_window);
}

bool ego::gui::SurfaceRoot::ViewportAccessor::DockWindow(SurfaceRoot& _root, const WindowPointer& _window, const DockingLocation& _location)
{
    return _root.dockWindow(_window, _location);
}

void ego::gui::SurfaceRoot::ViewportAccessor::InvalidateLayout(SurfaceRoot& _root)
{
    _root.invalidateLayout();
}

void ego::gui::SurfaceRoot::ViewportAccessor::UpdateExternalDragPreview(SurfaceRoot& _root, const WindowPointer& _window, const Position& _position)
{
    _root.updateExternalDragPreview(_window, _position);
}

bool ego::gui::SurfaceRoot::ViewportAccessor::DockExternalWindow(SurfaceRoot& _root, const WindowPointer& _window, const Position& _position)
{
    return _root.dockExternalWindow(_window, _position);
}

void ego::gui::SurfaceRoot::ViewportAccessor::ClearExternalDragPreview(SurfaceRoot& _root)
{
    _root.clearExternalDragPreview();
}

void ego::gui::SurfaceRoot::InputAccessor::ClearInteraction(SurfaceRoot& _root)
{
    _root.clearInteraction();
}

void ego::gui::SurfaceRoot::InputAccessor::ClearPreview(SurfaceRoot& _root)
{
    _root.clearDockingPreview();
}

void ego::gui::SurfaceRoot::InputAccessor::BringWidgetToFront(SurfaceRoot& _root, const WidgetPointer& _widget)
{
    _root.bringWidgetToFront(_widget);
}

bool ego::gui::SurfaceRoot::WindowDragAccessor::ContainsWindow(const SurfaceRoot& _root, const WindowPointer& _window)
{
    return _root.containsWindow(_window);
}

bool ego::gui::SurfaceRoot::WindowDragAccessor::IsWindowFloating(const SurfaceRoot& _root, const WindowPointer& _window)
{
    return _root.isWindowFloating(_window);
}

bool ego::gui::SurfaceRoot::WindowDragAccessor::IsWindowDocked(const SurfaceRoot& _root, const WindowPointer& _window)
{
    return _root.m_dockingArea && _root.m_dockingArea->getRootNode() && _root.m_dockingArea->findWindowSpace(_window);
}

bool ego::gui::SurfaceRoot::WindowDragAccessor::MakeWindowFloating(SurfaceRoot& _root, const WindowPointer& _window, const Rect& _bounds)
{
    return _root.makeWindowFloating(_window, _bounds);
}

const ego::gui::DockingStyle& ego::gui::SurfaceRoot::WindowDragAccessor::GetDockingStyle(const SurfaceRoot& _root)
{
    return _root.m_dockingStyle;
}

ego::gui::DockingPreview ego::gui::SurfaceRoot::WindowDragAccessor::BuildPreview(
    const SurfaceRoot& _root,
    const WindowPointer& _window,
    const Position& _position)
{
    return _root.buildDockingPreview(_window, _position);
}

void ego::gui::SurfaceRoot::WindowDragAccessor::ShowPreview(SurfaceRoot& _root, const DockingPreview& _preview)
{
    _root.showDockingPreview(_preview);
}

bool ego::gui::SurfaceRoot::WindowDragAccessor::ApplyPreview(SurfaceRoot& _root, const WindowPointer& _window, const DockingPreview& _preview)
{
    return _root.applyDockingPreview(_window, _preview);
}

void ego::gui::SurfaceRoot::WindowDragAccessor::ClearPreview(SurfaceRoot& _root)
{
    _root.clearDockingPreview();
}

ego::gui::SurfaceRootPointer ego::gui::SurfaceRoot::Create()
{
    const SurfaceRootPointer root = new SurfaceRoot();

    return root->initialize() ? root : nullptr;
}

bool ego::gui::SurfaceRoot::initialize()
{
    m_dockingArea = DockingArea::Create();
    m_dockingOverlay = DockingOverlay::Create();
    if (!m_dockingArea || !m_dockingOverlay || !attachChild(m_dockingArea))
    {
        return false;
    }

    if (!attachChild(m_dockingOverlay))
    {
        detachChild(m_dockingArea);
        m_dockingArea = nullptr;

        return false;
    }

    return true;
}

bool ego::gui::SurfaceRoot::attachWindow(const WindowPointer& _window, bool _isBound)
{
    if (!_window || containsWindow(_window) || _window->getParent() || !attachFloatingWindow(_window))
    {
        return false;
    }

    if (_isBound)
    {
        m_boundWindows.push_back(_window);
    }

    return true;
}

ego::gui::WindowPointer ego::gui::SurfaceRoot::extractWindow(const WindowPointer& _window)
{
    const WindowPointer window = _window;
    if (!containsWindow(window))
    {
        return nullptr;
    }

    if (!detachFloatingWindow(window) && (!m_dockingArea || !m_dockingArea->undockWindow(window)))
    {
        return nullptr;
    }

    removeWindowBinding(window);

    return window;
}

bool ego::gui::SurfaceRoot::transferWindowTo(SurfaceRoot& _target, const WindowPointer& _window)
{
    const WindowPointer window = _window;
    if (&_target == this || !isWindowFloating(window))
    {
        return false;
    }

    const WindowCollection::iterator windowIt = std::find_if(
        m_floatingWindows.begin(),
        m_floatingWindows.end(),
        [&window](const WindowPointer& _currentWindow)
        {
            return _currentWindow.get() == window.get();
        });
    EGO_ASSERT(windowIt != m_floatingWindows.end());

    const size_t windowIndex = static_cast<size_t>(windowIt - m_floatingWindows.begin());
    const bool isBound = isWindowBound(window);
    if (!detachFloatingWindow(window))
    {
        return false;
    }

    removeWindowBinding(window);
    if (_target.attachWindow(window, isBound))
    {
        return true;
    }

    const bool attached = attachFloatingWindow(window);
    EGO_ASSERT(attached);
    if (windowIndex + 1 < m_floatingWindows.size())
    {
        m_floatingWindows.insert(m_floatingWindows.begin() + static_cast<std::ptrdiff_t>(windowIndex), window);
        m_floatingWindows.pop_back();
    }

    if (isBound)
    {
        m_boundWindows.push_back(window);
    }

    return false;
}

void ego::gui::SurfaceRoot::clearWindows()
{
    clearInteraction();

    while (getWindowCount() > 0)
    {
        const WindowPointer window = getWindow(0);
        const WindowPointer extractedWindow = extractWindow(window);
        if (!extractedWindow)
        {
            EGO_ASSERT_FAIL_MESSAGE("Failed to extract a window while clearing the surface.");
            break;
        }
    }

    m_boundWindows.clear();
}

bool ego::gui::SurfaceRoot::containsWindow(const WindowPointer& _window) const
{
    if (!_window)
    {
        return false;
    }

    if (isWindowFloating(_window))
    {
        return true;
    }

    return m_dockingArea && m_dockingArea->findWindowSpace(_window);
}

size_t ego::gui::SurfaceRoot::getWindowCount() const
{
    return (m_dockingArea ? m_dockingArea->getWindowCount() : 0) + m_floatingWindows.size();
}

ego::gui::WindowPointer ego::gui::SurfaceRoot::getWindow(size_t _index) const
{
    const size_t dockedWindowCount = m_dockingArea ? m_dockingArea->getWindowCount() : 0;
    if (_index < dockedWindowCount)
    {
        return m_dockingArea->getWindow(_index);
    }

    _index -= dockedWindowCount;

    return getFloatingWindow(_index);
}

size_t ego::gui::SurfaceRoot::getFloatingWindowCount() const
{
    return m_floatingWindows.size();
}

ego::gui::WindowPointer ego::gui::SurfaceRoot::getFloatingWindow(size_t _index) const
{
    return _index < m_floatingWindows.size() ? m_floatingWindows[_index] : nullptr;
}

bool ego::gui::SurfaceRoot::isWindowBound(const WindowPointer& _window) const
{
    if (!_window)
    {
        return false;
    }

    return std::find_if(
               m_boundWindows.begin(),
               m_boundWindows.end(),
               [&_window](const WindowWeakPointer& _boundWindow)
               {
                   const WindowPointer boundWindow = _boundWindow.lock();

                   return boundWindow.get() == _window.get();
               }) != m_boundWindows.end();
}

bool ego::gui::SurfaceRoot::isWindowFloating(const WindowPointer& _window) const
{
    return _window && std::find_if(
                          m_floatingWindows.begin(),
                          m_floatingWindows.end(),
                          [&_window](const WindowPointer& _floatingWindow)
                          {
                              return _floatingWindow.get() == _window.get();
                          }) != m_floatingWindows.end();
}

bool ego::gui::SurfaceRoot::setDockingEnabled(bool _isEnabled)
{
    if (isDockingEnabled() == _isEnabled)
    {
        return true;
    }

    clearInteraction();

    if (_isEnabled)
    {
        if (!m_dockingArea || !m_dockingArea->initializeRoot())
        {
            return false;
        }

        invalidateLayout();

        return true;
    }

    while (m_dockingArea && m_dockingArea->getWindowCount() > 0)
    {
        const WindowPointer window = m_dockingArea->getWindow(0);
        const WindowPointer undockedWindow = m_dockingArea->undockWindow(window);
        const bool attached = undockedWindow && attachFloatingWindow(undockedWindow);
        EGO_ASSERT(attached);
        if (!attached)
        {
            break;
        }
    }

    const bool rootCleared = !m_dockingArea || m_dockingArea->clearRoot();
    EGO_ASSERT(rootCleared);
    invalidateLayout();

    return true;
}

bool ego::gui::SurfaceRoot::isDockingEnabled() const
{
    return m_dockingArea && m_dockingArea->getRootNode();
}

ego::gui::DockingSpaceID ego::gui::SurfaceRoot::getDefaultDockingSpaceID() const
{
    const DockingSpacePointer space = isDockingEnabled() ? m_dockingArea->getDefaultSpace() : nullptr;

    return space ? space->getID() : InvalidDockingSpaceID;
}

ego::gui::DockingSpaceID ego::gui::SurfaceRoot::getWindowDockingSpaceID(const WindowPointer& _window) const
{
    const DockingSpacePointer space = _window && isDockingEnabled() ? m_dockingArea->findWindowSpace(_window) : nullptr;

    return space ? space->getID() : InvalidDockingSpaceID;
}

bool ego::gui::SurfaceRoot::dockWindow(const WindowPointer& _window, const DockingLocation& _location)
{
    const WindowPointer window = _window;
    if (!containsWindow(window))
    {
        return false;
    }

    if (_location.m_spaceID == InvalidDockingSpaceID)
    {
        return makeWindowFloating(window, window->getFloatingBounds());
    }

    if (!isDockingEnabled())
    {
        return false;
    }

    switch (_location.m_placement)
    {
    case DockingPlacement::Center:
    case DockingPlacement::Left:
    case DockingPlacement::Right:
    case DockingPlacement::Top:
    case DockingPlacement::Bottom:
        break;

    default:
        return false;
    }

    if (!std::isfinite(_location.m_splitRatio))
    {
        return false;
    }

    const DockingSpacePointer targetSpace = m_dockingArea->findSpace(_location.m_spaceID);
    if (!targetSpace)
    {
        return false;
    }

    const bool wasFloating = isWindowFloating(window);
    if (wasFloating && !detachFloatingWindow(window))
    {
        return false;
    }

    if (m_dockingArea->dockWindow(window, targetSpace, _location.m_placement, _location.m_splitRatio))
    {
        invalidateLayout();

        return true;
    }

    if (wasFloating)
    {
        const bool restored = attachFloatingWindow(window);
        EGO_ASSERT(restored);
    }

    return false;
}

bool ego::gui::SurfaceRoot::dockWindowToRoot(const WindowPointer& _window, DockingPlacement _placement, float _ratio)
{
    const WindowPointer window = _window;
    if (!containsWindow(window) || !isDockingEnabled())
    {
        return false;
    }

    switch (_placement)
    {
    case DockingPlacement::Left:
    case DockingPlacement::Right:
    case DockingPlacement::Top:
    case DockingPlacement::Bottom:
        break;

    default:
        return false;
    }

    if (!std::isfinite(_ratio))
    {
        return false;
    }

    const bool wasFloating = isWindowFloating(window);
    if (wasFloating && !detachFloatingWindow(window))
    {
        return false;
    }

    if (m_dockingArea->dockWindowToRoot(window, _placement, _ratio))
    {
        invalidateLayout();

        return true;
    }

    if (wasFloating)
    {
        const bool restored = attachFloatingWindow(window);
        EGO_ASSERT(restored);
    }

    return false;
}

bool ego::gui::SurfaceRoot::makeWindowFloating(const WindowPointer& _window, const Rect& _bounds)
{
    const WindowPointer window = _window;
    if (!containsWindow(window))
    {
        return false;
    }

    if (!isWindowFloating(window))
    {
        const WindowPointer undockedWindow = m_dockingArea ? m_dockingArea->undockWindow(window) : nullptr;
        if (!undockedWindow || !attachFloatingWindow(undockedWindow))
        {
            return false;
        }
    }

    window->setBounds(_bounds);
    moveFloatingWindowToFront(window);
    invalidateLayout();

    return true;
}

void ego::gui::SurfaceRoot::clearInteraction()
{
    if (m_dockingArea)
    {
        m_dockingArea->clearInteraction();
    }

    clearDockingPreview();
}

ego::gui::DockingPreview ego::gui::SurfaceRoot::buildDockingPreview(const WindowPointer& _window, const Position& _position) const
{
    const DockingNodePointer dockingRoot = m_dockingArea ? m_dockingArea->getRootNode() : nullptr;
    const DockingSpacePointer targetSpace = m_dockingArea ? m_dockingArea->findSpaceAt(_position) : nullptr;
    const DockingMetrics rootMetrics = dockingRoot ? dockingRoot->measure(DockingMeasureContext()) : DockingMetrics();
    const bool showRootTargets = rootMetrics.m_spaceCount > 1;
    if (!targetSpace && (!showRootTargets || !getLayoutBounds().contains(_position)))
    {
        return DockingPreview();
    }

    const WindowPointer dragWindow = _window;
    const DockingSpacePointer dragSpace = m_dockingArea ? m_dockingArea->findWindowSpace(dragWindow) : nullptr;
    const bool removesOriginSpace = dragSpace && dragWindow && !dragSpace->hasNonCollapsedWindowExcept(*dragWindow);
    const DockingSpaceID targetSpaceID = targetSpace ? targetSpace->getID() : InvalidDockingSpaceID;
    const Rect targetSpaceBounds = m_dockingArea ? m_dockingArea->getDockingBounds(targetSpace) : Rect();
    DockingMeasureContext rootMeasureContext{
        .m_minimumSpaceSize = m_dockingStyle.m_minimumSpaceSize,
        .m_separatorThickness = m_dockingStyle.m_separatorThickness,
    };
    if (removesOriginSpace)
    {
        rootMeasureContext.m_excludedSpace = std::cref(*dragSpace);
    }

    const Size rootMinimumSize = dockingRoot ? dockingRoot->measure(rootMeasureContext).m_minimumSize : SizeZero;
    const bool canReuseOriginSpaceExtent = removesOriginSpace && targetSpace && dragSpace.get() != targetSpace.get();
    const bool allowHorizontalSpaceSplitAfterOriginRemoval = canReuseOriginSpaceExtent && m_dockingArea->isSplitAvailableAfterRemoving(
                                                                                              dragSpace,
                                                                                              targetSpace,
                                                                                              DockingAxis::Horizontal,
                                                                                              getLayoutBounds().m_size,
                                                                                              m_dockingStyle.m_minimumSpaceSize,
                                                                                              m_dockingStyle.m_separatorThickness);
    const bool allowVerticalSpaceSplitAfterOriginRemoval = canReuseOriginSpaceExtent && m_dockingArea->isSplitAvailableAfterRemoving(
                                                                                            dragSpace,
                                                                                            targetSpace,
                                                                                            DockingAxis::Vertical,
                                                                                            getLayoutBounds().m_size,
                                                                                            m_dockingStyle.m_minimumSpaceSize,
                                                                                            m_dockingStyle.m_separatorThickness);
    DockingPreview preview = DockingPreviewBuilder::Build(
        _position,
        targetSpaceID,
        targetSpaceBounds,
        getLayoutBounds(),
        rootMinimumSize,
        showRootTargets,
        allowHorizontalSpaceSplitAfterOriginRemoval,
        allowVerticalSpaceSplitAfterOriginRemoval,
        m_dockingStyle);

    const bool isOnlyTabInTarget =
        dragSpace && dragWindow && targetSpace && dragSpace.get() == targetSpace.get() && !targetSpace->hasNonCollapsedWindowExcept(*dragWindow);
    if (targetSpace && (targetSpace->measure(DockingMeasureContext()).m_spaceCount == 0 || isOnlyTabInTarget))
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

    return preview;
}

void ego::gui::SurfaceRoot::showDockingPreview(const DockingPreview& _preview)
{
    if (m_dockingOverlay)
    {
        m_dockingOverlay->setPreview(_preview);
    }
}

bool ego::gui::SurfaceRoot::applyDockingPreview(const WindowPointer& _window, const DockingPreview& _preview)
{
    if (!_window || !_preview.m_isDropAllowed)
    {
        return false;
    }

    if (_preview.m_targetLevel == DockingTargetLevel::Root)
    {
        return dockWindowToRoot(_window, _preview.m_placement, _preview.m_splitRatio);
    }

    return dockWindow(
        _window,
        DockingLocation{
            .m_spaceID = _preview.m_targetSpaceID,
            .m_placement = _preview.m_placement,
            .m_splitRatio = _preview.m_splitRatio,
        });
}

void ego::gui::SurfaceRoot::clearDockingPreview()
{
    if (m_dockingOverlay)
    {
        m_dockingOverlay->clearPreview();
    }
}

void ego::gui::SurfaceRoot::updateExternalDragPreview(const WindowPointer& _window, const Position& _position)
{
    if (_window)
    {
        showDockingPreview(buildDockingPreview(_window, _position));
    }
}

bool ego::gui::SurfaceRoot::dockExternalWindow(const WindowPointer& _window, const Position& _position)
{
    if (!_window)
    {
        clearDockingPreview();

        return false;
    }

    const DockingPreview preview = buildDockingPreview(_window, _position);
    const bool docked = applyDockingPreview(_window, preview);
    clearDockingPreview();

    return docked;
}

void ego::gui::SurfaceRoot::clearExternalDragPreview()
{
    clearDockingPreview();
}

void ego::gui::SurfaceRoot::bringWidgetToFront(const WidgetPointer& _widget)
{
    WidgetPointer widget = _widget;
    while (widget && widget.get() != this)
    {
        if (rtti::IsObjectBasedOn<Window>(*widget))
        {
            const WindowPointer window = ego::StaticPointerCast<Window>(widget);
            if (containsWindow(window))
            {
                moveFloatingWindowToFront(window);
                return;
            }
        }

        widget = widget->getParent();
    }
}

ego::gui::Size ego::gui::SurfaceRoot::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    m_dockingStyle = _context.getTheme().m_docking;
    m_dockingStyle.m_targetSize = (std::max)(0.0f, m_dockingStyle.m_targetSize);
    m_dockingStyle.m_targetSpacing = (std::max)(0.0f, m_dockingStyle.m_targetSpacing);
    m_dockingStyle.m_separatorThickness = (std::max)(0.0f, m_dockingStyle.m_separatorThickness);
    m_dockingStyle.m_tabHeight = (std::max)(0.0f, m_dockingStyle.m_tabHeight);
    m_dockingStyle.m_dragThreshold = (std::max)(0.0f, m_dockingStyle.m_dragThreshold);
    m_dockingStyle.m_minimumSpaceSize = Size((std::max)(0.0f, m_dockingStyle.m_minimumSpaceSize.m_x), (std::max)(0.0f, m_dockingStyle.m_minimumSpaceSize.m_y));

    if (m_dockingArea)
    {
        _context.measure(*m_dockingArea, _constraints);
    }

    for (const WindowPointer& window : m_floatingWindows)
    {
        if (window && window->getParent().get() == this)
        {
            _context.measure(*window, LayoutConstraints(window->getFloatingBounds().m_size));
        }
    }

    if (m_dockingOverlay)
    {
        _context.measure(*m_dockingOverlay, _constraints);
    }

    return _constraints.m_maximumSize;
}

void ego::gui::SurfaceRoot::updateGeometry(const LayoutContext& _context)
{
    const Rect& surfaceBounds = getLayoutBounds();
    if (m_dockingArea)
    {
        _context.arrange(*m_dockingArea, surfaceBounds);
    }

    for (const WindowPointer& window : m_floatingWindows)
    {
        if (window && window->getParent().get() == this)
        {
            _context.arrange(*window, window->getFloatingBounds());
        }
    }

    if (m_dockingOverlay)
    {
        _context.arrange(*m_dockingOverlay, surfaceBounds);
    }
}

bool ego::gui::SurfaceRoot::attachFloatingWindow(const WindowPointer& _window)
{
    if (!_window || !attachChild(_window))
    {
        return false;
    }

    m_floatingWindows.push_back(_window);
    notifyTreeChanged();

    return true;
}

bool ego::gui::SurfaceRoot::detachFloatingWindow(const WindowPointer& _window)
{
    const WindowCollection::iterator windowIt = std::find_if(
        m_floatingWindows.begin(),
        m_floatingWindows.end(),
        [&_window](const WindowPointer& _currentWindow)
        {
            return _currentWindow.get() == _window.get();
        });
    if (windowIt == m_floatingWindows.end() || !_window || !detachChild(_window))
    {
        return false;
    }

    m_floatingWindows.erase(windowIt);
    notifyTreeChanged();

    return true;
}

void ego::gui::SurfaceRoot::moveFloatingWindowToFront(const WindowPointer& _window)
{
    const WindowCollection::iterator windowIt = std::find_if(
        m_floatingWindows.begin(),
        m_floatingWindows.end(),
        [&_window](const WindowPointer& _currentWindow)
        {
            return _currentWindow.get() == _window.get();
        });
    if (windowIt == m_floatingWindows.end() || windowIt == m_floatingWindows.end() - 1)
    {
        return;
    }

    const WindowPointer window = *windowIt;
    m_floatingWindows.erase(windowIt);
    m_floatingWindows.push_back(window);
    notifyTreeChanged();
}

void ego::gui::SurfaceRoot::removeWindowBinding(const WindowPointer& _window)
{
    m_boundWindows.erase(
        std::remove_if(
            m_boundWindows.begin(),
            m_boundWindows.end(),
            [&_window](const WindowWeakPointer& _boundWindow)
            {
                const WindowPointer boundWindow = _boundWindow.lock();

                return !boundWindow || boundWindow.get() == _window.get();
            }),
        m_boundWindows.end());
}

size_t ego::gui::SurfaceRoot::getChildCount() const
{
    return (m_dockingArea ? 1 : 0) + m_floatingWindows.size() + (m_dockingOverlay ? 1 : 0);
}

ego::gui::WidgetPointer ego::gui::SurfaceRoot::getChild(size_t _index) const
{
    if (m_dockingArea)
    {
        if (_index == 0)
        {
            return m_dockingArea;
        }

        --_index;
    }

    if (_index < m_floatingWindows.size())
    {
        return m_floatingWindows[_index];
    }

    _index -= m_floatingWindows.size();

    return _index == 0 ? m_dockingOverlay : nullptr;
}
