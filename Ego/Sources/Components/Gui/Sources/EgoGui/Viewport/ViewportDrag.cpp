#include "ViewportDrag.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGui/Widgets/Window.h"

#include "Viewport.h"
#include "ViewportProvider.h"

namespace
{
    using ViewportAccessor = ego::gui::Viewport::ViewportManagerAccessor;
} // namespace

bool ego::gui::ViewportManager::ViewportDrag::begin(
    ViewportManager& _manager,
    ViewportID _inputViewportID,
    const WindowPointer& _window,
    const Position& _screenPosition,
    const Rect& _localWindowBounds)
{
    if (!_manager.m_isMultiViewportEnabled || !_window)
    {
        return false;
    }

    const ViewportPointer sourceViewport = _manager.findViewport(_inputViewportID);
    EGO_CHECK_RETURN_FALSE(
        sourceViewport && ViewportAccessor::ContainsWindow(*sourceViewport, _window) && !ViewportAccessor::IsWindowBound(*sourceViewport, _window));

    const ViewportID sourceViewportID = ViewportAccessor::GetID(*sourceViewport);
    if (isInput(sourceViewportID, _window))
    {
        return true;
    }

    reset(_manager);

    ViewportDrag dragState;
    dragState.m_inputViewportID = sourceViewportID;
    dragState.m_windowViewportID = sourceViewportID;
    dragState.m_window = _window;

    const Position& sourcePosition = ViewportAccessor::GetPosition(*sourceViewport);
    dragState.m_screenWindowBounds = Rect(
        sourcePosition.m_x + _localWindowBounds.m_position.m_x,
        sourcePosition.m_y + _localWindowBounds.m_position.m_y,
        _localWindowBounds.m_size.m_x,
        _localWindowBounds.m_size.m_y);
    dragState.m_grabOffset =
        Position(_screenPosition.m_x - dragState.m_screenWindowBounds.m_position.m_x, _screenPosition.m_y - dragState.m_screenWindowBounds.m_position.m_y);
    dragState.m_screenPosition = _screenPosition;

    if (_manager.isManagedViewport(sourceViewport))
    {
        if (ViewportAccessor::GetWindowCount(*sourceViewport) != 1 || !_manager.m_provider->setViewportInputPassthrough(sourceViewportID, true))
        {
            return false;
        }

        dragState.m_isMovingViewport = true;
    }
    else
    {
        dragState.m_isWindowViewportCreationPending = true;
    }

    *this = dragState;
    update(_manager, _inputViewportID, _window, _screenPosition);

    return true;
}

void ego::gui::ViewportManager::ViewportDrag::update(
    ViewportManager& _manager,
    ViewportID _inputViewportID,
    const WindowPointer& _window,
    const Position& _screenPosition)
{
    const ViewportPointer inputViewport = _manager.findViewport(_inputViewportID);
    const ViewportPointer windowViewport = _manager.findViewport(m_windowViewportID);
    if (!inputViewport || !windowViewport || !isInput(ViewportAccessor::GetID(*inputViewport), _window) ||
        !ViewportAccessor::ContainsWindow(*windowViewport, _window) || m_isCompletionPending)
    {
        return;
    }

    m_screenPosition = _screenPosition;
    m_screenWindowBounds.m_position = Position(_screenPosition.m_x - m_grabOffset.m_x, _screenPosition.m_y - m_grabOffset.m_y);

    if (m_isMovingViewport)
    {
        Position position = m_screenWindowBounds.m_position;
        if (_manager.m_provider->setViewportPosition(ViewportAccessor::GetID(*windowViewport), position))
        {
            m_screenWindowBounds.m_position = position;

            ViewportAccessor::SetPosition(*windowViewport, position);
            _window->setBounds(Rect(PositionZero, m_screenWindowBounds.m_size));
        }
    }

    updateTarget(_manager);
}

bool ego::gui::ViewportManager::ViewportDrag::finish(
    ViewportManager& _manager,
    ViewportID _inputViewportID,
    const WindowPointer& _window,
    const Position& _screenPosition)
{
    const ViewportPointer inputViewport = _manager.findViewport(_inputViewportID);
    const ViewportPointer windowViewport = _manager.findViewport(m_windowViewportID);
    if (!inputViewport || !windowViewport || !isInput(ViewportAccessor::GetID(*inputViewport), _window))
    {
        return false;
    }

    update(_manager, _inputViewportID, _window, _screenPosition);

    if (!_manager.m_isMultiViewportEnabled || ViewportAccessor::IsWindowBound(*windowViewport, _window))
    {
        reset(_manager, false);
        return false;
    }

    const Rect sourceBounds(ViewportAccessor::GetPosition(*windowViewport), ViewportAccessor::GetSize(*windowViewport));
    const bool leavesSourceViewport = !sourceBounds.contains(_screenPosition);
    const bool completesOutsideSource = m_isMovingViewport || m_targetViewportID != InvalidViewportID || leavesSourceViewport;
    if (!completesOutsideSource)
    {
        reset(_manager, false);
        return false;
    }

    m_isCompletionPending = true;

    return true;
}

void ego::gui::ViewportManager::ViewportDrag::cancel(ViewportManager& _manager, ViewportID _inputViewportID, const WindowPointer& _window)
{
    const ViewportPointer sourceViewport = _manager.findViewport(_inputViewportID);
    if (!sourceViewport || !_window || m_window.getObject() != _window.getObject() || m_isCommitting)
    {
        return;
    }

    const ViewportID sourceViewportID = ViewportAccessor::GetID(*sourceViewport);
    if (sourceViewportID != m_inputViewportID && sourceViewportID != m_windowViewportID)
    {
        return;
    }

    reset(_manager, sourceViewportID != m_inputViewportID);
}

bool ego::gui::ViewportManager::ViewportDrag::allowsInteractionOutsideSurface(const ViewportManager& _manager, ViewportID _inputViewportID) const
{
    return _manager.m_isMultiViewportEnabled && _manager.isManagedViewport(_manager.findViewport(_inputViewportID));
}

void ego::gui::ViewportManager::ViewportDrag::completeInput(ViewportManager& _manager, ViewportID _inputViewportID)
{
    const ViewportPointer sourceViewport = _manager.findViewport(_inputViewportID);
    if (!sourceViewport)
    {
        return;
    }

    const ViewportID sourceViewportID = ViewportAccessor::GetID(*sourceViewport);
    _manager.syncManagedViewport(sourceViewport);

    if (m_inputViewportID != sourceViewportID)
    {
        return;
    }

    if (m_isWindowViewportCreationPending)
    {
        m_isWindowViewportCreationPending = false;
        createWindowViewport(_manager);
    }

    if (!m_isCompletionPending)
    {
        return;
    }

    m_isCommitting = true;
    const bool commitResult = commit(_manager);
    EGO_ASSERT_MESSAGE(commitResult, "Failed to complete a cross-viewport window drag.");

    m_isCommitting = false;
    reset(_manager);
}

void ego::gui::ViewportManager::ViewportDrag::handleViewportDestroying(ViewportManager& _manager, const ViewportPointer& _viewport)
{
    if (!_viewport)
    {
        return;
    }

    const ViewportID viewportID = ViewportAccessor::GetID(*_viewport);
    const bool destroysDragViewport = m_inputViewportID == viewportID || m_windowViewportID == viewportID;
    if (!m_isCommitting && destroysDragViewport)
    {
        reset(_manager);
    }
    else if (m_targetViewportID == viewportID)
    {
        ViewportAccessor::ClearExternalDragPreview(*_viewport);
        m_targetViewportID = InvalidViewportID;
    }
}

bool ego::gui::ViewportManager::ViewportDrag::isValid(const ViewportManager& _manager) const
{
    if (!m_window)
    {
        return true;
    }

    const ViewportPointer inputViewport = _manager.findViewport(m_inputViewportID);
    const ViewportPointer windowViewport = _manager.findViewport(m_windowViewportID);
    if (!inputViewport || !windowViewport || !ViewportAccessor::ContainsWindow(*windowViewport, m_window) ||
        ViewportAccessor::IsWindowBound(*windowViewport, m_window))
    {
        return false;
    }

    return m_isCompletionPending || ViewportAccessor::IsWindowDragActive(*inputViewport, m_window);
}

bool ego::gui::ViewportManager::ViewportDrag::isInput(ViewportID _inputViewportID, const WindowPointer& _window) const
{
    return _window && m_inputViewportID == _inputViewportID && m_window.getObject() == _window.getObject();
}

void ego::gui::ViewportManager::ViewportDrag::reset(ViewportManager& _manager, bool _clearInputInteraction)
{
    const ViewportID inputViewportID = m_inputViewportID;
    const ViewportID windowViewportID = m_windowViewportID;
    const ViewportID targetViewportID = m_targetViewportID;
    const WindowPointer window = m_window;
    *this = ViewportDrag();

    const ViewportPointer targetViewport = _manager.findViewport(targetViewportID);
    if (targetViewport)
    {
        ViewportAccessor::ClearExternalDragPreview(*targetViewport);
    }

    const ViewportPointer inputViewport = _manager.findViewport(inputViewportID);
    if (_clearInputInteraction && inputViewport)
    {
        ViewportAccessor::CancelWindowInteraction(*inputViewport, window);
        ViewportAccessor::ClearExternalDragPreview(*inputViewport);
    }

    if (_manager.m_provider && _manager.findViewport(windowViewportID))
    {
        _manager.m_provider->setViewportInputPassthrough(windowViewportID, false);
    }
}

void ego::gui::ViewportManager::ViewportDrag::updateTarget(ViewportManager& _manager)
{
    const ViewportID targetViewportIDAtPosition = _manager.m_provider->findViewportAtScreenPosition(m_screenPosition);
    ViewportID targetViewportID = _manager.m_isMultiViewportEnabled ? targetViewportIDAtPosition : InvalidViewportID;

    const ViewportPointer windowViewport = _manager.findViewport(m_windowViewportID);
    if (windowViewport && ViewportAccessor::IsWindowBound(*windowViewport, m_window))
    {
        targetViewportID = InvalidViewportID;
    }

    if (targetViewportID == m_windowViewportID)
    {
        targetViewportID = InvalidViewportID;
    }

    if (m_targetViewportID != targetViewportID)
    {
        clearTarget(_manager);
        m_targetViewportID = targetViewportID;
    }

    const ViewportPointer targetViewport = _manager.findViewport(m_targetViewportID);
    if (!targetViewport)
    {
        m_targetViewportID = InvalidViewportID;
        return;
    }

    const Position& targetViewportPosition = ViewportAccessor::GetPosition(*targetViewport);
    const Position targetPosition(m_screenPosition.m_x - targetViewportPosition.m_x, m_screenPosition.m_y - targetViewportPosition.m_y);
    ViewportAccessor::UpdateExternalDragPreview(*targetViewport, m_window, targetPosition);
}

void ego::gui::ViewportManager::ViewportDrag::clearTarget(ViewportManager& _manager)
{
    const ViewportPointer targetViewport = _manager.findViewport(m_targetViewportID);
    if (targetViewport)
    {
        ViewportAccessor::ClearExternalDragPreview(*targetViewport);
    }

    m_targetViewportID = InvalidViewportID;
}

bool ego::gui::ViewportManager::ViewportDrag::commit(ViewportManager& _manager)
{
    const ViewportPointer sourceViewport = _manager.findViewport(m_windowViewportID);
    EGO_CHECK_RETURN_FALSE(sourceViewport && m_window);

    _manager.m_provider->setViewportInputPassthrough(m_windowViewportID, false);

    const ViewportPointer targetViewport = _manager.findViewport(m_targetViewportID);
    if (targetViewport && transferWindow(_manager, targetViewport))
    {
        return true;
    }

    if (m_isMovingViewport)
    {
        return true;
    }

    return createDetachedViewport(_manager);
}

bool ego::gui::ViewportManager::ViewportDrag::transferWindow(ViewportManager& _manager, const ViewportPointer& _targetViewport)
{
    const ViewportPointer sourceViewport = _manager.findViewport(m_windowViewportID);
    const WindowPointer window = m_window;
    EGO_CHECK_RETURN_FALSE(sourceViewport && _targetViewport && window);
    EGO_CHECK_RETURN_FALSE(sourceViewport.getObject() != _targetViewport.getObject());

    const Rect sourceBounds = window->getFloatingBounds();
    EGO_CHECK_RETURN_FALSE(sourceViewport->removeWindow(window));

    const Position& targetViewportPosition = ViewportAccessor::GetPosition(*_targetViewport);
    const Position targetWindowPosition(
        m_screenWindowBounds.m_position.m_x - targetViewportPosition.m_x,
        m_screenWindowBounds.m_position.m_y - targetViewportPosition.m_y);
    window->setBounds(Rect(targetWindowPosition, m_screenWindowBounds.m_size));
    if (!_targetViewport->addWindow(window))
    {
        window->setBounds(sourceBounds);
        const bool restored = sourceViewport->addWindow(window);
        EGO_ASSERT(restored);

        return false;
    }

    const Position targetPointerPosition(m_screenPosition.m_x - targetViewportPosition.m_x, m_screenPosition.m_y - targetViewportPosition.m_y);
    if (!ViewportAccessor::DockExternalWindow(*_targetViewport, window, targetPointerPosition))
    {
        const bool targetWindowRemoved = _targetViewport->removeWindow(window);
        EGO_ASSERT(targetWindowRemoved);

        window->setBounds(sourceBounds);
        const bool restored = sourceViewport->addWindow(window);
        EGO_ASSERT(restored);

        return false;
    }

    if (_manager.isManagedViewport(sourceViewport) && ViewportAccessor::GetWindowCount(*sourceViewport) == 0)
    {
        const bool destroyResult = _manager.destroyViewport(sourceViewport, false);
        EGO_ASSERT(destroyResult);
    }

    return true;
}

bool ego::gui::ViewportManager::ViewportDrag::createWindowViewport(ViewportManager& _manager)
{
    const WindowPointer window = m_window;
    const ViewportPointer sourceViewport = _manager.findViewport(m_windowViewportID);
    EGO_CHECK_RETURN_FALSE(window && sourceViewport && ViewportAccessor::ContainsWindow(*sourceViewport, window));
    EGO_CHECK_RETURN_FALSE(_manager.m_isMultiViewportEnabled);
    EGO_CHECK_RETURN_FALSE(!ViewportAccessor::IsWindowBound(*sourceViewport, window) && ViewportAccessor::IsWindowFloating(*sourceViewport, window));
    EGO_CHECK_RETURN_FALSE(!_manager.isManagedViewport(sourceViewport));

    Size viewportSize = m_screenWindowBounds.m_size;
    if (viewportSize.m_x <= 0.0f || viewportSize.m_y <= 0.0f)
    {
        viewportSize = Size(ViewportManager::DefaultManagedViewportExtent, ViewportManager::DefaultManagedViewportExtent);
    }

    ViewportDesc viewportDesc;
    viewportDesc.m_title = window->getTitle();
    viewportDesc.m_position = m_screenWindowBounds.m_position;
    viewportDesc.m_size = viewportSize;

    const ViewportPointer viewport = _manager.createViewport(ViewportRole::Secondary, viewportDesc, true);
    EGO_CHECK_RETURN_FALSE(viewport);

    const Rect sourceBounds = window->getFloatingBounds();
    if (!ViewportAccessor::TransferWindowTo(*sourceViewport, *viewport, window))
    {
        const bool destroyResult = _manager.destroyViewport(viewport, false);
        EGO_ASSERT(destroyResult);

        return false;
    }

    const Size createdViewportSize = ViewportAccessor::GetSize(*viewport);
    const bool isViewportValid = createdViewportSize.m_x > 0.0f && createdViewportSize.m_y > 0.0f;
    if (isViewportValid)
    {
        window->setBounds(Rect(PositionZero, createdViewportSize));
    }

    const ViewportID viewportID = ViewportAccessor::GetID(*viewport);
    const bool passthroughEnabled = isViewportValid && _manager.m_provider->setViewportInputPassthrough(viewportID, true);
    const bool viewportShown = passthroughEnabled && _manager.m_provider->showViewport(viewportID, false);
    if (!viewportShown)
    {
        if (passthroughEnabled)
        {
            _manager.m_provider->setViewportInputPassthrough(viewportID, false);
        }

        const bool restored = ViewportAccessor::TransferWindowTo(*viewport, *sourceViewport, window);
        EGO_ASSERT(restored);
        window->setBounds(sourceBounds);

        const bool destroyResult = _manager.destroyViewport(viewport, false);
        EGO_ASSERT(destroyResult);

        return false;
    }

    const Position& viewportPosition = ViewportAccessor::GetPosition(*viewport);
    m_windowViewportID = viewportID;
    m_screenWindowBounds = Rect(viewportPosition, createdViewportSize);
    m_grabOffset = Position(m_screenPosition.m_x - viewportPosition.m_x, m_screenPosition.m_y - viewportPosition.m_y);
    m_isMovingViewport = true;
    updateTarget(_manager);

    return true;
}

bool ego::gui::ViewportManager::ViewportDrag::createDetachedViewport(ViewportManager& _manager)
{
    const WindowPointer window = m_window;
    const ViewportPointer sourceViewport = _manager.findViewport(m_windowViewportID);
    EGO_CHECK_RETURN_FALSE(window && sourceViewport);
    EGO_CHECK_RETURN_FALSE(_manager.m_isMultiViewportEnabled);
    EGO_CHECK_RETURN_FALSE(!ViewportAccessor::IsWindowBound(*sourceViewport, window));

    return _manager.createWindowViewport(window, sourceViewport, m_screenWindowBounds);
}
