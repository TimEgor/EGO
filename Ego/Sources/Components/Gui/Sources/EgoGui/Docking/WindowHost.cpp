#include "WindowHost.h"

#include <algorithm>
#include <vector>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoGui/Rendering/PaintContext.h"
#include "EgoGui/Theme/Theme.h"
#include "EgoGui/Viewport/SurfaceRoot.h"

bool ego::gui::WindowHost::WindowHostAccessor::IsInteractionActive(const WindowHost& _host)
{
    return _host.isInteractionActive();
}

ego::gui::WindowHost::WindowHost(const SurfaceRootPointer& _root)
    : m_root(_root),
      m_style(Theme::GetDefault().m_docking)
{
}

ego::gui::WindowHost::~WindowHost()
{
    clearWidgets();
    m_isDockingEnabled = false;
}

ego::gui::WindowHostPointer ego::gui::WindowHost::Create(const SurfaceRootPointer& _root)
{
    return _root ? WindowHostPointer(new WindowHost(_root)) : nullptr;
}

bool ego::gui::WindowHost::addWidget(const WidgetPointer& _widget)
{
    if (!canMutateTree() || !attachChild(_widget))
    {
        return false;
    }

    m_widgets.push_back(_widget);
    const WindowPointer window = GetWindow(_widget);
    if (m_isDockingEnabled && window)
    {
        bindWindowDrag(window);
    }

    notifyTreeChanged();
    return true;
}

ego::gui::WidgetPointer ego::gui::WindowHost::removeWidget(const WidgetPointer& _widget)
{
    if (!canMutateTree())
    {
        return nullptr;
    }

    const WidgetCollection::iterator widgetIt = std::find_if(
        m_widgets.begin(),
        m_widgets.end(),
        [&_widget](const WidgetPointer& _currentWidget)
        {
            return _currentWidget.get() == _widget.get();
        });
    if (widgetIt == m_widgets.end())
    {
        return nullptr;
    }

    const WidgetPointer widget = *widgetIt;
    if (!detachChild(widget))
    {
        return nullptr;
    }

    const WindowPointer window = GetWindow(widget);
    if (m_isDockingEnabled && window)
    {
        releaseWindow(window);
    }

    m_widgets.erase(widgetIt);
    notifyTreeChanged();
    queueWindowNotification(window);

    return widget;
}

void ego::gui::WindowHost::clearWidgets()
{
    if (!canMutateTree())
    {
        return;
    }

    if (m_isDockingEnabled)
    {
        releaseDocking();
    }

    if (m_widgets.empty())
    {
        return;
    }

    WidgetCollection widgets;
    widgets.swap(m_widgets);
    for (const WidgetPointer& widget : widgets)
    {
        detachChild(widget);
    }

    notifyTreeChanged();

    for (const WidgetPointer& widget : widgets)
    {
        queueWindowNotification(GetWindow(widget));
    }
}

void ego::gui::WindowHost::bringWidgetToFront(const WidgetPointer& _widget)
{
    if (!canMutateTree())
    {
        return;
    }

    WidgetPointer hostedWidget = _widget;
    while (hostedWidget && !hostedWidget->isDirectChildOf(*this))
    {
        hostedWidget = hostedWidget->getParent();
    }

    if (!hostedWidget)
    {
        return;
    }

    moveWidgetToFront(hostedWidget);
}

const ego::gui::WindowHost::WidgetCollection& ego::gui::WindowHost::getWidgets() const
{
    return m_widgets;
}

bool ego::gui::WindowHost::flushWindowNotifications()
{
    bool flushed = false;
    std::vector<WindowPointer> pendingWindows;
    pendingWindows.swap(m_pendingWindowNotifications);
    const WidgetCollection widgets = m_widgets;
    for (const WidgetPointer& widget : widgets)
    {
        const WindowPointer window = GetWindow(widget);
        if (window)
        {
            flushed = Window::WindowAccessor::FlushSizeChanged(*window) || flushed;
        }
    }

    for (const WindowPointer& window : pendingWindows)
    {
        if (window)
        {
            flushed = Window::WindowAccessor::FlushSizeChanged(*window) || flushed;
        }
    }

    return flushed;
}

bool ego::gui::WindowHost::setDockingEnabled(bool _isEnabled)
{
    if (!canMutateTree())
    {
        return false;
    }

    if (m_isDockingEnabled == _isEnabled)
    {
        return true;
    }

    if (!_isEnabled)
    {
        releaseDocking();
        m_isDockingEnabled = false;
        invalidateLayout();
        return true;
    }

    for (const WidgetPointer& widget : m_widgets)
    {
        const WindowPointer window = GetWindow(widget);
        if (window)
        {
            bindWindowDrag(window);
        }
    }

    m_isDockingEnabled = true;
    invalidateLayout();
    return true;
}

bool ego::gui::WindowHost::isDockingEnabled() const
{
    return m_isDockingEnabled;
}

ego::gui::DockingSpaceID ego::gui::WindowHost::getDefaultSpaceID() const
{
    return m_isDockingEnabled ? m_layout.getDefaultSpaceID() : InvalidDockingSpaceID;
}

ego::gui::DockingSpaceID ego::gui::WindowHost::getWindowSpaceID(const WindowPointer& _window) const
{
    return m_isDockingEnabled && _window && findHostedWindow(*_window) ? m_layout.findWindowSpace(_window) : InvalidDockingSpaceID;
}

bool ego::gui::WindowHost::placeWindow(const WindowPointer& _window, const WindowPlacement& _placement)
{
    if (!canMutateTree() || !_window || !findHostedWindow(*_window))
    {
        return false;
    }

    const WindowPointer dragWindow = m_drag.m_window.lock();
    const bool isInteractingWindow = dragWindow.get() == _window.get();

    if (_placement.m_spaceID == InvalidDockingSpaceID)
    {
        if (m_layout.isDocked(_window) && !m_layout.undockWindow(_window))
        {
            return false;
        }

        Window::WindowAccessor::ClearDockedBounds(*_window);
        moveWidgetToFront(_window);
        if (isInteractingWindow)
        {
            clearInteraction();
        }

        invalidateLayout();

        return true;
    }

    if (!m_isDockingEnabled)
    {
        return false;
    }

    if (!m_layout.dockWindow(_window, _placement.m_spaceID, _placement.m_placement, _placement.m_splitRatio))
    {
        return false;
    }

    moveDockedWindowsToBack();
    if (isInteractingWindow)
    {
        clearInteraction();
    }

    invalidateLayout();

    return true;
}

void ego::gui::WindowHost::bindWindowDrag(const WindowPointer& _window)
{
    const WindowHostPointer self = ego::StaticPointerCast<WindowHost>(sharedFromThis());
    const WindowDragDelegatePointer delegate = self;
    Window::WindowAccessor::BindDragDelegate(*_window, WindowDragDelegateWeakPointer(delegate));
}

void ego::gui::WindowHost::releaseWindow(const WindowPointer& _window)
{
    if (!_window)
    {
        return;
    }

    const WindowPointer dragWindow = m_drag.m_window.lock();
    if (dragWindow.get() == _window.get())
    {
        clearInteraction();
    }

    m_layout.undockWindow(_window);
    Window::WindowAccessor::ClearDockedBounds(*_window);
    Window::WindowAccessor::BindDragDelegate(*_window, WindowDragDelegateWeakPointer());
}

void ego::gui::WindowHost::releaseDocking()
{
    clearInteraction();
    for (const WidgetPointer& widget : m_widgets)
    {
        const WindowPointer window = GetWindow(widget);
        if (window)
        {
            Window::WindowAccessor::ClearDockedBounds(*window);
            Window::WindowAccessor::BindDragDelegate(*window, WindowDragDelegateWeakPointer());
        }
    }

    m_layout.clear();
    m_arrangement = DockingLayout::Arrangement();
    m_dockingBounds = Rect();
    m_tabs.clear();
}

void ego::gui::WindowHost::queueWindowNotification(const WindowPointer& _window)
{
    if (!_window)
    {
        return;
    }

    const std::vector<WindowPointer>::const_iterator windowIt = std::find_if(
        m_pendingWindowNotifications.begin(),
        m_pendingWindowNotifications.end(),
        [&_window](const WindowPointer& _pendingWindow)
        {
            return _pendingWindow.get() == _window.get();
        });
    if (windowIt == m_pendingWindowNotifications.end())
    {
        m_pendingWindowNotifications.push_back(_window);
    }
}

ego::gui::WindowPointer ego::gui::WindowHost::GetWindow(const WidgetPointer& _widget)
{
    return _widget && rtti::IsObjectBasedOn<Window>(*_widget) ? ego::StaticPointerCast<Window>(_widget) : nullptr;
}

void ego::gui::WindowHost::moveDockedWindowsToBack()
{
    if (!m_isDockingEnabled)
    {
        return;
    }

    std::stable_partition(
        m_widgets.begin(),
        m_widgets.end(),
        [this](const WidgetPointer& _widget)
        {
            const WindowPointer window = GetWindow(_widget);
            return window && m_layout.isDocked(window);
        });
}

void ego::gui::WindowHost::moveWidgetToFront(const WidgetPointer& _widget)
{
    const WidgetCollection::iterator widgetIt = std::find_if(
        m_widgets.begin(),
        m_widgets.end(),
        [&_widget](const WidgetPointer& _currentWidget)
        {
            return _currentWidget.get() == _widget.get();
        });
    if (widgetIt == m_widgets.end())
    {
        return;
    }

    const WidgetPointer widget = *widgetIt;
    const WindowPointer window = GetWindow(widget);
    const bool isDocked = m_isDockingEnabled && window && m_layout.isDocked(window);
    if (isDocked)
    {
        return;
    }

    if (widgetIt == m_widgets.end() - 1)
    {
        return;
    }

    m_widgets.erase(widgetIt);
    m_widgets.push_back(widget);

    notifyTreeChanged();
}

ego::gui::WindowPointer ego::gui::WindowHost::findHostedWindow(const Window& _window) const
{
    for (const WidgetPointer& widget : m_widgets)
    {
        const WindowPointer window = GetWindow(widget);
        if (window.get() == &_window)
        {
            return window;
        }
    }

    return nullptr;
}

void ego::gui::WindowHost::updateDockingLayout(const Rect& _bounds, const DockingStyle& _style)
{
    const WindowPointer dragWindow = m_drag.m_window.lock();
    if (dragWindow && !dragWindow->isVisible())
    {
        clearInteraction();
    }

    m_style = _style;
    m_style.m_targetSize = (std::max)(0.0f, m_style.m_targetSize);
    m_style.m_targetSpacing = (std::max)(0.0f, m_style.m_targetSpacing);
    m_style.m_separatorThickness = (std::max)(0.0f, m_style.m_separatorThickness);
    m_style.m_tabHeight = (std::max)(0.0f, m_style.m_tabHeight);
    m_style.m_dragThreshold = (std::max)(0.0f, m_style.m_dragThreshold);
    m_style.m_minimumSpaceSize = Size((std::max)(0.0f, m_style.m_minimumSpaceSize.m_x), (std::max)(0.0f, m_style.m_minimumSpaceSize.m_y));

    m_dockingBounds = _bounds;
    m_arrangement = m_layout.arrange(_bounds, m_style.m_minimumSpaceSize, m_style.m_separatorThickness);
    m_tabs.clear();

    for (const DockingLayout::Space& space : m_arrangement.m_spaces)
    {
        updateDockingSpace(space);
    }

    for (const WidgetPointer& widget : m_widgets)
    {
        const WindowPointer window = GetWindow(widget);
        if (window && !m_layout.isDocked(window))
        {
            Window::WindowAccessor::ClearDockedBounds(*window);
        }
    }

    if (m_drag.m_hasPassedThreshold)
    {
        updatePreview(m_drag.m_position);
    }
}

void ego::gui::WindowHost::updateDockingSpace(const DockingLayout::Space& _space)
{
    if (_space.m_windows.empty())
    {
        return;
    }

    DockingLayout::WindowCollection visibleWindows;
    for (const WindowPointer& window : _space.m_windows)
    {
        if (window && window->isVisible())
        {
            visibleWindows.push_back(window);
        }
    }

    WindowPointer selectedWindow = _space.m_selectedWindow;
    if (selectedWindow && !selectedWindow->isVisible())
    {
        selectedWindow = nullptr;
    }

    if (!selectedWindow && !visibleWindows.empty())
    {
        selectedWindow = visibleWindows.front();
        m_layout.selectWindow(selectedWindow);
    }

    const float tabHeight = (std::min)(m_style.m_tabHeight, _space.m_bounds.m_size.m_y);
    const Rect tabBarBounds(_space.m_bounds.m_position, Size(_space.m_bounds.m_size.m_x, tabHeight));
    const Rect contentBounds(
        _space.m_bounds.m_position.m_x,
        _space.m_bounds.m_position.m_y + tabHeight,
        _space.m_bounds.m_size.m_x,
        (std::max)(0.0f, _space.m_bounds.m_size.m_y - tabHeight));

    for (const WindowPointer& window : _space.m_windows)
    {
        if (!window)
        {
            continue;
        }

        const bool isSelected = window->isVisible() && window.get() == selectedWindow.get();
        Window::WindowAccessor::SetDockedBounds(*window, contentBounds, isSelected);
    }

    if (visibleWindows.empty())
    {
        return;
    }

    const float tabWidth = tabBarBounds.m_size.m_x / static_cast<float>(visibleWindows.size());
    for (size_t windowIndex = 0; windowIndex < visibleWindows.size(); ++windowIndex)
    {
        const WindowPointer& window = visibleWindows[windowIndex];
        const Rect tabBounds(
            tabBarBounds.m_position.m_x + tabWidth * static_cast<float>(windowIndex),
            tabBarBounds.m_position.m_y,
            tabWidth,
            tabBarBounds.m_size.m_y);
        m_tabs.push_back(
            {
                .m_window = window,
                .m_bounds = tabBounds,
                .m_isSelected = window.get() == selectedWindow.get(),
            });
    }
}

ego::gui::Size ego::gui::WindowHost::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    const Rect hostBounds(PositionZero, _constraints.m_maximumSize);
    if (m_isDockingEnabled)
    {
        updateDockingLayout(hostBounds, _context.getTheme().m_docking);
    }

    for (const WidgetPointer& widget : m_widgets)
    {
        if (!widget)
        {
            continue;
        }

        const Rect widgetBounds = widget->resolveTopLevelBounds(hostBounds);
        widget->updatePreferredSize(_context, LayoutConstraints(widgetBounds.m_size));
    }

    return _constraints.m_maximumSize;
}

void ego::gui::WindowHost::updateGeometry(const LayoutContext& _context)
{
    const Rect& hostBounds = getLayoutBounds();
    for (const WidgetPointer& widget : m_widgets)
    {
        if (widget)
        {
            widget->applyLayout(_context, widget->resolveTopLevelBounds(hostBounds));
        }
    }
}

void ego::gui::WindowHost::clearInteraction()
{
    m_hoveredTabWindow.reset();
    m_hoveredSplitID = InvalidDockingSplitID;
    m_activeSplitID = InvalidDockingSplitID;
    m_drag = DragState();
    m_preview = DockingPreview();
    invalidateLayout();
}

bool ego::gui::WindowHost::isInteractionActive() const
{
    return m_activeSplitID != InvalidDockingSplitID || m_drag.m_origin != DragOrigin::None;
}

bool ego::gui::WindowHost::isDraggingWindow(const Window& _window, DragOrigin _origin) const
{
    const WindowPointer dragWindow = m_drag.m_window.lock();
    return m_drag.m_origin == _origin && dragWindow.get() == &_window;
}

bool ego::gui::WindowHost::isInteractionAffectedByPlacement(const WindowPointer& _window, const WindowPlacement& _placement) const
{
    if (!_window || !isInteractionActive())
    {
        return false;
    }

    const WindowPointer dragWindow = m_drag.m_window.lock();
    if (dragWindow.get() == _window.get())
    {
        return true;
    }

    return _placement.m_spaceID != InvalidDockingSpaceID || m_layout.isDocked(_window);
}

bool ego::gui::WindowHost::isDockChromeUnoccluded(const Position& _position) const
{
    const SurfaceRootPointer root = m_root.lock();
    if (!root)
    {
        return false;
    }

    const WidgetPointer hitWidget = root->findWidgetAt(_position);
    if (!hitWidget)
    {
        return false;
    }

    if (hitWidget.get() == this || hitWidget.get() == root.get())
    {
        return true;
    }

    WidgetPointer hostedWidget = hitWidget;
    while (hostedWidget && !hostedWidget->isDirectChildOf(*this))
    {
        hostedWidget = hostedWidget->getParent();
    }

    if (!hostedWidget)
    {
        return false;
    }

    const WindowPointer window = GetWindow(hostedWidget);
    return window && m_layout.isDocked(window);
}

ego::gui::DockingSplitID ego::gui::WindowHost::findSplitAt(const Position& _position) const
{
    if (!isDockChromeUnoccluded(_position))
    {
        return InvalidDockingSplitID;
    }

    for (const DockingSeparator& separator : m_arrangement.m_separators)
    {
        if (separator.m_bounds.contains(_position))
        {
            return separator.m_splitID;
        }
    }

    return InvalidDockingSplitID;
}

ego::gui::WindowPointer ego::gui::WindowHost::findTabAt(const Position& _position) const
{
    if (!isDockChromeUnoccluded(_position))
    {
        return nullptr;
    }

    for (const ArrangedTab& tab : m_tabs)
    {
        if (!tab.m_bounds.contains(_position))
        {
            continue;
        }

        return tab.m_window.lock();
    }

    return nullptr;
}

void ego::gui::WindowHost::updateActiveSplit(const Position& _position)
{
    for (const DockingSeparator& separator : m_arrangement.m_separators)
    {
        if (separator.m_splitID != m_activeSplitID)
        {
            continue;
        }

        const bool horizontal = separator.m_axis == DockingSplitAxis::Horizontal;
        const float totalExtent = horizontal ? separator.m_splitBounds.m_size.m_x : separator.m_splitBounds.m_size.m_y;
        const float separatorExtent = horizontal ? separator.m_bounds.m_size.m_x : separator.m_bounds.m_size.m_y;
        const float availableExtent = (std::max)(0.0f, totalExtent - separatorExtent);
        if (availableExtent <= 0.0f)
        {
            return;
        }

        const float pointerOffset =
            horizontal ? _position.m_x - separator.m_splitBounds.m_position.m_x : _position.m_y - separator.m_splitBounds.m_position.m_y;
        float minimumRatio = separator.m_firstMinimumExtent / availableExtent;
        float maximumRatio = 1.0f - separator.m_secondMinimumExtent / availableExtent;
        if (minimumRatio > maximumRatio)
        {
            const float totalMinimumExtent = separator.m_firstMinimumExtent + separator.m_secondMinimumExtent;
            minimumRatio = totalMinimumExtent > 0.0f ? separator.m_firstMinimumExtent / totalMinimumExtent : 0.5f;
            maximumRatio = minimumRatio;
        }

        const float ratio = (std::clamp)((pointerOffset - separatorExtent * 0.5f) / availableExtent, minimumRatio, maximumRatio);
        m_layout.setSplitRatio(separator.m_splitID, ratio);
        return;
    }
}

ego::gui::InputReply ego::gui::WindowHost::onPointerMove(const PointerMoveEvent& _event)
{
    if (!m_isDockingEnabled)
    {
        return InputReply::Unhandled;
    }

    if (m_activeSplitID != InvalidDockingSplitID)
    {
        m_hoveredTabWindow.reset();
        m_hoveredSplitID = m_activeSplitID;
        updateActiveSplit(_event.m_position);
        invalidateLayout();
        return InputReply::Handled;
    }

    m_hoveredSplitID = findSplitAt(_event.m_position);
    const WindowPointer hoveredTabWindow = findTabAt(_event.m_position);
    m_hoveredTabWindow = hoveredTabWindow;

    if (m_drag.m_origin != DragOrigin::Tab)
    {
        if (m_hoveredSplitID != InvalidDockingSplitID)
        {
            m_hoveredTabWindow.reset();
        }

        return InputReply::Unhandled;
    }

    m_hoveredTabWindow.reset();
    updateDrag(_event.m_position);
    invalidateLayout();
    return InputReply::Handled;
}

ego::gui::InputReply ego::gui::WindowHost::onMouseButton(const MouseButtonEvent& _event)
{
    if (!m_isDockingEnabled || _event.m_key != MouseInputKey::ButtonLeft)
    {
        return InputReply::Unhandled;
    }

    if (_event.m_action == InputButtonAction::Pressed)
    {
        const DockingSplitID splitID = findSplitAt(_event.m_position);
        if (splitID != InvalidDockingSplitID)
        {
            m_activeSplitID = splitID;
            m_hoveredSplitID = splitID;
            m_hoveredTabWindow.reset();
            invalidateLayout();
            return InputReply::Capture;
        }

        const WindowPointer tabWindow = findTabAt(_event.m_position);
        if (!tabWindow)
        {
            return InputReply::Unhandled;
        }

        m_layout.selectWindow(tabWindow);
        beginTabInteraction(tabWindow, _event.m_position);
        invalidateLayout();
        return InputReply::Capture;
    }

    if (m_activeSplitID != InvalidDockingSplitID)
    {
        updateActiveSplit(_event.m_position);
        m_activeSplitID = InvalidDockingSplitID;
        m_hoveredSplitID = findSplitAt(_event.m_position);
        invalidateLayout();
        return InputReply::Handled;
    }

    if (m_drag.m_origin != DragOrigin::Tab)
    {
        return InputReply::Unhandled;
    }

    updateDrag(_event.m_position);
    if (m_drag.m_hasPassedThreshold)
    {
        finishDrag(_event.m_position);
    }
    else
    {
        clearInteraction();
    }

    return InputReply::Handled;
}

void ego::gui::WindowHost::onPointerLeave(const Position& _position, const InputModifiers&)
{
    m_drag.m_position = _position;
    m_hoveredSplitID = InvalidDockingSplitID;
    m_hoveredTabWindow.reset();
    m_preview = DockingPreview();
    invalidateLayout();
}

void ego::gui::WindowHost::onPointerCaptureLost(const Position&)
{
    clearInteraction();
}

void ego::gui::WindowHost::onWindowDragBegin(Window& _window, const Position& _position)
{
    const WindowPointer window = findHostedWindow(_window);
    if (!m_isDockingEnabled || !window)
    {
        return;
    }

    beginDrag(window, DragOrigin::FloatingWindow, _position, window->getBounds());
}

void ego::gui::WindowHost::onWindowDragMove(Window& _window, const Position& _position)
{
    if (m_isDockingEnabled && isDraggingWindow(_window, DragOrigin::FloatingWindow))
    {
        updateDrag(_position);
        invalidateLayout();
    }
}

void ego::gui::WindowHost::onWindowDragEnd(Window& _window, const Position& _position)
{
    if (m_isDockingEnabled && isDraggingWindow(_window, DragOrigin::FloatingWindow))
    {
        if (m_drag.m_hasPassedThreshold)
        {
            finishDrag(_position);
        }
        else
        {
            clearInteraction();
        }
    }
}

void ego::gui::WindowHost::onWindowDragCancel(Window& _window, const Position&)
{
    if (!isDraggingWindow(_window, DragOrigin::FloatingWindow))
    {
        return;
    }

    clearInteraction();
}

void ego::gui::WindowHost::beginTabInteraction(const WindowPointer& _window, const Position& _position)
{
    if (!m_layout.isDocked(_window))
    {
        EGO_ASSERT_FAIL_MESSAGE("Docking tab must belong to a docking space before it can be dragged.");
        return;
    }

    const Size requestedSize = Window::WindowAccessor::GetFloatingBounds(*_window).m_size;
    const Size floatingSize(
        requestedSize.m_x > 0.0f ? requestedSize.m_x : m_style.m_minimumSpaceSize.m_x,
        requestedSize.m_y > 0.0f ? requestedSize.m_y : m_style.m_minimumSpaceSize.m_y);
    const Position grabOffset(floatingSize.m_x * 0.5f, (std::min)(floatingSize.m_y, m_style.m_tabHeight) * 0.5f);
    const Rect floatingBounds(Position(_position.m_x - grabOffset.m_x, _position.m_y - grabOffset.m_y), floatingSize);

    beginDrag(_window, DragOrigin::Tab, _position, floatingBounds);
}

void ego::gui::WindowHost::beginDrag(const WindowPointer& _window, DragOrigin _origin, const Position& _position, const Rect& _floatingBounds)
{
    m_drag = DragState();
    m_drag.m_origin = _origin;
    m_drag.m_window = _window;
    m_drag.m_startPosition = _position;
    m_drag.m_position = _position;
    m_drag.m_startBounds = _floatingBounds;
    m_drag.m_floatingBounds = _floatingBounds;
}

void ego::gui::WindowHost::updateDrag(const Position& _position)
{
    const WindowPointer window = m_drag.m_window.lock();
    if (!window || m_drag.m_origin == DragOrigin::None || !getLayoutBounds().contains(_position))
    {
        return;
    }

    m_drag.m_position = _position;
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

    updatePreview(_position);
}

void ego::gui::WindowHost::finishDrag(const Position& _position)
{
    const WindowPointer window = m_drag.m_window.lock();
    if (window)
    {
        updatePreview(_position);
        if (m_preview.m_isDropAllowed)
        {
            const bool docked = m_preview.m_targetLevel == DockingTargetLevel::Root ?
                                    m_layout.dockWindowToRoot(window, m_preview.m_placement, m_preview.m_splitRatio) :
                                    m_layout.dockWindow(window, m_preview.m_targetSpaceID, m_preview.m_placement, m_preview.m_splitRatio);
            if (docked)
            {
                moveDockedWindowsToBack();
            }
        }
        else if (m_drag.m_origin == DragOrigin::Tab && !m_preview.m_isTargetHovered && m_layout.undockWindow(window))
        {
            Window::WindowAccessor::ClearDockedBounds(*window);
            window->setBounds(m_drag.m_floatingBounds);
            moveWidgetToFront(window);
        }
    }

    clearInteraction();
}

void ego::gui::WindowHost::updatePreview(const Position& _position)
{
    const DockingLayout::SpaceCollection::const_iterator targetSpaceIt = std::find_if(
        m_arrangement.m_spaces.begin(),
        m_arrangement.m_spaces.end(),
        [&_position](const DockingLayout::Space& _space)
        {
            return _space.m_bounds.contains(_position);
        });
    const bool showRootTargets = m_arrangement.m_spaces.size() > 1;
    if (targetSpaceIt == m_arrangement.m_spaces.end() && (!showRootTargets || !m_dockingBounds.contains(_position)))
    {
        m_preview = DockingPreview();
        return;
    }

    const bool hasTargetSpace = targetSpaceIt != m_arrangement.m_spaces.end();
    const DockingSpaceID targetSpaceID = hasTargetSpace ? targetSpaceIt->m_id : InvalidDockingSpaceID;
    const Rect targetSpaceBounds = hasTargetSpace ? targetSpaceIt->m_bounds : Rect();
    m_preview =
        DockingPreviewBuilder::Build(_position, targetSpaceID, targetSpaceBounds, m_dockingBounds, m_arrangement.m_minimumSize, showRootTargets, m_style);
    if (!hasTargetSpace)
    {
        return;
    }

    const DockingLayout::Space& targetSpace = *targetSpaceIt;

    const WindowPointer dragWindow = m_drag.m_window.lock();
    const bool isOnlyTabInTarget = dragWindow && m_layout.findWindowSpace(dragWindow) == targetSpace.m_id && targetSpace.m_windows.size() == 1;
    if (targetSpace.m_windows.empty() || isOnlyTabInTarget)
    {
        for (DockingTarget& target : m_preview.m_targets)
        {
            if (target.m_level == DockingTargetLevel::Space && target.m_placement != DockingPlacement::Center)
            {
                target.m_isAvailable = false;
            }
        }

        if (m_preview.m_targetLevel == DockingTargetLevel::Space && m_preview.m_placement != DockingPlacement::Center)
        {
            m_preview.m_isDropAllowed = false;
        }
    }
}

void ego::gui::WindowHost::drawBaseLayer(PaintContext& _context) const
{
    if (!m_isDockingEnabled)
    {
        return;
    }

    for (const DockingSeparator& separator : m_arrangement.m_separators)
    {
        const bool isHovered = separator.m_splitID == m_hoveredSplitID;
        const bool isPressed = separator.m_splitID == m_activeSplitID;
        _context.drawBox(separator.m_bounds, m_style.m_separator.resolve(isHovered, isPressed));
    }

    for (const DockingLayout::Space& space : m_arrangement.m_spaces)
    {
        if (space.m_windows.empty())
        {
            continue;
        }

        _context.drawBox(space.m_bounds, m_style.m_background);
        _context.drawBorder(space.m_bounds, 1.0f, m_style.m_spaceBorder);
    }

    const WindowPointer hoveredTabWindow = m_hoveredTabWindow.lock();
    for (const ArrangedTab& tab : m_tabs)
    {
        const WindowPointer window = tab.m_window.lock();
        if (!window)
        {
            continue;
        }

        const bool isHovered = hoveredTabWindow.get() == window.get();
        const NormalizedColorRGBA& tabColor = tab.m_isSelected ? m_style.m_tabActive : m_style.m_tab.resolve(isHovered, false);
        _context.drawBox(tab.m_bounds, tabColor);

        const float textWidth = (std::max)(0.0f, tab.m_bounds.m_size.m_x - m_style.m_tabPadding.getHorizontal());
        const float textHeight = (std::max)(0.0f, tab.m_bounds.m_size.m_y - m_style.m_tabPadding.getVertical());
        const Rect textBounds(
            tab.m_bounds.m_position.m_x + m_style.m_tabPadding.m_left,
            tab.m_bounds.m_position.m_y + m_style.m_tabPadding.m_top,
            textWidth,
            textHeight);
        _context.drawText(window->getTitle(), textBounds, _context.getTheme().m_typography.m_primary);
    }
}

void ego::gui::WindowHost::drawOverlayLayer(PaintContext& _context) const
{
    if (!m_isDockingEnabled)
    {
        return;
    }

    const WindowPointer dragWindow = m_drag.m_window.lock();
    if (m_drag.m_origin == DragOrigin::Tab && m_drag.m_hasPassedThreshold && dragWindow)
    {
        const WindowStyle& windowStyle = _context.getTheme().m_window;
        const Rect& floatingBounds = m_drag.m_floatingBounds;
        const float titleHeight = (std::min)((std::max)(0.0f, windowStyle.m_titleHeight), floatingBounds.m_size.m_y);
        const Rect titleBounds(floatingBounds.m_position, Size(floatingBounds.m_size.m_x, titleHeight));
        _context.drawBox(floatingBounds, windowStyle.m_surface);
        _context.drawBox(titleBounds, windowStyle.m_title.resolve(false, true));

        const Rect textBounds(
            titleBounds.m_position.m_x + windowStyle.m_titlePadding.m_left,
            titleBounds.m_position.m_y + windowStyle.m_titlePadding.m_top,
            (std::max)(0.0f, titleBounds.m_size.m_x - windowStyle.m_titlePadding.getHorizontal()),
            (std::max)(0.0f, titleBounds.m_size.m_y - windowStyle.m_titlePadding.getVertical()));
        _context.drawText(dragWindow->getTitle(), textBounds, _context.getTheme().m_typography.m_primary);
    }

    if (!m_preview.m_isVisible)
    {
        return;
    }

    if (m_preview.m_isDropAllowed)
    {
        _context.drawBox(m_preview.m_previewBounds, m_style.m_previewFill);
    }

    for (const DockingTarget& target : m_preview.m_targets)
    {
        if (!target.m_isAvailable)
        {
            continue;
        }

        const bool hovered = m_preview.m_isDropAllowed && target.m_level == m_preview.m_targetLevel && target.m_placement == m_preview.m_placement;
        const NormalizedColorRGBA& color = m_style.m_target.resolve(hovered, false);
        DrawTarget(_context, target.m_bounds, target.m_placement, color, m_style.m_targetBorder);
    }
}

void ego::gui::WindowHost::DrawTarget(
    PaintContext& _context,
    const Rect& _bounds,
    DockingPlacement _placement,
    const NormalizedColorRGBA& _fillColor,
    const NormalizedColorRGBA& _borderColor)
{
    constexpr float BorderThickness = 2.0f;
    _context.drawBox(_bounds, _fillColor);
    _context.drawBorder(_bounds, BorderThickness, _borderColor);

    const float centerX = _bounds.m_position.m_x + _bounds.m_size.m_x * 0.5f;
    const float centerY = _bounds.m_position.m_y + _bounds.m_size.m_y * 0.5f;
    if (_placement == DockingPlacement::Left || _placement == DockingPlacement::Right)
    {
        _context.drawBox(Rect(centerX - BorderThickness * 0.5f, _bounds.getTop(), BorderThickness, _bounds.m_size.m_y), _borderColor);
    }
    else if (_placement == DockingPlacement::Top || _placement == DockingPlacement::Bottom)
    {
        _context.drawBox(Rect(_bounds.getLeft(), centerY - BorderThickness * 0.5f, _bounds.m_size.m_x, BorderThickness), _borderColor);
    }
}

size_t ego::gui::WindowHost::getChildCount() const
{
    return m_widgets.size();
}

const ego::gui::WidgetPointer& ego::gui::WindowHost::getChild(size_t _index) const
{
    return m_widgets[_index];
}
