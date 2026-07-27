#include "WindowDrag.h"

#include <algorithm>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Docking/DockingPreview.h"
#include "EgoGui/Theme/Theme.h"
#include "EgoGui/Viewport/SurfaceRoot.h"
#include "EgoGui/Viewport/ViewportInputContext.h"
#include "EgoGui/Widgets/Window.h"

#include "InputState.h"

namespace
{
    using InputAccessor = ego::gui::InputState::WindowDragAccessor;
    using RootAccessor = ego::gui::SurfaceRoot::WindowDragAccessor;
} // namespace

void ego::gui::WindowDrag::beginFloating(WindowDragContext& _context, const WindowPointer& _window, const Position& _position, const Position& _screenPosition)
{
    if (!_window || !RootAccessor::IsWindowFloating(_context.m_root, _window))
    {
        return;
    }

    begin(_context.m_root, _window, Origin::FloatingWindow, _position, _screenPosition, _window->getFloatingBounds());
}

void ego::gui::WindowDrag::beginDocked(WindowDragContext& _context, const WindowPointer& _window, const Position& _position, const Position& _screenPosition)
{
    if (!_window || !RootAccessor::IsWindowDocked(_context.m_root, _window))
    {
        return;
    }

    const DockingStyle& style = RootAccessor::GetDockingStyle(_context.m_root);
    const Size requestedSize = _window->getFloatingBounds().m_size;
    const Size floatingSize(
        requestedSize.m_x > 0.0f ? requestedSize.m_x : style.m_minimumSpaceSize.m_x,
        requestedSize.m_y > 0.0f ? requestedSize.m_y : style.m_minimumSpaceSize.m_y);
    const Position grabOffset(floatingSize.m_x * 0.5f, (std::min)(floatingSize.m_y, style.m_tabHeight) * 0.5f);
    const Rect floatingBounds(Position(_position.m_x - grabOffset.m_x, _position.m_y - grabOffset.m_y), floatingSize);
    begin(_context.m_root, _window, Origin::DockingTab, _position, _screenPosition, floatingBounds);
}

bool ego::gui::WindowDrag::update(WindowDragContext& _context, const WindowPointer& _window, const Position& _position, const Position& _screenPosition)
{
    const WindowPointer window = _window;
    if (!isActive(window))
    {
        return false;
    }

    const DockingStyle& style = RootAccessor::GetDockingStyle(_context.m_root);
    const bool isPointerInsideSurface = _context.m_root.getLayoutBounds().contains(_position);
    const Position offset(_position.m_x - m_state.m_startPosition.m_x, _position.m_y - m_state.m_startPosition.m_y);
    m_state.m_floatingBounds.m_position = Position(m_state.m_startBounds.m_position.m_x + offset.m_x, m_state.m_startBounds.m_position.m_y + offset.m_y);

    if (!m_state.m_hasPassedThreshold)
    {
        if (m_state.m_origin == Origin::FloatingWindow && isPointerInsideSurface && RootAccessor::ContainsWindow(_context.m_root, window))
        {
            window->setPosition(constrainFloatingBounds(_context.m_root, m_state.m_floatingBounds).m_position);
        }

        const Position screenOffset(_screenPosition.m_x - m_state.m_startScreenPosition.m_x, _screenPosition.m_y - m_state.m_startScreenPosition.m_y);
        const float distanceSquared = screenOffset.m_x * screenOffset.m_x + screenOffset.m_y * screenOffset.m_y;
        if (distanceSquared < style.m_dragThreshold * style.m_dragThreshold)
        {
            return true;
        }

        m_state.m_hasPassedThreshold = true;
    }

    bool didBeginViewportDrag = false;
    if (!m_state.m_isViewportDragActive)
    {
        m_state.m_isViewportDragActive = RootAccessor::ContainsWindow(_context.m_root, window) &&
                                         _context.m_viewportInput.beginWindowDrag(window, _screenPosition, m_state.m_floatingBounds);
        didBeginViewportDrag = m_state.m_isViewportDragActive;
    }

    if (!m_state.m_isViewportDragActive && !isPointerInsideSurface)
    {
        RootAccessor::ClearPreview(_context.m_root);

        return true;
    }

    if (!m_state.m_isViewportDragActive)
    {
        m_state.m_floatingBounds = constrainFloatingBounds(_context.m_root, m_state.m_floatingBounds);
    }

    if (m_state.m_origin == Origin::DockingTab && !makeDockedWindowFloating(_context, window))
    {
        if (m_state.m_isViewportDragActive)
        {
            _context.m_viewportInput.cancelWindowDrag(window);
        }

        clear(_context.m_root);

        return true;
    }

    if (didBeginViewportDrag)
    {
        if (!takePointerCapture(_context, window))
        {
            _context.m_viewportInput.cancelWindowDrag(window);
            clear(_context.m_root);

            return true;
        }

        window->clearInteraction();
    }

    if (m_state.m_origin == Origin::FloatingWindow && RootAccessor::ContainsWindow(_context.m_root, window))
    {
        window->setPosition(m_state.m_floatingBounds.m_position);
    }

    if (m_state.m_isViewportDragActive)
    {
        _context.m_viewportInput.updateWindowDrag(window, _screenPosition);
    }

    if (!RootAccessor::IsWindowDocked(_context.m_root, window) && !RootAccessor::IsWindowFloating(_context.m_root, window))
    {
        if (!m_state.m_isViewportDragActive)
        {
            RootAccessor::ClearPreview(_context.m_root);
        }

        return true;
    }

    if (!isPointerInsideSurface)
    {
        RootAccessor::ClearPreview(_context.m_root);

        return true;
    }

    RootAccessor::ShowPreview(_context.m_root, RootAccessor::BuildPreview(_context.m_root, window, _position));

    return true;
}

void ego::gui::WindowDrag::finish(WindowDragContext& _context, const WindowPointer& _window, const Position& _position, const Position& _screenPosition)
{
    const WindowPointer window = _window;
    if (!isActive(window))
    {
        return;
    }

    update(_context, window, _position, _screenPosition);
    if (!isActive(window))
    {
        return;
    }

    if (m_state.m_isViewportDragActive && _context.m_viewportInput.finishWindowDrag(window, _screenPosition))
    {
        clearState();

        return;
    }

    if (m_state.m_isViewportDragActive)
    {
        _context.m_viewportInput.cancelWindowDrag(window);
    }

    if (!RootAccessor::ContainsWindow(_context.m_root, window))
    {
        clear(_context.m_root);

        return;
    }

    const DockingPreview preview = RootAccessor::BuildPreview(_context.m_root, window, _position);
    if (m_state.m_hasPassedThreshold && preview.m_isDropAllowed)
    {
        const bool docked = RootAccessor::ApplyPreview(_context.m_root, window, preview);
        EGO_ASSERT(docked);
    }

    clear(_context.m_root);
}

bool ego::gui::WindowDrag::cancel(WindowDragContext& _context, const WindowPointer& _window)
{
    if (!_window)
    {
        return false;
    }

    const bool wasActive = isActive(_window);
    if (m_state.m_isViewportDragActive)
    {
        _context.m_viewportInput.cancelWindowDrag(_window);
    }

    if (wasActive)
    {
        clear(_context.m_root);
    }

    return wasActive;
}

bool ego::gui::WindowDrag::clearLocal(SurfaceRoot& _root, const WindowPointer& _window)
{
    if (!isActive(_window))
    {
        return false;
    }

    clear(_root);

    return true;
}

bool ego::gui::WindowDrag::isActive(const WindowPointer& _window) const
{
    const WindowPointer window = getWindow();

    return _window && m_state.m_origin != Origin::None && window.get() == _window.get();
}

bool ego::gui::WindowDrag::hasPointerCapture() const
{
    return m_state.m_hasPointerCapture;
}

ego::gui::WindowPointer ego::gui::WindowDrag::getWindow() const
{
    return m_state.m_window.lock();
}

bool ego::gui::WindowDrag::takePointerCapture(WindowDragContext& _context, const WidgetPointer& _source)
{
    if (m_state.m_hasPointerCapture)
    {
        return true;
    }

    const WidgetPointer root = _context.m_root.sharedFromThis();
    if (!root || !InputAccessor::TransferPointerCapture(_context.m_inputState, _source, root))
    {
        return false;
    }

    m_state.m_hasPointerCapture = true;

    return true;
}

bool ego::gui::WindowDrag::makeDockedWindowFloating(WindowDragContext& _context, const WindowPointer& _window)
{
    const WidgetPointer captureWidget = _window ? _window->getParent() : nullptr;
    if (!captureWidget || !takePointerCapture(_context, captureWidget))
    {
        return false;
    }

    if (!RootAccessor::MakeWindowFloating(_context.m_root, _window, m_state.m_floatingBounds))
    {
        const WidgetPointer root = _context.m_root.sharedFromThis();
        const bool captureRestored = InputAccessor::TransferPointerCapture(_context.m_inputState, root, captureWidget);
        EGO_ASSERT(captureRestored);
        m_state.m_hasPointerCapture = false;

        return false;
    }

    m_state.m_origin = Origin::FloatingWindow;

    return true;
}

void ego::gui::WindowDrag::begin(
    SurfaceRoot& _root,
    const WindowPointer& _window,
    Origin _origin,
    const Position& _position,
    const Position& _screenPosition,
    const Rect& _floatingBounds)
{
    m_state = State();
    m_state.m_origin = _origin;
    m_state.m_window = _window;
    m_state.m_startPosition = _position;
    m_state.m_startScreenPosition = _screenPosition;
    m_state.m_startBounds = _floatingBounds;
    m_state.m_floatingBounds = _floatingBounds;
    RootAccessor::ClearPreview(_root);
}

ego::gui::Rect ego::gui::WindowDrag::constrainFloatingBounds(const SurfaceRoot& _root, const Rect& _bounds) const
{
    Rect bounds = _bounds;
    const Rect& surfaceBounds = _root.getLayoutBounds();
    const float maximumX = (std::max)(surfaceBounds.getLeft(), surfaceBounds.getRight() - bounds.m_size.m_x);
    const float maximumY = (std::max)(surfaceBounds.getTop(), surfaceBounds.getBottom() - bounds.m_size.m_y);
    bounds.m_position.m_x = (std::clamp)(bounds.m_position.m_x, surfaceBounds.getLeft(), maximumX);
    bounds.m_position.m_y = (std::clamp)(bounds.m_position.m_y, surfaceBounds.getTop(), maximumY);

    return bounds;
}

void ego::gui::WindowDrag::clear(SurfaceRoot& _root)
{
    clearState();
    RootAccessor::ClearPreview(_root);
}

void ego::gui::WindowDrag::clearState()
{
    m_state = State();
}
