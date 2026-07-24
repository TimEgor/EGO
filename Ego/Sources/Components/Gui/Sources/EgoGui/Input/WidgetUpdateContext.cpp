#include "WidgetUpdateContext.h"

#include <algorithm>
#include <cstddef>
#include <utility>

#include "EgoGui/Docking/DockingArea.h"
#include "EgoGui/Docking/DockingOverlay.h"
#include "EgoGui/Viewport/SurfaceRoot.h"

ego::gui::WidgetUpdateContext::WidgetUpdateContext(SurfaceRoot& _root, WidgetUpdateState& _state)
    : m_root(_root),
      m_dockingArea(_root.getDockingArea()),
      m_dockingOverlay(_root.getDockingOverlay()),
      m_state(_state)
{
}

void ego::gui::WidgetUpdateContext::update(const InputEvent& _event)
{
    ++m_state.m_dispatchDepth;
    _event.update(*this);

    --m_state.m_dispatchDepth;

    if (m_state.m_isClearPending)
    {
        clear();
    }
    else if (!m_root.isLayoutInvalidated())
    {
        refresh();
    }
}

void ego::gui::WidgetUpdateContext::updatePointerMove(const PointerMoveEvent& _event)
{
    WidgetUpdateState::PointerState& pointer = m_state.m_pointer;
    pointer.m_position = _event.m_position;
    pointer.m_modifiers = _event.m_modifiers;

    const bool isPointerInsideViewport = m_root.getLayoutBounds().contains(_event.m_position);
    const WidgetPath hoverPath = buildPath(m_root.findWidgetAt(_event.m_position));
    const WidgetPath capturePath = buildPath(pointer.m_captureWidget.lock());
    const WidgetPath& eventPath = isPointerInsideViewport && !capturePath.empty() ? capturePath : hoverPath;

    updateHover(hoverPath);
    if (!isPointerInsideViewport && m_dockingOverlay)
    {
        m_dockingOverlay->clearPreview();
    }

    for (size_t responderIndex = 0; responderIndex < eventPath.size(); ++responderIndex)
    {
        if (!isPathResponderValid(eventPath, responderIndex))
        {
            break;
        }

        const WidgetPointer& responder = eventPath[responderIndex];
        if (applyReply(responder, responder->onPointerMove(*this, _event)))
        {
            break;
        }
    }
}

void ego::gui::WidgetUpdateContext::updatePointerExit(const PointerExitEvent& _event)
{
    WidgetUpdateState::PointerState& pointer = m_state.m_pointer;
    pointer.m_position = _event.m_position;
    pointer.m_modifiers = _event.m_modifiers;

    updateHover(WidgetPath());
    if (m_dockingOverlay)
    {
        m_dockingOverlay->clearPreview();
    }
}

void ego::gui::WidgetUpdateContext::updateMouseButton(const MouseButtonEvent& _event)
{
    if (_event.m_key < MouseInputKey::ButtonLeft || _event.m_key > MouseInputKey::ButtonEight)
    {
        return;
    }

    WidgetUpdateState::PointerState& pointer = m_state.m_pointer;
    pointer.m_position = _event.m_position;
    pointer.m_modifiers = _event.m_modifiers;

    const bool isPointerInsideViewport = m_root.getLayoutBounds().contains(_event.m_position);
    const WidgetPointer captureWidget = pointer.m_captureWidget.lock();
    const WidgetPointer hitWidget = m_root.findWidgetAt(_event.m_position);
    if (_event.m_action == InputButtonAction::Pressed && isPointerInsideViewport && !m_root.isInputTarget(captureWidget))
    {
        m_root.bringWidgetToFront(hitWidget);
    }

    const WidgetPath hoverPath = buildPath(hitWidget);
    const WidgetPath capturePath = buildPath(captureWidget);
    const bool canRouteToCapture = !capturePath.empty() && (isPointerInsideViewport || _event.m_action == InputButtonAction::Released);
    const WidgetPath& eventPath = canRouteToCapture ? capturePath : hoverPath;

    updateHover(hoverPath);

    bool replyPreservesFocus = false;
    bool captureReceivedEvent = false;
    for (size_t responderIndex = 0; responderIndex < eventPath.size(); ++responderIndex)
    {
        if (!isPathResponderValid(eventPath, responderIndex))
        {
            break;
        }

        const WidgetPointer& responder = eventPath[responderIndex];
        captureReceivedEvent = captureReceivedEvent || (canRouteToCapture && responder.get() == captureWidget.get());
        const InputReply reply = responder->onMouseButton(*this, _event);
        if (!applyReply(responder, reply))
        {
            continue;
        }

        const bool focusAcquired = (reply == InputReply::Focus || reply == InputReply::FocusAndCapture) && getFocusedWidget().get() == responder.get();
        bool captureAcquired = false;
        if (_event.m_action == InputButtonAction::Pressed && isPointerInsideViewport && (reply == InputReply::Capture || reply == InputReply::FocusAndCapture))
        {
            captureAcquired = capturePointer(responder, _event.m_key);
            if (!captureAcquired)
            {
                if (captureWidget.get() == responder.get())
                {
                    const WidgetPointer currentCaptureWidget = pointer.m_captureWidget.lock();
                    if (currentCaptureWidget.get() == responder.get())
                    {
                        clearPointerCapture();
                    }
                }
                else
                {
                    responder->onPointerCaptureLost(*this, _event.m_position);
                }
            }
        }
        replyPreservesFocus = focusAcquired || captureAcquired;

        break;
    }

    if (_event.m_action == InputButtonAction::Pressed)
    {
        const WidgetPointer focusedWidget = getFocusedWidget();
        if (isPointerInsideViewport && !replyPreservesFocus && focusedWidget && !ContainsWidget(eventPath, focusedWidget))
        {
            setFocusedWidget(nullptr);
        }
    }
    else
    {
        if (captureWidget && !captureReceivedEvent)
        {
            clearPointerCapture();
        }
        else
        {
            releasePointerButton(_event.m_key);
        }
    }
}

void ego::gui::WidgetUpdateContext::updateMouseWheel(const MouseWheelEvent& _event)
{
    WidgetUpdateState::PointerState& pointer = m_state.m_pointer;
    pointer.m_position = _event.m_position;
    pointer.m_modifiers = _event.m_modifiers;

    const WidgetPath hoverPath = buildPath(m_root.findWidgetAt(_event.m_position));
    updateHover(hoverPath);

    for (size_t responderIndex = 0; responderIndex < hoverPath.size(); ++responderIndex)
    {
        if (!isPathResponderValid(hoverPath, responderIndex))
        {
            break;
        }

        const WidgetPointer& responder = hoverPath[responderIndex];
        if (applyReply(responder, responder->onMouseWheel(*this, _event)))
        {
            break;
        }
    }
}

void ego::gui::WidgetUpdateContext::updateKey(const KeyEvent& _event)
{
    const WidgetPath path = buildPath(getFocusedWidget());
    for (size_t responderIndex = 0; responderIndex < path.size(); ++responderIndex)
    {
        if (!isPathResponderValid(path, responderIndex))
        {
            break;
        }

        const WidgetPointer& responder = path[responderIndex];
        if (applyReply(responder, responder->onKey(*this, _event)))
        {
            break;
        }
    }
}

void ego::gui::WidgetUpdateContext::updateTextInput(const TextInputEvent& _event)
{
    const WidgetPath path = buildPath(getFocusedWidget());
    for (size_t responderIndex = 0; responderIndex < path.size(); ++responderIndex)
    {
        if (!isPathResponderValid(path, responderIndex))
        {
            break;
        }

        const WidgetPointer& responder = path[responderIndex];
        if (applyReply(responder, responder->onTextInput(*this, _event)))
        {
            break;
        }
    }
}

void ego::gui::WidgetUpdateContext::updateViewportDeactivated(const ViewportDeactivatedEvent&)
{
    clear();
}

void ego::gui::WidgetUpdateContext::refresh()
{
    if (m_state.m_dispatchDepth > 0)
    {
        return;
    }

    ++m_state.m_dispatchDepth;
    applyPointerCaptureCancellation();
    refreshState();

    --m_state.m_dispatchDepth;

    if (m_state.m_isClearPending)
    {
        clear();

        return;
    }

    ++m_state.m_dispatchDepth;

    applyPointerCaptureCancellation();

    --m_state.m_dispatchDepth;

    if (m_state.m_isClearPending)
    {
        clear();

        return;
    }
}

void ego::gui::WidgetUpdateContext::refreshState()
{
    const WidgetPointer focusedWidget = m_state.m_focusedWidget.lock();
    if (focusedWidget && !m_root.isInputTarget(focusedWidget))
    {
        m_state.m_focusedWidget.reset();
        focusedWidget->onFocusChanged(*this, FocusChange::Lost);
    }

    const WidgetPointer captureWidget = m_state.m_pointer.m_captureWidget.lock();
    if (!captureWidget)
    {
        m_state.m_pointer.m_captureButtons = 0;
    }
    else if (!m_root.isInputTarget(captureWidget))
    {
        m_state.m_pointer.m_captureWidget.reset();
        m_state.m_pointer.m_captureButtons = 0;
        captureWidget->onPointerCaptureLost(*this, m_state.m_pointer.m_position);
    }

    if (!m_state.m_pointer.m_hoverPath.empty() || m_state.m_pointer.m_captureWidget.lock())
    {
        updateHover(buildPath(m_root.findWidgetAt(m_state.m_pointer.m_position)));
    }
}

void ego::gui::WidgetUpdateContext::clear()
{
    if (m_state.m_dispatchDepth > 0)
    {
        m_state.m_isClearPending = true;

        return;
    }

    m_state.m_isClearPending = false;
    ++m_state.m_dispatchDepth;

    clearState();

    --m_state.m_dispatchDepth;
    m_state.m_isClearPending = false;
}

void ego::gui::WidgetUpdateContext::clearState()
{
    updateHover(WidgetPath());
    setFocusedWidget(nullptr);
    clearPointerCapture();

    if (m_dockingArea)
    {
        m_dockingArea->clearInteraction();
    }

    if (m_dockingOverlay)
    {
        m_dockingOverlay->clear();
    }

    m_state.m_pointer = WidgetUpdateState::PointerState();
    m_state.m_pointerCaptureCancellationTarget.reset();
}

void ego::gui::WidgetUpdateContext::detachInputState(const WidgetPointer& _subtree)
{
    const auto isAffected = [this, &_subtree](const WidgetPointer& _widget)
    {
        return _widget && (!_subtree || _widget.get() == _subtree.get() || _widget->isDescendantOf(*_subtree) || !m_root.isInputTarget(_widget));
    };

    WidgetPath hoverPath;
    WidgetUpdateState::WeakWidgetPath remainingHoverPath;
    for (const WidgetWeakPointer& hoveredWidget : m_state.m_pointer.m_hoverPath)
    {
        const WidgetPointer widget = hoveredWidget.lock();
        if (isAffected(widget))
        {
            hoverPath.push_back(widget);
        }
        else if (widget)
        {
            remainingHoverPath.push_back(widget);
        }
    }

    const WidgetPointer focusedWidget = m_state.m_focusedWidget.lock();
    const WidgetPointer captureWidget = m_state.m_pointer.m_captureWidget.lock();
    const WidgetPointer captureCancellationTarget = m_state.m_pointerCaptureCancellationTarget.lock();
    const bool clearFocus = isAffected(focusedWidget);
    const bool clearCapture = isAffected(captureWidget);

    m_state.m_pointer.m_hoverPath = std::move(remainingHoverPath);

    if (clearFocus)
    {
        m_state.m_focusedWidget.reset();
    }

    if (!_subtree || clearCapture || !captureWidget)
    {
        m_state.m_pointer.m_captureWidget.reset();
        m_state.m_pointer.m_captureButtons = 0;
    }

    if (!_subtree || isAffected(captureCancellationTarget))
    {
        m_state.m_pointerCaptureCancellationTarget.reset();
    }

    ++m_state.m_dispatchDepth;
    for (const WidgetPointer& widget : hoverPath)
    {
        widget->onPointerLeave(*this, m_state.m_pointer.m_position, m_state.m_pointer.m_modifiers);
    }

    if (clearFocus)
    {
        focusedWidget->onFocusChanged(*this, FocusChange::Lost);
    }

    if (clearCapture)
    {
        captureWidget->onPointerCaptureLost(*this, m_state.m_pointer.m_position);
    }

    --m_state.m_dispatchDepth;
    if (m_state.m_dispatchDepth == 0 && m_state.m_isClearPending)
    {
        clear();
    }
}

void ego::gui::WidgetUpdateContext::requestPointerCaptureCancellation(const WidgetPointer& _relatedWidget)
{
    if (m_state.m_pointerCaptureCancellationTarget.lock())
    {
        return;
    }

    const WidgetPointer captureWidget = m_state.m_pointer.m_captureWidget.lock();
    if (!captureWidget)
    {
        m_state.m_pointer.m_captureButtons = 0;

        return;
    }

    if (_relatedWidget && captureWidget.get() != _relatedWidget.get() && !captureWidget->isDescendantOf(*_relatedWidget) &&
        !_relatedWidget->isDescendantOf(*captureWidget) && m_root.isInputTarget(captureWidget))
    {
        return;
    }

    m_state.m_pointerCaptureCancellationTarget = captureWidget;
}

void ego::gui::WidgetUpdateContext::flushPointerCaptureCancellation()
{
    if (m_state.m_dispatchDepth == 0)
    {
        ++m_state.m_dispatchDepth;
        applyPointerCaptureCancellation();
        --m_state.m_dispatchDepth;
    }
}

bool ego::gui::WidgetUpdateContext::transferPointerCapture(const WidgetPointer& _source, const WidgetPointer& _target)
{
    WidgetUpdateState::PointerState& pointer = m_state.m_pointer;
    const WidgetPointer captureWidget = pointer.m_captureWidget.lock();
    if (!_source || captureWidget.get() != _source.get() || pointer.m_captureButtons == 0 || !m_root.isInputTarget(_target))
    {
        return false;
    }

    pointer.m_captureWidget = _target;

    return true;
}

ego::gui::WidgetPointer ego::gui::WidgetUpdateContext::getFocusedWidget() const
{
    const WidgetPointer focusedWidget = m_state.m_focusedWidget.lock();

    return m_root.isInputTarget(focusedWidget) ? focusedWidget : nullptr;
}

ego::gui::SurfaceRoot& ego::gui::WidgetUpdateContext::getRoot() const
{
    return m_root;
}

ego::gui::DockingAreaPointer ego::gui::WidgetUpdateContext::getDockingArea() const
{
    return m_dockingArea;
}

ego::gui::DockingOverlayPointer ego::gui::WidgetUpdateContext::getDockingOverlay() const
{
    return m_dockingOverlay;
}

ego::gui::WidgetUpdateContext::MouseButtonMask ego::gui::WidgetUpdateContext::GetMouseButtonMask(MouseInputKey _key)
{
    const uint32_t buttonIndex = static_cast<uint32_t>(_key) - static_cast<uint32_t>(MouseInputKey::ButtonLeft);

    return static_cast<MouseButtonMask>(1u << buttonIndex);
}

bool ego::gui::WidgetUpdateContext::ContainsWidget(const WidgetPath& _path, const WidgetPointer& _widget)
{
    if (!_widget)
    {
        return false;
    }

    return std::find_if(
               _path.begin(),
               _path.end(),
               [&_widget](const WidgetPointer& _pathWidget)
               {
                   return _pathWidget.get() == _widget.get();
               }) != _path.end();
}

ego::gui::WidgetUpdateContext::WidgetPath ego::gui::WidgetUpdateContext::buildPath(const WidgetPointer& _target) const
{
    WidgetPath path;
    if (!m_root.isInputTarget(_target))
    {
        return path;
    }

    WidgetPointer widget = _target;
    while (widget)
    {
        path.push_back(widget);
        if (widget.get() == &m_root)
        {
            return path;
        }

        widget = widget->getParent();
    }

    path.clear();

    return path;
}

bool ego::gui::WidgetUpdateContext::isPathResponderValid(const WidgetPath& _path, size_t _responderIndex) const
{
    if (_responderIndex >= _path.size() || !m_root.isInputTarget(_path[_responderIndex]))
    {
        return false;
    }

    return _responderIndex == 0 || (m_root.isInputTarget(_path[_responderIndex - 1]) && _path[_responderIndex - 1]->isDirectChildOf(*_path[_responderIndex]));
}

bool ego::gui::WidgetUpdateContext::applyReply(const WidgetPointer& _responder, InputReply _reply)
{
    if (_reply == InputReply::Unhandled)
    {
        return false;
    }

    if ((_reply == InputReply::Focus || _reply == InputReply::FocusAndCapture) && m_root.isInputTarget(_responder))
    {
        setFocusedWidget(_responder);
    }
    else if (_reply == InputReply::ClearFocus)
    {
        setFocusedWidget(nullptr);
    }

    return true;
}

void ego::gui::WidgetUpdateContext::setFocusedWidget(const WidgetPointer& _widget)
{
    const WidgetPointer targetWidget = m_root.isInputTarget(_widget) ? _widget : nullptr;
    const WidgetPointer focusedWidget = m_state.m_focusedWidget.lock();
    if (focusedWidget.get() == targetWidget.get())
    {
        return;
    }

    m_state.m_focusedWidget.reset();
    if (focusedWidget)
    {
        focusedWidget->onFocusChanged(*this, FocusChange::Lost);
    }

    if (m_root.isInputTarget(targetWidget))
    {
        m_state.m_focusedWidget = targetWidget;
        targetWidget->onFocusChanged(*this, FocusChange::Gained);
    }
}

bool ego::gui::WidgetUpdateContext::capturePointer(const WidgetPointer& _widget, MouseInputKey _key)
{
    if (!m_root.isInputTarget(_widget))
    {
        return false;
    }

    const WidgetPointer captureWidget = m_state.m_pointer.m_captureWidget.lock();
    if (captureWidget && captureWidget.get() != _widget.get())
    {
        return false;
    }

    m_state.m_pointer.m_captureWidget = _widget;
    m_state.m_pointer.m_captureButtons |= GetMouseButtonMask(_key);

    return true;
}

void ego::gui::WidgetUpdateContext::releasePointerButton(MouseInputKey _key)
{
    m_state.m_pointer.m_captureButtons &= static_cast<MouseButtonMask>(~GetMouseButtonMask(_key));
    if (m_state.m_pointer.m_captureButtons == 0)
    {
        m_state.m_pointer.m_captureWidget.reset();
    }
}

void ego::gui::WidgetUpdateContext::clearPointerCapture()
{
    const WidgetPointer captureWidget = m_state.m_pointer.m_captureWidget.lock();
    m_state.m_pointer.m_captureWidget.reset();
    m_state.m_pointer.m_captureButtons = 0;

    if (captureWidget)
    {
        captureWidget->onPointerCaptureLost(*this, m_state.m_pointer.m_position);
    }
}

void ego::gui::WidgetUpdateContext::applyPointerCaptureCancellation()
{
    const WidgetPointer captureCancellationTarget = m_state.m_pointerCaptureCancellationTarget.lock();
    m_state.m_pointerCaptureCancellationTarget.reset();
    if (!captureCancellationTarget)
    {
        return;
    }

    const WidgetPointer captureWidget = m_state.m_pointer.m_captureWidget.lock();
    if (captureWidget.get() == captureCancellationTarget.get())
    {
        clearPointerCapture();
    }
}

void ego::gui::WidgetUpdateContext::updateHover(const WidgetPath& _path)
{
    WidgetPath previousPath;
    previousPath.reserve(m_state.m_pointer.m_hoverPath.size());
    for (const WidgetWeakPointer& hoveredWidget : m_state.m_pointer.m_hoverPath)
    {
        previousPath.push_back(hoveredWidget.lock());
    }

    size_t leaveCount = previousPath.size();
    size_t enterCount = _path.size();

    while (leaveCount > 0 && enterCount > 0)
    {
        const WidgetPointer& previousWidget = previousPath[leaveCount - 1];
        const WidgetPointer& currentWidget = _path[enterCount - 1];
        if (!previousWidget || previousWidget.get() != currentWidget.get())
        {
            break;
        }

        --leaveCount;
        --enterCount;
    }

    m_state.m_pointer.m_hoverPath.assign(_path.begin() + enterCount, _path.end());

    for (size_t widgetIndex = 0; widgetIndex < leaveCount; ++widgetIndex)
    {
        const WidgetPointer& widget = previousPath[widgetIndex];
        if (widget)
        {
            widget->onPointerLeave(*this, m_state.m_pointer.m_position, m_state.m_pointer.m_modifiers);
        }
    }

    for (size_t widgetIndex = enterCount; widgetIndex > 0; --widgetIndex)
    {
        const WidgetPointer& widget = _path[widgetIndex - 1];
        const bool hasExpectedParent = widgetIndex == _path.size() || widget->isDirectChildOf(*_path[widgetIndex]);
        if (!hasExpectedParent || !m_root.isInputTarget(widget))
        {
            break;
        }

        m_state.m_pointer.m_hoverPath.insert(m_state.m_pointer.m_hoverPath.begin(), widget);
        widget->onPointerEnter(*this, m_state.m_pointer.m_position, m_state.m_pointer.m_modifiers);
    }
}
