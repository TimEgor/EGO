#include "SurfaceRoot.h"

#include <algorithm>
#include <cmath>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoGui/Docking/DockingArea.h"
#include "EgoGui/Docking/DockingOverlay.h"
#include "EgoGui/Docking/DockingSpace.h"
#include "EgoGui/Widgets/Window.h"

ego::gui::SurfaceRoot::~SurfaceRoot() = default;

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

bool ego::gui::SurfaceRoot::addWindow(const WindowPointer& _window)
{
    const WindowPointer window = _window;

    return window && !isHostedWindow(window) && attachFloatingWindow(window);
}

ego::gui::WindowPointer ego::gui::SurfaceRoot::removeWindow(const WindowPointer& _window)
{
    const WindowPointer window = _window;
    if (!window)
    {
        return nullptr;
    }

    if (m_dockingOverlay)
    {
        m_dockingOverlay->cancelDrag(window);
    }

    if (detachFloatingWindow(window))
    {
        return window;
    }

    return m_dockingArea ? m_dockingArea->undockWindow(window) : nullptr;
}

void ego::gui::SurfaceRoot::clearWindows()
{
    if (m_dockingArea)
    {
        m_dockingArea->clearInteraction();
    }

    if (m_dockingOverlay)
    {
        m_dockingOverlay->clear();
    }

    if (m_dockingArea)
    {
        m_dockingArea->clearWindows();
    }

    const WindowCollection floatingWindows = m_floatingWindows;
    for (const WindowPointer& window : floatingWindows)
    {
        const bool detached = detachFloatingWindow(window);
        EGO_ASSERT(detached);
    }
}

ego::gui::SurfaceRoot::WindowCollection ego::gui::SurfaceRoot::getWindows() const
{
    WindowCollection windows = m_dockingArea ? m_dockingArea->getWindows() : WindowCollection();
    windows.insert(windows.end(), m_floatingWindows.begin(), m_floatingWindows.end());

    return windows;
}

bool ego::gui::SurfaceRoot::setDockingEnabled(bool _isEnabled)
{
    if (isDockingEnabled() == _isEnabled)
    {
        return true;
    }

    if (m_dockingArea)
    {
        m_dockingArea->clearInteraction();
    }

    if (m_dockingOverlay)
    {
        m_dockingOverlay->clear();
    }

    if (_isEnabled)
    {
        if (!m_dockingArea || !m_dockingArea->initializeRoot())
        {
            return false;
        }

        invalidateLayout();

        return true;
    }

    const WindowCollection dockedWindows = m_dockingArea ? m_dockingArea->releaseWindows() : WindowCollection();
    for (const WindowPointer& window : dockedWindows)
    {
        const bool attached = attachFloatingWindow(window);
        EGO_ASSERT(attached);
    }

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
    const DockingSpacePointer space = isDockingEnabled() ? m_dockingArea->findWindowSpace(_window) : nullptr;

    return space ? space->getID() : InvalidDockingSpaceID;
}

bool ego::gui::SurfaceRoot::dockWindow(const WindowPointer& _window, const WindowPlacement& _placement)
{
    const WindowPointer window = _window;
    if (!window || !isHostedWindow(window))
    {
        return false;
    }

    if (_placement.m_spaceID == InvalidDockingSpaceID)
    {
        return makeWindowFloating(window, window->getFloatingBounds());
    }

    if (!isDockingEnabled())
    {
        return false;
    }

    switch (_placement.m_placement)
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

    if (!std::isfinite(_placement.m_splitRatio))
    {
        return false;
    }

    const DockingSpacePointer targetSpace = m_dockingArea->findSpace(_placement.m_spaceID);
    if (!targetSpace)
    {
        return false;
    }

    const bool wasFloating = window->isDirectChildOf(*this);
    if (wasFloating && !detachFloatingWindow(window))
    {
        return false;
    }

    if (m_dockingArea->dockWindow(window, targetSpace, _placement.m_placement, _placement.m_splitRatio))
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
    if (!window || !isHostedWindow(window) || !isDockingEnabled())
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

    const bool wasFloating = window->isDirectChildOf(*this);
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
    if (!window || !isHostedWindow(window))
    {
        return false;
    }

    if (!window->isDirectChildOf(*this))
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

ego::gui::DockingAreaPointer ego::gui::SurfaceRoot::getDockingArea() const
{
    return m_dockingArea;
}

ego::gui::DockingOverlayPointer ego::gui::SurfaceRoot::getDockingOverlay() const
{
    return m_dockingOverlay;
}

void ego::gui::SurfaceRoot::bringWidgetToFront(const WidgetPointer& _widget)
{
    WidgetPointer widget = _widget;
    while (widget && widget->getParent().get() != this)
    {
        widget = widget->getParent();
    }

    if (widget && rtti::IsObjectBasedOn<Window>(*widget))
    {
        moveFloatingWindowToFront(ego::StaticPointerCast<Window>(widget));
    }
}

ego::gui::WidgetPointer ego::gui::SurfaceRoot::findWidgetAt(const Position& _position)
{
    if (!hitTest(_position))
    {
        return nullptr;
    }

    WidgetPointer currentWidget = sharedFromThis();
    while (currentWidget && currentWidget->isChildHitTestVisible(_position))
    {
        WidgetPointer hitWidget = nullptr;
        const size_t childCount = currentWidget->getChildCount();
        for (size_t childIndex = childCount; childIndex > 0; --childIndex)
        {
            const WidgetPointer child = currentWidget->getChild(childIndex - 1);
            if (child && currentWidget->isChildActive(childIndex - 1) && child->isDirectChildOf(*currentWidget) && child->hitTest(_position))
            {
                hitWidget = child;

                break;
            }
        }

        if (!hitWidget)
        {
            break;
        }

        currentWidget = hitWidget;
    }

    return currentWidget;
}

bool ego::gui::SurfaceRoot::isInputTarget(const WidgetPointer& _widget) const
{
    if (!_widget)
    {
        return false;
    }

    WidgetPointer currentWidget = _widget;
    while (currentWidget)
    {
        const Rect& bounds = currentWidget->getLayoutBounds();
        if (!currentWidget->isVisible() || (currentWidget.get() != this && (bounds.m_size.m_x <= 0.0f || bounds.m_size.m_y <= 0.0f)))
        {
            return false;
        }

        if (currentWidget.get() == this)
        {
            return true;
        }

        const WidgetPointer parent = currentWidget->getParent();
        if (!parent || !containsDirectChild(*parent, *currentWidget))
        {
            return false;
        }

        currentWidget = parent;
    }

    return false;
}

bool ego::gui::SurfaceRoot::updateLayoutIfNeeded(const LayoutContext& _context, const Size& _size)
{
    if (!isLayoutInvalidated())
    {
        return false;
    }

    const Rect surfaceBounds(0.0f, 0.0f, _size.m_x, _size.m_y);
    updatePreferredSize(_context, LayoutConstraints(_size));
    applyLayout(_context, surfaceBounds);
    completeLayout();

    return true;
}

ego::gui::Size ego::gui::SurfaceRoot::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    if (m_dockingArea)
    {
        m_dockingArea->updatePreferredSize(_context, _constraints);
    }

    const WindowCollection floatingWindows = m_floatingWindows;
    for (const WindowPointer& window : floatingWindows)
    {
        if (window && window->getParent().get() == this)
        {
            const Rect windowBounds = window->getFloatingBounds();
            window->updatePreferredSize(_context, LayoutConstraints(windowBounds.m_size));
        }
    }

    if (m_dockingOverlay)
    {
        m_dockingOverlay->updatePreferredSize(_context, _constraints);
    }

    return _constraints.m_maximumSize;
}

void ego::gui::SurfaceRoot::updateGeometry(const LayoutContext& _context)
{
    const Rect& surfaceBounds = getLayoutBounds();
    if (m_dockingArea)
    {
        m_dockingArea->applyLayout(_context, surfaceBounds);
    }

    const WindowCollection floatingWindows = m_floatingWindows;
    for (const WindowPointer& window : floatingWindows)
    {
        if (window && window->getParent().get() == this)
        {
            window->applyLayout(_context, window->getFloatingBounds());
        }
    }

    if (m_dockingOverlay)
    {
        m_dockingOverlay->applyLayout(_context, surfaceBounds);
    }
}

bool ego::gui::SurfaceRoot::isHostedWindow(const WindowPointer& _window) const
{
    if (!_window)
    {
        return false;
    }

    const WindowCollection windows = getWindows();

    return std::find_if(
               windows.begin(),
               windows.end(),
               [&_window](const WindowPointer& _currentWindow)
               {
                   return _currentWindow.get() == _window.get();
               }) != windows.end();
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
    if (windowIt == m_floatingWindows.end() || !detachChild(*windowIt))
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

bool ego::gui::SurfaceRoot::containsDirectChild(const Widget& _parent, const Widget& _child) const
{
    const size_t childCount = _parent.getChildCount();
    for (size_t childIndex = 0; childIndex < childCount; ++childIndex)
    {
        const WidgetPointer child = _parent.getChild(childIndex);
        if (child.get() == &_child && _parent.isChildActive(childIndex))
        {
            return true;
        }
    }

    return false;
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
