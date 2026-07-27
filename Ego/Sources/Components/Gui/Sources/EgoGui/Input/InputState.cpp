#include "InputState.h"

#include <algorithm>
#include <cstddef>
#include <utility>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Viewport/SurfaceRoot.h"
#include "EgoGui/Viewport/ViewportInputContext.h"
#include "EgoGui/Widgets/Window.h"

#include "InputContext.h"
#include "WindowDrag.h"

namespace
{
    using ContextAccessor = ego::gui::InputContext::InputAccessor;
    using RootAccessor = ego::gui::SurfaceRoot::InputAccessor;
    using StateAccessor = ego::gui::InputState::InputPassAccessor;
    using WidgetAccessor = ego::gui::Widget::WidgetAccessor;
} // namespace

class ego::gui::InputPass final : public InputEventSink
{
public:
    static void Run(InputState& _state, SurfaceRoot& _root, InputContext& _context, const InputEvent& _event);

private:
    using WidgetPath = std::vector<WidgetPointer>;

    InputPass(InputState& _state, SurfaceRoot& _root, InputContext& _context);

    static bool ContainsWidget(const WidgetPath& _path, const WidgetPointer& _widget);

    void dispatch(const InputEvent& _event);
    void process(const PointerMoveEvent& _event) override;
    void process(const PointerExitEvent& _event) override;
    void process(const MouseButtonEvent& _event) override;
    void process(const MouseWheelEvent& _event) override;
    void process(const KeyEvent& _event) override;
    void process(const TextInputEvent& _event) override;
    void process(const PointerCaptureLostEvent& _event) override;
    void process(const ViewportDeactivatedEvent& _event) override;

    void updatePointerMove(const PointerMoveEvent& _event);
    void updatePointerExit(const PointerExitEvent& _event);
    void updateMouseButton(const MouseButtonEvent& _event);
    void updateMouseWheel(const MouseWheelEvent& _event);
    void updateKey(const KeyEvent& _event);
    void updateTextInput(const TextInputEvent& _event);
    void updatePointerCaptureLost(const PointerCaptureLostEvent& _event);
    void updateViewportDeactivated(const ViewportDeactivatedEvent& _event);

    WidgetPointer findWidgetAt(const Position& _position) const;
    bool isPathResponderValid(const WidgetPath& _path, size_t _responderIndex) const;
    bool applyReply(const WidgetPointer& _responder, InputReply _reply);
    void clearPointerCapture();
    void cancelWindowDrag(const WindowPointer& _window);
    void clear();
    void refresh();

    InputState& m_state;
    SurfaceRoot& m_root;
    InputContext& m_context;
};

void ego::gui::InputPass::Run(InputState& _state, SurfaceRoot& _root, InputContext& _context, const InputEvent& _event)
{
    InputPass pass(_state, _root, _context);
    pass.dispatch(_event);
}

ego::gui::InputPass::InputPass(InputState& _state, SurfaceRoot& _root, InputContext& _context)
    : m_state(_state),
      m_root(_root),
      m_context(_context)
{
}

bool ego::gui::InputPass::ContainsWidget(const WidgetPath& _path, const WidgetPointer& _widget)
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

void ego::gui::InputPass::dispatch(const InputEvent& _event)
{
    if (!StateAccessor::BeginDispatch(m_state))
    {
        return;
    }

    _event.dispatch(*this);
    StateAccessor::EndDispatch(m_state);

    if (StateAccessor::IsClearPending(m_state))
    {
        clear();
    }
    else if (!WidgetAccessor::IsLayoutInvalidated(m_root))
    {
        refresh();
    }
}

void ego::gui::InputPass::process(const PointerMoveEvent& _event)
{
    updatePointerMove(_event);
}

void ego::gui::InputPass::process(const PointerExitEvent& _event)
{
    updatePointerExit(_event);
}

void ego::gui::InputPass::process(const MouseButtonEvent& _event)
{
    updateMouseButton(_event);
}

void ego::gui::InputPass::process(const MouseWheelEvent& _event)
{
    updateMouseWheel(_event);
}

void ego::gui::InputPass::process(const KeyEvent& _event)
{
    updateKey(_event);
}

void ego::gui::InputPass::process(const TextInputEvent& _event)
{
    updateTextInput(_event);
}

void ego::gui::InputPass::process(const PointerCaptureLostEvent& _event)
{
    updatePointerCaptureLost(_event);
}

void ego::gui::InputPass::process(const ViewportDeactivatedEvent& _event)
{
    updateViewportDeactivated(_event);
}

void ego::gui::InputPass::updatePointerMove(const PointerMoveEvent& _event)
{
    StateAccessor::UpdatePointer(m_state, _event.m_position, _event.m_screenPosition, _event.m_modifiers);

    const bool isPointerInsideViewport = m_root.getLayoutBounds().contains(_event.m_position);
    const WidgetPath hoverPath = StateAccessor::BuildPath(m_state, findWidgetAt(_event.m_position));
    const WidgetPath capturePath = StateAccessor::BuildPath(m_state, StateAccessor::GetPointerCapture(m_state));
    const WidgetPath& eventPath = !capturePath.empty() ? capturePath : hoverPath;

    StateAccessor::UpdateHover(m_state, hoverPath);
    if (!isPointerInsideViewport)
    {
        RootAccessor::ClearPreview(m_root);
    }

    if (ContextAccessor::UpdateCapturedWindowDrag(m_context, _event.m_position))
    {
        return;
    }

    for (size_t responderIndex = 0; responderIndex < eventPath.size(); ++responderIndex)
    {
        if (!isPathResponderValid(eventPath, responderIndex))
        {
            break;
        }

        const WidgetPointer& responder = eventPath[responderIndex];
        if (applyReply(responder, WidgetAccessor::OnPointerMove(*responder, m_context, _event)))
        {
            break;
        }
    }
}

void ego::gui::InputPass::updatePointerExit(const PointerExitEvent& _event)
{
    StateAccessor::UpdatePointer(m_state, _event.m_position, _event.m_screenPosition, _event.m_modifiers);

    StateAccessor::UpdateHover(m_state, WidgetPath());
    RootAccessor::ClearPreview(m_root);
}

void ego::gui::InputPass::updateMouseButton(const MouseButtonEvent& _event)
{
    if (_event.m_key < MouseInputKey::ButtonLeft || _event.m_key > MouseInputKey::ButtonEight)
    {
        return;
    }

    StateAccessor::UpdatePointer(m_state, _event.m_position, _event.m_screenPosition, _event.m_modifiers);

    const bool isPointerInsideViewport = m_root.getLayoutBounds().contains(_event.m_position);
    const WidgetPointer captureWidget = StateAccessor::GetPointerCapture(m_state);
    const WidgetPointer hitWidget = findWidgetAt(_event.m_position);
    if (_event.m_action == InputButtonAction::Pressed && isPointerInsideViewport && !StateAccessor::IsInputTarget(m_state, captureWidget))
    {
        RootAccessor::BringWidgetToFront(m_root, hitWidget);
    }

    const WidgetPath hoverPath = StateAccessor::BuildPath(m_state, hitWidget);
    const WidgetPath capturePath = StateAccessor::BuildPath(m_state, captureWidget);
    const bool canRouteToCapture = !capturePath.empty() && (isPointerInsideViewport || _event.m_action == InputButtonAction::Released);
    const WidgetPath& eventPath = canRouteToCapture ? capturePath : hoverPath;

    StateAccessor::UpdateHover(m_state, hoverPath);
    if (_event.m_key == MouseInputKey::ButtonLeft && _event.m_action == InputButtonAction::Released &&
        ContextAccessor::FinishCapturedWindowDrag(m_context, _event.m_position))
    {
        StateAccessor::ReleasePointerButton(m_state, _event.m_key);

        return;
    }

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
        const InputReply reply = WidgetAccessor::OnMouseButton(*responder, m_context, _event);
        if (!applyReply(responder, reply))
        {
            continue;
        }

        const bool focusAcquired = (reply == InputReply::Focus || reply == InputReply::FocusAndCapture) && m_state.getFocusedWidget().get() == responder.get();
        bool captureAcquired = false;
        if (_event.m_action == InputButtonAction::Pressed && isPointerInsideViewport && (reply == InputReply::Capture || reply == InputReply::FocusAndCapture))
        {
            captureAcquired = StateAccessor::CapturePointer(m_state, responder, _event.m_key);
            if (!captureAcquired)
            {
                WidgetAccessor::OnPointerCaptureLost(*responder, _event.m_position);
                ContextAccessor::CancelWindowDragAfterCaptureFailure(m_context);
                if (captureWidget.get() == responder.get())
                {
                    StateAccessor::ResetPointerCaptureIfOwned(m_state, responder);
                }
            }
        }
        replyPreservesFocus = focusAcquired || captureAcquired;

        break;
    }

    if (_event.m_action == InputButtonAction::Pressed)
    {
        const WidgetPointer focusedWidget = m_state.getFocusedWidget();
        if (isPointerInsideViewport && !replyPreservesFocus && focusedWidget && !ContainsWidget(eventPath, focusedWidget))
        {
            StateAccessor::SetFocusedWidget(m_state, nullptr);
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
            StateAccessor::ReleasePointerButton(m_state, _event.m_key);
        }
    }
}

void ego::gui::InputPass::updateMouseWheel(const MouseWheelEvent& _event)
{
    StateAccessor::UpdatePointer(m_state, _event.m_position, _event.m_screenPosition, _event.m_modifiers);

    const WidgetPath hoverPath = StateAccessor::BuildPath(m_state, findWidgetAt(_event.m_position));
    StateAccessor::UpdateHover(m_state, hoverPath);

    for (size_t responderIndex = 0; responderIndex < hoverPath.size(); ++responderIndex)
    {
        if (!isPathResponderValid(hoverPath, responderIndex))
        {
            break;
        }

        const WidgetPointer& responder = hoverPath[responderIndex];
        if (applyReply(responder, WidgetAccessor::OnMouseWheel(*responder, m_context, _event)))
        {
            break;
        }
    }
}

void ego::gui::InputPass::updateKey(const KeyEvent& _event)
{
    const WidgetPath path = StateAccessor::BuildPath(m_state, m_state.getFocusedWidget());
    for (size_t responderIndex = 0; responderIndex < path.size(); ++responderIndex)
    {
        if (!isPathResponderValid(path, responderIndex))
        {
            break;
        }

        const WidgetPointer& responder = path[responderIndex];
        if (applyReply(responder, WidgetAccessor::OnKey(*responder, m_context, _event)))
        {
            break;
        }
    }
}

void ego::gui::InputPass::updateTextInput(const TextInputEvent& _event)
{
    const WidgetPath path = StateAccessor::BuildPath(m_state, m_state.getFocusedWidget());
    for (size_t responderIndex = 0; responderIndex < path.size(); ++responderIndex)
    {
        if (!isPathResponderValid(path, responderIndex))
        {
            break;
        }

        const WidgetPointer& responder = path[responderIndex];
        if (applyReply(responder, WidgetAccessor::OnTextInput(*responder, m_context, _event)))
        {
            break;
        }
    }
}

void ego::gui::InputPass::updatePointerCaptureLost(const PointerCaptureLostEvent&)
{
    clearPointerCapture();
}

void ego::gui::InputPass::updateViewportDeactivated(const ViewportDeactivatedEvent&)
{
    clear();
}

ego::gui::WidgetPointer ego::gui::InputPass::findWidgetAt(const Position& _position) const
{
    return StateAccessor::FindWidgetAt(m_state, _position);
}

bool ego::gui::InputPass::isPathResponderValid(const WidgetPath& _path, size_t _responderIndex) const
{
    if (_responderIndex >= _path.size() || !StateAccessor::IsInputTarget(m_state, _path[_responderIndex]))
    {
        return false;
    }

    return _responderIndex == 0 ||
           (StateAccessor::IsInputTarget(m_state, _path[_responderIndex - 1]) && _path[_responderIndex - 1]->isDirectChildOf(*_path[_responderIndex]));
}

bool ego::gui::InputPass::applyReply(const WidgetPointer& _responder, InputReply _reply)
{
    if (_reply == InputReply::Unhandled)
    {
        return false;
    }

    if ((_reply == InputReply::Focus || _reply == InputReply::FocusAndCapture) && StateAccessor::IsInputTarget(m_state, _responder))
    {
        StateAccessor::SetFocusedWidget(m_state, _responder);
    }
    else if (_reply == InputReply::ClearFocus)
    {
        StateAccessor::SetFocusedWidget(m_state, nullptr);
    }

    return true;
}

void ego::gui::InputPass::clearPointerCapture()
{
    cancelWindowDrag(StateAccessor::ClearPointerCapture(m_state));
}

void ego::gui::InputPass::cancelWindowDrag(const WindowPointer& _window)
{
    if (_window)
    {
        m_context.cancelWindowDrag(_window);
    }
}

void ego::gui::InputPass::clear()
{
    if (!StateAccessor::BeginClear(m_state))
    {
        return;
    }

    cancelWindowDrag(StateAccessor::ClearState(m_state));
    StateAccessor::EndClear(m_state);
}

void ego::gui::InputPass::refresh()
{
    if (!StateAccessor::BeginRefresh(m_state))
    {
        return;
    }

    cancelWindowDrag(StateAccessor::ApplyPointerCaptureCancellation(m_state));
    cancelWindowDrag(StateAccessor::RefreshState(m_state));
    StateAccessor::EndRefresh(m_state);

    if (StateAccessor::IsClearPending(m_state))
    {
        clear();

        return;
    }

    const bool refreshStarted = StateAccessor::BeginRefresh(m_state);
    EGO_ASSERT(refreshStarted);
    if (!refreshStarted)
    {
        return;
    }

    cancelWindowDrag(StateAccessor::ApplyPointerCaptureCancellation(m_state));
    StateAccessor::EndRefresh(m_state);

    if (StateAccessor::IsClearPending(m_state))
    {
        clear();
    }
}

bool ego::gui::InputState::InputPassAccessor::BeginDispatch(InputState& _state)
{
    EGO_ASSERT(!_state.m_isDispatchingInput);
    if (_state.m_isDispatchingInput)
    {
        return false;
    }

    _state.m_isDispatchingInput = true;
    ++_state.m_dispatchDepth;

    return true;
}

void ego::gui::InputState::InputPassAccessor::EndDispatch(InputState& _state)
{
    EGO_ASSERT(_state.m_isDispatchingInput && _state.m_dispatchDepth > 0);

    --_state.m_dispatchDepth;
    _state.m_isDispatchingInput = false;
}

bool ego::gui::InputState::InputPassAccessor::IsClearPending(const InputState& _state)
{
    return _state.m_isClearPending;
}

bool ego::gui::InputState::InputPassAccessor::BeginClear(InputState& _state)
{
    if (_state.m_dispatchDepth > 0)
    {
        _state.m_isClearPending = true;

        return false;
    }

    _state.m_isClearPending = false;
    ++_state.m_dispatchDepth;

    return true;
}

void ego::gui::InputState::InputPassAccessor::EndClear(InputState& _state)
{
    EGO_ASSERT(_state.m_dispatchDepth > 0);

    --_state.m_dispatchDepth;
    _state.m_isClearPending = false;
}

bool ego::gui::InputState::InputPassAccessor::BeginRefresh(InputState& _state)
{
    if (_state.m_dispatchDepth > 0)
    {
        return false;
    }

    ++_state.m_dispatchDepth;

    return true;
}

void ego::gui::InputState::InputPassAccessor::EndRefresh(InputState& _state)
{
    EGO_ASSERT(_state.m_dispatchDepth > 0);
    --_state.m_dispatchDepth;
}

void ego::gui::InputState::InputPassAccessor::UpdatePointer(
    InputState& _state,
    const Position& _position,
    const Position& _screenPosition,
    const InputModifiers& _modifiers)
{
    _state.m_pointer.m_position = _position;
    _state.m_pointer.m_screenPosition = _screenPosition;
    _state.m_pointer.m_modifiers = _modifiers;
}

ego::gui::WidgetPointer ego::gui::InputState::InputPassAccessor::GetPointerCapture(const InputState& _state)
{
    return _state.m_pointer.m_captureWidget.lock();
}

void ego::gui::InputState::InputPassAccessor::ResetPointerCaptureIfOwned(InputState& _state, const WidgetPointer& _widget)
{
    const WidgetPointer captureWidget = _state.m_pointer.m_captureWidget.lock();
    if (captureWidget.get() == _widget.get())
    {
        _state.m_pointer.m_captureWidget.reset();
        _state.m_pointer.m_captureButtons = 0;
    }
}

ego::gui::WidgetPointer ego::gui::InputState::InputPassAccessor::FindWidgetAt(const InputState& _state, const Position& _position)
{
    return _state.findWidgetAt(_position);
}

bool ego::gui::InputState::InputPassAccessor::IsInputTarget(const InputState& _state, const WidgetPointer& _widget)
{
    return _state.isInputTarget(_widget);
}

std::vector<ego::gui::WidgetPointer> ego::gui::InputState::InputPassAccessor::BuildPath(const InputState& _state, const WidgetPointer& _target)
{
    return _state.buildPath(_target);
}

void ego::gui::InputState::InputPassAccessor::UpdateHover(InputState& _state, const std::vector<WidgetPointer>& _path)
{
    _state.updateHover(_path);
}

void ego::gui::InputState::InputPassAccessor::SetFocusedWidget(InputState& _state, const WidgetPointer& _widget)
{
    _state.setFocusedWidget(_widget);
}

bool ego::gui::InputState::InputPassAccessor::CapturePointer(InputState& _state, const WidgetPointer& _widget, MouseInputKey _key)
{
    return _state.capturePointer(_widget, _key);
}

void ego::gui::InputState::InputPassAccessor::ReleasePointerButton(InputState& _state, MouseInputKey _key)
{
    _state.releasePointerButton(_key);
}

ego::gui::WindowPointer ego::gui::InputState::InputPassAccessor::ClearPointerCapture(InputState& _state)
{
    return _state.clearPointerCapture();
}

ego::gui::WindowPointer ego::gui::InputState::InputPassAccessor::ApplyPointerCaptureCancellation(InputState& _state)
{
    return _state.applyPointerCaptureCancellation();
}

ego::gui::WindowPointer ego::gui::InputState::InputPassAccessor::ClearState(InputState& _state)
{
    return _state.clearState();
}

ego::gui::WindowPointer ego::gui::InputState::InputPassAccessor::RefreshState(InputState& _state)
{
    return _state.refreshState();
}

bool ego::gui::InputState::WindowDragAccessor::TransferPointerCapture(InputState& _state, const WidgetPointer& _source, const WidgetPointer& _target)
{
    return _state.transferPointerCapture(_source, _target);
}

ego::gui::InputState::InputState(SurfaceRoot& _root)
    : m_root(_root),
      m_windowDrag(std::make_unique<WindowDrag>())
{
}

ego::gui::InputState::~InputState() = default;

void ego::gui::InputState::dispatch(const InputEvent& _event, ViewportInputContext& _viewportInput)
{
    WindowDragContext windowDragContext{
        .m_inputState = *this,
        .m_root = m_root,
        .m_viewportInput = _viewportInput,
    };
    InputContext context = ContextAccessor::Create(*m_windowDrag, windowDragContext, m_pointer.m_screenPosition);
    InputPass::Run(*this, m_root, context, _event);
}

void ego::gui::InputState::refresh()
{
    if (m_dispatchDepth > 0)
    {
        return;
    }

    ++m_dispatchDepth;
    m_windowDrag->clearLocal(m_root, applyPointerCaptureCancellation());
    m_windowDrag->clearLocal(m_root, refreshState());

    --m_dispatchDepth;

    if (m_isClearPending)
    {
        clear();

        return;
    }

    ++m_dispatchDepth;

    m_windowDrag->clearLocal(m_root, applyPointerCaptureCancellation());

    --m_dispatchDepth;

    if (m_isClearPending)
    {
        clear();
    }
}

void ego::gui::InputState::clear()
{
    if (m_dispatchDepth > 0)
    {
        m_isClearPending = true;

        return;
    }

    m_isClearPending = false;
    ++m_dispatchDepth;

    m_windowDrag->clearLocal(m_root, clearState());

    --m_dispatchDepth;
    m_isClearPending = false;
}

void ego::gui::InputState::detachInputState(const WidgetPointer& _subtree)
{
    const auto isAffected = [this, &_subtree](const WidgetPointer& _widget)
    {
        return _widget && (!_subtree || _widget.get() == _subtree.get() || _widget->isDescendantOf(*_subtree) || !isInputTarget(_widget));
    };

    WidgetPath hoverPath;
    WeakWidgetPath remainingHoverPath;
    for (const WidgetWeakPointer& hoveredWidget : m_pointer.m_hoverPath)
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

    const WidgetPointer focusedWidget = m_focusedWidget.lock();
    const WidgetPointer captureWidget = m_pointer.m_captureWidget.lock();
    const WidgetPointer captureCancellationTarget = m_pointerCaptureCancellationTarget.lock();
    const WindowPointer draggedWindow = m_windowDrag->getWindow();
    const WidgetPointer draggedWidget = ego::StaticPointerCast<Widget>(draggedWindow);
    const bool clearFocus = isAffected(focusedWidget);
    const bool clearCapture = isAffected(captureWidget);
    const bool clearWindowDrag = isAffected(draggedWidget);

    m_pointer.m_hoverPath = std::move(remainingHoverPath);

    if (clearFocus)
    {
        m_focusedWidget.reset();
    }

    if (!_subtree || clearCapture || !captureWidget)
    {
        m_pointer.m_captureWidget.reset();
        m_pointer.m_captureButtons = 0;
    }

    if (!_subtree || isAffected(captureCancellationTarget))
    {
        m_pointerCaptureCancellationTarget.reset();
    }

    ++m_dispatchDepth;
    for (const WidgetPointer& widget : hoverPath)
    {
        WidgetAccessor::OnPointerLeave(*widget, m_pointer.m_position, m_pointer.m_modifiers);
    }

    if (clearFocus)
    {
        WidgetAccessor::OnFocusChanged(*focusedWidget, FocusChange::Lost);
    }

    if (clearCapture)
    {
        WidgetAccessor::OnPointerCaptureLost(*captureWidget, m_pointer.m_position);
        RootAccessor::ClearInteraction(m_root);
    }

    if (clearWindowDrag)
    {
        m_windowDrag->clearLocal(m_root, draggedWindow);
    }

    --m_dispatchDepth;
    if (m_dispatchDepth == 0 && m_isClearPending)
    {
        clear();
    }
}

void ego::gui::InputState::requestPointerCaptureCancellation(const WidgetPointer& _relatedWidget)
{
    if (m_pointerCaptureCancellationTarget.lock())
    {
        return;
    }

    const WidgetPointer captureWidget = m_pointer.m_captureWidget.lock();
    if (!captureWidget)
    {
        m_pointer.m_captureButtons = 0;

        return;
    }

    if (_relatedWidget && captureWidget.get() != _relatedWidget.get() && !captureWidget->isDescendantOf(*_relatedWidget) &&
        !_relatedWidget->isDescendantOf(*captureWidget) && isInputTarget(captureWidget))
    {
        return;
    }

    m_pointerCaptureCancellationTarget = captureWidget;
}

void ego::gui::InputState::flushPointerCaptureCancellation()
{
    if (m_dispatchDepth == 0)
    {
        ++m_dispatchDepth;
        m_windowDrag->clearLocal(m_root, applyPointerCaptureCancellation());
        --m_dispatchDepth;
    }
}

void ego::gui::InputState::cancelWindowDrag(const WindowPointer& _window)
{
    m_windowDrag->clearLocal(m_root, _window);
}

ego::gui::WidgetPointer ego::gui::InputState::getFocusedWidget() const
{
    const WidgetPointer focusedWidget = m_focusedWidget.lock();

    return isInputTarget(focusedWidget) ? focusedWidget : nullptr;
}

bool ego::gui::InputState::isWindowDragActive(const WindowPointer& _window) const
{
    return m_windowDrag->isActive(_window);
}

ego::gui::WindowPointer ego::gui::InputState::getDraggedWindow() const
{
    return m_windowDrag->getWindow();
}

ego::gui::InputState::MouseButtonMask ego::gui::InputState::GetMouseButtonMask(MouseInputKey _key)
{
    const uint32_t buttonIndex = static_cast<uint32_t>(_key) - static_cast<uint32_t>(MouseInputKey::ButtonLeft);

    return static_cast<MouseButtonMask>(1u << buttonIndex);
}

bool ego::gui::InputState::ContainsDirectChild(const Widget& _parent, const Widget& _child)
{
    const size_t childCount = WidgetAccessor::GetChildCount(_parent);
    for (size_t childIndex = 0; childIndex < childCount; ++childIndex)
    {
        const WidgetPointer child = WidgetAccessor::GetChild(_parent, childIndex);
        if (child.get() == &_child && WidgetAccessor::IsChildActive(_parent, childIndex))
        {
            return true;
        }
    }

    return false;
}

bool ego::gui::InputState::transferPointerCapture(const WidgetPointer& _source, const WidgetPointer& _target)
{
    const WidgetPointer captureWidget = m_pointer.m_captureWidget.lock();
    if (!_source || captureWidget.get() != _source.get() || m_pointer.m_captureButtons == 0 || !isInputTarget(_target))
    {
        return false;
    }

    m_pointer.m_captureWidget = _target;

    return true;
}

ego::gui::WidgetPointer ego::gui::InputState::findWidgetAt(const Position& _position) const
{
    if (!WidgetAccessor::HitTest(m_root, _position))
    {
        return nullptr;
    }

    WidgetPointer currentWidget = m_root.sharedFromThis();
    while (currentWidget && WidgetAccessor::IsChildHitTestVisible(*currentWidget, _position))
    {
        WidgetPointer hitWidget = nullptr;
        const size_t childCount = WidgetAccessor::GetChildCount(*currentWidget);
        for (size_t childIndex = childCount; childIndex > 0; --childIndex)
        {
            const WidgetPointer child = WidgetAccessor::GetChild(*currentWidget, childIndex - 1);
            if (child && WidgetAccessor::IsChildActive(*currentWidget, childIndex - 1) && child->isDirectChildOf(*currentWidget) &&
                WidgetAccessor::HitTest(*child, _position))
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

bool ego::gui::InputState::isInputTarget(const WidgetPointer& _widget) const
{
    if (!_widget)
    {
        return false;
    }

    WidgetPointer currentWidget = _widget;
    while (currentWidget)
    {
        const Rect& bounds = currentWidget->getLayoutBounds();
        if (!currentWidget->isVisible() || (currentWidget.get() != &m_root && (bounds.m_size.m_x <= 0.0f || bounds.m_size.m_y <= 0.0f)))
        {
            return false;
        }

        if (currentWidget.get() == &m_root)
        {
            return true;
        }

        const WidgetPointer parent = currentWidget->getParent();
        if (!parent || !ContainsDirectChild(*parent, *currentWidget))
        {
            return false;
        }

        currentWidget = parent;
    }

    return false;
}

ego::gui::InputState::WidgetPath ego::gui::InputState::buildPath(const WidgetPointer& _target) const
{
    WidgetPath path;
    if (!isInputTarget(_target))
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

void ego::gui::InputState::setFocusedWidget(const WidgetPointer& _widget)
{
    const WidgetPointer targetWidget = isInputTarget(_widget) ? _widget : nullptr;
    const WidgetPointer focusedWidget = m_focusedWidget.lock();
    if (focusedWidget.get() == targetWidget.get())
    {
        return;
    }

    m_focusedWidget.reset();
    if (focusedWidget)
    {
        WidgetAccessor::OnFocusChanged(*focusedWidget, FocusChange::Lost);
    }

    if (isInputTarget(targetWidget))
    {
        m_focusedWidget = targetWidget;
        WidgetAccessor::OnFocusChanged(*targetWidget, FocusChange::Gained);
    }
}

bool ego::gui::InputState::capturePointer(const WidgetPointer& _widget, MouseInputKey _key)
{
    if (!isInputTarget(_widget))
    {
        return false;
    }

    const WidgetPointer captureWidget = m_pointer.m_captureWidget.lock();
    if (captureWidget && captureWidget.get() != _widget.get())
    {
        return false;
    }

    m_pointer.m_captureWidget = _widget;
    m_pointer.m_captureButtons |= GetMouseButtonMask(_key);

    return true;
}

void ego::gui::InputState::releasePointerButton(MouseInputKey _key)
{
    m_pointer.m_captureButtons &= static_cast<MouseButtonMask>(~GetMouseButtonMask(_key));
    if (m_pointer.m_captureButtons == 0)
    {
        m_pointer.m_captureWidget.reset();
    }
}

ego::gui::WindowPointer ego::gui::InputState::clearPointerCapture()
{
    const WindowPointer draggedWindow = m_windowDrag->getWindow();
    const WidgetPointer captureWidget = m_pointer.m_captureWidget.lock();
    m_pointer.m_captureWidget.reset();
    m_pointer.m_captureButtons = 0;

    if (!captureWidget)
    {
        return draggedWindow;
    }

    WidgetAccessor::OnPointerCaptureLost(*captureWidget, m_pointer.m_position);
    RootAccessor::ClearInteraction(m_root);

    return draggedWindow;
}

ego::gui::WindowPointer ego::gui::InputState::applyPointerCaptureCancellation()
{
    const WidgetPointer captureCancellationTarget = m_pointerCaptureCancellationTarget.lock();
    m_pointerCaptureCancellationTarget.reset();
    if (!captureCancellationTarget)
    {
        return nullptr;
    }

    const WidgetPointer captureWidget = m_pointer.m_captureWidget.lock();
    if (captureWidget.get() != captureCancellationTarget.get())
    {
        return nullptr;
    }

    return clearPointerCapture();
}

ego::gui::WindowPointer ego::gui::InputState::clearState()
{
    updateHover(WidgetPath());
    setFocusedWidget(nullptr);

    const bool hadPointerCapture = static_cast<bool>(m_pointer.m_captureWidget.lock());
    const WindowPointer draggedWindow = clearPointerCapture();
    if (!hadPointerCapture)
    {
        RootAccessor::ClearInteraction(m_root);
    }

    m_pointer = PointerState();
    m_pointerCaptureCancellationTarget.reset();

    return draggedWindow;
}

ego::gui::WindowPointer ego::gui::InputState::refreshState()
{
    const WidgetPointer focusedWidget = m_focusedWidget.lock();
    if (focusedWidget && !isInputTarget(focusedWidget))
    {
        m_focusedWidget.reset();
        WidgetAccessor::OnFocusChanged(*focusedWidget, FocusChange::Lost);
    }

    WindowPointer draggedWindow = nullptr;
    const WidgetPointer captureWidget = m_pointer.m_captureWidget.lock();
    if (!captureWidget)
    {
        m_pointer.m_captureButtons = 0;
    }
    else if (!isInputTarget(captureWidget))
    {
        draggedWindow = clearPointerCapture();
    }

    if (!m_pointer.m_hoverPath.empty() || m_pointer.m_captureWidget.lock())
    {
        updateHover(buildPath(findWidgetAt(m_pointer.m_position)));
    }

    return draggedWindow;
}

void ego::gui::InputState::updateHover(const WidgetPath& _path)
{
    WidgetPath previousPath;
    previousPath.reserve(m_pointer.m_hoverPath.size());
    for (const WidgetWeakPointer& hoveredWidget : m_pointer.m_hoverPath)
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

    m_pointer.m_hoverPath.assign(_path.begin() + enterCount, _path.end());

    for (size_t widgetIndex = 0; widgetIndex < leaveCount; ++widgetIndex)
    {
        const WidgetPointer& widget = previousPath[widgetIndex];
        if (widget)
        {
            WidgetAccessor::OnPointerLeave(*widget, m_pointer.m_position, m_pointer.m_modifiers);
        }
    }

    for (size_t widgetIndex = enterCount; widgetIndex > 0; --widgetIndex)
    {
        const WidgetPointer& widget = _path[widgetIndex - 1];
        const bool hasExpectedParent = widgetIndex == _path.size() || widget->isDirectChildOf(*_path[widgetIndex]);
        if (!hasExpectedParent || !isInputTarget(widget))
        {
            break;
        }

        m_pointer.m_hoverPath.insert(m_pointer.m_hoverPath.begin(), widget);
        WidgetAccessor::OnPointerEnter(*widget, m_pointer.m_position, m_pointer.m_modifiers);
    }
}
