#include "InputRouter.h"

#include <cstddef>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Viewport/SurfaceRoot.h"

ego::gui::InputRouter::ProcessingScope::ProcessingScope(InputRouter& _router)
    : m_router(_router.sharedFromThis())
{
    if (!m_router)
    {
        EGO_ASSERT_FAIL_MESSAGE("Input router must have a shared owner.");
        return;
    }
    if (m_router->m_isProcessing)
    {
        EGO_ASSERT_FAIL_MESSAGE("Input routing cannot be reentered.");
        return;
    }

    m_router->m_isProcessing = true;
    m_isActive = true;
}

ego::gui::InputRouter::ProcessingScope::~ProcessingScope()
{
    if (m_isActive)
    {
        m_router->m_isProcessing = false;
    }
}

ego::gui::InputRouter::ProcessingScope::operator bool() const
{
    return m_isActive;
}

ego::gui::InputRouter::~InputRouter()
{
    clearState();
    m_root = nullptr;
}

ego::SharedPointer<ego::gui::InputRouter> ego::gui::InputRouter::Create(const ego::SharedPointer<SurfaceRoot>& _root)
{
    if (!_root)
    {
        return nullptr;
    }

    return new InputRouter(_root);
}

ego::gui::InputRouter::InputRouter(const ego::SharedPointer<SurfaceRoot>& _root)
    : m_root(_root)
{
}

void ego::gui::InputRouter::process(const InputEvent& _event)
{
    const ProcessingScope processingScope(*this);
    if (!processingScope)
    {
        return;
    }

    _event.sendTo(*this);
}

void ego::gui::InputRouter::onPointerMove(const PointerMoveEvent& _event)
{
    PointerState& pointerState = m_pointerState;
    pointerState.m_position = _event.m_position;
    pointerState.m_modifiers = _event.m_modifiers;

    const bool isPointerInsideViewport = m_root->getLayoutBounds().contains(_event.m_position);
    const WidgetPointer hitWidget = m_root->findWidgetAt(_event.m_position);
    const WidgetPath hoverPath = buildPath(hitWidget);
    const WidgetPath capturePath = buildPath(pointerState.m_captureWidget.lock());
    const WidgetPath& eventPath = isPointerInsideViewport && !capturePath.empty() ? capturePath : hoverPath;

    updateHover(hoverPath);

    for (const WidgetPointer& responder : eventPath)
    {
        if (applyReply(responder, responder->onPointerMove(_event)))
        {
            break;
        }
    }
}

void ego::gui::InputRouter::onPointerExit(const PointerExitEvent& _event)
{
    PointerState& pointerState = m_pointerState;
    pointerState.m_position = _event.m_position;
    pointerState.m_modifiers = _event.m_modifiers;

    updateHover(WidgetPath());
}

void ego::gui::InputRouter::onMouseButton(const MouseButtonEvent& _event)
{
    if (_event.m_key < MouseInputKey::ButtonLeft || _event.m_key > MouseInputKey::ButtonEight)
    {
        return;
    }

    PointerState& pointerState = m_pointerState;
    pointerState.m_position = _event.m_position;
    pointerState.m_modifiers = _event.m_modifiers;

    const bool isPointerInsideViewport = m_root->getLayoutBounds().contains(_event.m_position);
    const WidgetPointer captureWidget = pointerState.m_captureWidget.lock();
    const WidgetPointer hitWidget = m_root->findWidgetAt(_event.m_position);
    if (_event.m_action == InputButtonAction::Pressed && isPointerInsideViewport && !m_root->isInputTarget(captureWidget))
    {
        m_root->bringWidgetToFront(hitWidget);
    }

    const WidgetPath hoverPath = buildPath(hitWidget);
    const WidgetPath capturePath = buildPath(captureWidget);
    const bool canRouteToCapture = !capturePath.empty() && (isPointerInsideViewport || _event.m_action == InputButtonAction::Released);
    const WidgetPath& eventPath = canRouteToCapture ? capturePath : hoverPath;

    updateHover(hoverPath);

    for (const WidgetPointer& responder : eventPath)
    {
        const InputReply reply = responder->onMouseButton(_event);
        if (!applyReply(responder, reply))
        {
            continue;
        }

        if (_event.m_action == InputButtonAction::Pressed && isPointerInsideViewport && reply == InputReply::FocusAndCapture)
        {
            capturePointer(responder, _event.m_key);
        }
        break;
    }

    if (_event.m_action == InputButtonAction::Pressed)
    {
        const WidgetPointer focusedWidget = getFocusedWidget();
        if (isPointerInsideViewport && focusedWidget && !ContainsWidget(eventPath, focusedWidget))
        {
            setFocusedWidget(nullptr);
        }
    }
    else
    {
        releasePointerButton(_event.m_key);
    }
}

void ego::gui::InputRouter::onMouseWheel(const MouseWheelEvent& _event)
{
    PointerState& pointerState = m_pointerState;
    pointerState.m_position = _event.m_position;
    pointerState.m_modifiers = _event.m_modifiers;

    const WidgetPointer hitWidget = m_root->findWidgetAt(_event.m_position);
    const WidgetPath hoverPath = buildPath(hitWidget);

    updateHover(hoverPath);

    for (const WidgetPointer& responder : hoverPath)
    {
        if (applyReply(responder, responder->onMouseWheel(_event)))
        {
            break;
        }
    }
}

void ego::gui::InputRouter::onKey(const KeyEvent& _event)
{
    const WidgetPath path = buildPath(getFocusedWidget());
    for (const WidgetPointer& responder : path)
    {
        if (applyReply(responder, responder->onKey(_event)))
        {
            break;
        }
    }
}

void ego::gui::InputRouter::onTextInput(const TextInputEvent& _event)
{
    const WidgetPath path = buildPath(getFocusedWidget());
    for (const WidgetPointer& responder : path)
    {
        if (applyReply(responder, responder->onTextInput(_event)))
        {
            break;
        }
    }
}

void ego::gui::InputRouter::onViewportDeactivated(const ViewportDeactivatedEvent&)
{
    clearState();
}

void ego::gui::InputRouter::refreshAfterLayout()
{
    const ProcessingScope processingScope(*this);
    if (!processingScope)
    {
        return;
    }

    const WidgetPointer focusLostWidget = takeInvalidTarget(m_focusedWidget);
    const WidgetPointer captureLostWidget = takeInvalidTarget(m_pointerState.m_captureWidget);
    if (!m_pointerState.m_captureWidget.lock())
    {
        m_pointerState.m_captureButtons = 0;
    }

    const bool shouldUpdateHover = !m_pointerState.m_hoverPath.empty() || m_pointerState.m_captureWidget.lock();
    WidgetPath hoverPath;
    if (shouldUpdateHover)
    {
        hoverPath = buildPath(m_root->findWidgetAt(m_pointerState.m_position));
    }

    updateHover(hoverPath);

    if (focusLostWidget)
    {
        focusLostWidget->onFocusChanged(FocusChange::Lost);
    }

    if (captureLostWidget)
    {
        captureLostWidget->onPointerCaptureLost(m_pointerState.m_position);
    }
}

void ego::gui::InputRouter::clear()
{
    const ProcessingScope processingScope(*this);
    if (!processingScope)
    {
        return;
    }

    clearState();
}

void ego::gui::InputRouter::clearState()
{
    updateHover(WidgetPath());

    const WidgetPointer focusedWidget = m_focusedWidget.lock();
    m_focusedWidget.reset();
    if (focusedWidget)
    {
        focusedWidget->onFocusChanged(FocusChange::Lost);
    }

    clearPointerCapture();
    m_pointerState = PointerState();
}

ego::gui::WidgetPointer ego::gui::InputRouter::getFocusedWidget() const
{
    const WidgetPointer focusedWidget = m_focusedWidget.lock();
    return m_root->isInputTarget(focusedWidget) ? focusedWidget : nullptr;
}

ego::gui::InputRouter::MouseButtonMask ego::gui::InputRouter::GetMouseButtonMask(MouseInputKey _key)
{
    const uint32_t buttonIndex = static_cast<uint32_t>(_key) - static_cast<uint32_t>(MouseInputKey::ButtonLeft);
    return static_cast<MouseButtonMask>(1u << buttonIndex);
}

bool ego::gui::InputRouter::ContainsWidget(const WidgetPath& _path, const WidgetPointer& _widget)
{
    if (!_widget)
    {
        return false;
    }

    for (const WidgetPointer& widget : _path)
    {
        if (widget.get() == _widget.get())
        {
            return true;
        }
    }
    return false;
}

ego::gui::InputRouter::WidgetPath ego::gui::InputRouter::buildPath(const WidgetPointer& _target) const
{
    WidgetPath path;
    if (!m_root->isInputTarget(_target))
    {
        return path;
    }

    WidgetPointer widget = _target;
    while (widget)
    {
        path.push_back(widget);
        if (widget.get() == m_root.get())
        {
            return path;
        }

        widget = widget->getParent();
    }

    path.clear();
    return path;
}

ego::gui::WidgetPointer ego::gui::InputRouter::takeInvalidTarget(WidgetWeakPointer& _target)
{
    const WidgetPointer widget = _target.lock();
    if (widget && m_root->isInputTarget(widget))
    {
        return nullptr;
    }

    _target.reset();
    return widget;
}

bool ego::gui::InputRouter::applyReply(const WidgetPointer& _responder, InputReply _reply)
{
    if (_reply == InputReply::Unhandled)
    {
        return false;
    }

    if ((_reply == InputReply::Focus || _reply == InputReply::FocusAndCapture) && m_root->isInputTarget(_responder))
    {
        setFocusedWidget(_responder);
    }
    else if (_reply == InputReply::ClearFocus)
    {
        setFocusedWidget(nullptr);
    }
    return true;
}

void ego::gui::InputRouter::setFocusedWidget(const WidgetPointer& _widget)
{
    const WidgetPointer targetWidget = m_root->isInputTarget(_widget) ? _widget : nullptr;
    const WidgetPointer focusedWidget = m_focusedWidget.lock();
    if (focusedWidget.get() == targetWidget.get())
    {
        return;
    }

    m_focusedWidget.reset();
    if (focusedWidget)
    {
        focusedWidget->onFocusChanged(FocusChange::Lost);
    }

    if (m_root->isInputTarget(targetWidget))
    {
        m_focusedWidget = targetWidget;
        targetWidget->onFocusChanged(FocusChange::Gained);
    }
}

void ego::gui::InputRouter::capturePointer(const WidgetPointer& _widget, MouseInputKey _key)
{
    if (!m_root->isInputTarget(_widget))
    {
        return;
    }

    const WidgetPointer captureWidget = m_pointerState.m_captureWidget.lock();
    if (captureWidget && captureWidget.get() != _widget.get())
    {
        return;
    }

    m_pointerState.m_captureWidget = _widget;
    m_pointerState.m_captureButtons |= GetMouseButtonMask(_key);
}

void ego::gui::InputRouter::releasePointerButton(MouseInputKey _key)
{
    m_pointerState.m_captureButtons &= static_cast<MouseButtonMask>(~GetMouseButtonMask(_key));
    if (m_pointerState.m_captureButtons == 0)
    {
        m_pointerState.m_captureWidget.reset();
    }
}

void ego::gui::InputRouter::clearPointerCapture()
{
    const WidgetPointer captureWidget = m_pointerState.m_captureWidget.lock();
    m_pointerState.m_captureWidget.reset();
    m_pointerState.m_captureButtons = 0;
    if (captureWidget)
    {
        captureWidget->onPointerCaptureLost(m_pointerState.m_position);
    }
}

void ego::gui::InputRouter::updateHover(const WidgetPath& _path)
{
    PointerState& pointerState = m_pointerState;
    WidgetPath previousPath;
    previousPath.reserve(pointerState.m_hoverPath.size());
    for (const WidgetWeakPointer& hoveredWidget : pointerState.m_hoverPath)
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

    pointerState.m_hoverPath.assign(_path.begin(), _path.end());

    for (size_t widgetIndex = 0; widgetIndex < leaveCount; ++widgetIndex)
    {
        const WidgetPointer& widget = previousPath[widgetIndex];
        if (widget)
        {
            widget->onPointerLeave(pointerState.m_position, pointerState.m_modifiers);
        }
    }

    for (size_t widgetIndex = enterCount; widgetIndex > 0; --widgetIndex)
    {
        _path[widgetIndex - 1]->onPointerEnter(pointerState.m_position, pointerState.m_modifiers);
    }
}
