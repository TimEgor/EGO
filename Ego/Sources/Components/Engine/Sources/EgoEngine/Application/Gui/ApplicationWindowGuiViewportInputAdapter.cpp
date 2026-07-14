#include "ApplicationWindowGuiViewportInputAdapter.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Platform/Input/MouseInputDevice.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoInput/InputEvents.h"

#include "EgoGui/GuiController.h"
#include "EgoGui/GuiViewport.h"

#include "EgoApplication/Window/ApplicationWindowEvents.h"

namespace
{
    ego::gui::GuiInputEvent CreateKeyboardInputEvent(const ego::WindowKeyboardInputData& _inputData)
    {
        ego::gui::GuiInputEvent event;
        event.m_type = _inputData.m_action == ego::WindowKeyboardInputAction::Pressed ? ego::gui::GuiInputEventType::KeyDown : ego::gui::GuiInputEventType::KeyUp;
        event.m_key = _inputData.m_key;
        return event;
    }

    ego::gui::GuiInputEvent CreateTextInputEvent(const ego::WindowTextInputData& _inputData)
    {
        ego::gui::GuiInputEvent event;
        event.m_type = ego::gui::GuiInputEventType::TextInput;
        event.m_textCodepoint = _inputData.m_codepoint;
        return event;
    }

    bool IsMouseDevice(const ego::InputDevicePointer& _device)
    {
        return _device && _device->getType() == ego::MouseInputDevice::GetMetaInfoID();
    }

    ego::InputDeviceKey ToInputDeviceKey(ego::MouseInputKey _key)
    {
        return static_cast<ego::InputDeviceKey>(_key);
    }

    ego::gui::GuiPosition GetMousePosition(const ego::InputDevicePointer& _device)
    {
        if (!_device)
        {
            return ego::gui::GuiPositionZero;
        }

        return ego::gui::GuiPosition(_device->getValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)), _device->getValue(ToInputDeviceKey(ego::MouseInputKey::AxisY)));
    }

    bool IsMousePositionChanged(const ego::InputDevicePointer& _device)
    {
        if (!_device)
        {
            return false;
        }

        return _device->getValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)) != _device->getPreviousValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)) ||
               _device->getValue(ToInputDeviceKey(ego::MouseInputKey::AxisY)) != _device->getPreviousValue(ToInputDeviceKey(ego::MouseInputKey::AxisY));
    }

    bool TryGetGuiMouseButton(ego::InputDeviceKey _key, ego::gui::GuiMouseButton& _mouseButton)
    {
        if (_key >= ego::MouseInputKeyCount)
        {
            return false;
        }

        switch (static_cast<ego::MouseInputKey>(_key))
        {
        case ego::MouseInputKey::ButtonLeft:
            _mouseButton = ego::gui::GuiMouseButton::Left;
            return true;
        case ego::MouseInputKey::ButtonRight:
            _mouseButton = ego::gui::GuiMouseButton::Right;
            return true;
        case ego::MouseInputKey::ButtonMiddle:
            _mouseButton = ego::gui::GuiMouseButton::Middle;
            return true;
        default:
            return false;
        }
    }

    bool IsMouseWheelKey(ego::InputDeviceKey _key)
    {
        return _key == ToInputDeviceKey(ego::MouseInputKey::Wheel);
    }

    ego::gui::GuiInputEvent CreateMouseEvent(ego::gui::GuiInputEventType _type, const ego::InputDevicePointer& _device)
    {
        ego::gui::GuiInputEvent event;
        event.m_type = _type;
        event.m_position = GetMousePosition(_device);
        event.m_hasPosition = true;
        return event;
    }

    ego::gui::GuiInputEvent CreateMouseButtonEvent(ego::gui::GuiInputEventType _type, const ego::InputDevicePointer& _device, ego::gui::GuiMouseButton _mouseButton)
    {
        ego::gui::GuiInputEvent event = CreateMouseEvent(_type, _device);
        event.m_mouseButton = _mouseButton;
        return event;
    }

    ego::gui::GuiInputEvent CreateMouseWheelEvent(const ego::InputDevicePointer& _device, ego::InputDeviceKeyValue _wheelDelta)
    {
        ego::gui::GuiInputEvent event = CreateMouseEvent(ego::gui::GuiInputEventType::MouseWheel, _device);
        event.m_wheelDelta = _wheelDelta;
        return event;
    }
} // namespace

ego::engine::ApplicationWindowGuiViewportInputAdapter::~ApplicationWindowGuiViewportInputAdapter()
{
    release();
}

bool ego::engine::ApplicationWindowGuiViewportInputAdapter::init(
    const application::ApplicationWindowPointer& _window,
    const EventControllerPointer& _applicationEventController,
    const EventControllerPointer& _inputEventController)
{
    EGO_CHECK_INITIALIZATION(!m_window);
    EGO_CHECK_INITIALIZATION(!m_applicationEventController);
    EGO_CHECK_INITIALIZATION(!m_inputEventController);
    EGO_CHECK_INITIALIZATION(_window);
    EGO_CHECK_INITIALIZATION(_applicationEventController);
    EGO_CHECK_INITIALIZATION(_inputEventController);

    m_window = _window;
    m_applicationEventController = _applicationEventController;
    m_inputEventController = _inputEventController;

    return true;
}

bool ego::engine::ApplicationWindowGuiViewportInputAdapter::setGuiController(const SharedPointer<gui::GuiController>& _guiController)
{
    EGO_CHECK_RETURN_FALSE(_guiController);

    unregisterInputEventCallbacks();
    unregisterWindowInputEventCallbacks();
    m_guiController = _guiController;
    EGO_CHECK_RETURN_FALSE(registerWindowInputEventCallbacks());
    EGO_CHECK_RETURN_FALSE(registerInputEventCallbacks());

    return true;
}

void ego::engine::ApplicationWindowGuiViewportInputAdapter::release()
{
    unregisterInputEventCallbacks();
    unregisterWindowInputEventCallbacks();
    m_guiController.reset();
    m_inputEventController = nullptr;
    m_applicationEventController = nullptr;
    m_window = nullptr;
}

bool ego::engine::ApplicationWindowGuiViewportInputAdapter::adaptInputEvent(gui::GuiInputEvent& _event, const gui::GuiViewport& _viewport) const
{
    if (!_event.m_hasPosition)
    {
        return true;
    }

    EGO_CHECK_RETURN_FALSE(m_window);

    const WindowSize& clientSize = m_window->getClientAreaSize();
    EGO_CHECK_RETURN_FALSE(clientSize.m_x != 0 && clientSize.m_y != 0);

    WindowPoint screenPoint(static_cast<int32_t>(_event.m_position.m_x), static_cast<int32_t>(_event.m_position.m_y));
    WindowPoint clientPoint;
    EGO_CHECK_RETURN_FALSE(m_window->screenToClient(screenPoint, clientPoint));

    const gui::GuiSize& targetSize = _viewport.getSize();

    EGO_CHECK_RETURN_FALSE(targetSize.m_x != 0.0f && targetSize.m_y != 0.0f);

    _event.m_position = gui::GuiPosition(
        static_cast<float>(clientPoint.m_x) * targetSize.m_x / static_cast<float>(clientSize.m_x),
        static_cast<float>(clientPoint.m_y) * targetSize.m_y / static_cast<float>(clientSize.m_y));

    return true;
}

bool ego::engine::ApplicationWindowGuiViewportInputAdapter::registerWindowInputEventCallbacks()
{
    EGO_CHECK_RETURN_FALSE(m_window);
    EGO_CHECK_RETURN_FALSE(m_applicationEventController);

    m_keyboardInputEventCallbackID = m_applicationEventController->addInstanceEventCallback<application::ApplicationWindowKeyboardInputEvent>(
        m_window->getKeyboardInputEventID(),
        [this](const application::ApplicationWindowKeyboardInputEvent& _event)
        {
            handleWindowKeyboardInputEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_keyboardInputEventCallbackID != InvalidInstancedEventCallbackID);

    m_textInputEventCallbackID = m_applicationEventController->addInstanceEventCallback<application::ApplicationWindowTextInputEvent>(
        m_window->getTextInputEventID(),
        [this](const application::ApplicationWindowTextInputEvent& _event)
        {
            handleWindowTextInputEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_textInputEventCallbackID != InvalidInstancedEventCallbackID);

    return true;
}

void ego::engine::ApplicationWindowGuiViewportInputAdapter::unregisterWindowInputEventCallbacks()
{
    if (!m_applicationEventController)
    {
        m_textInputEventCallbackID = InvalidInstancedEventCallbackID;
        m_keyboardInputEventCallbackID = InvalidInstancedEventCallbackID;
        return;
    }

    if (m_textInputEventCallbackID != InvalidInstancedEventCallbackID)
    {
        m_applicationEventController->removeInstancedEventDispatcher(m_textInputEventCallbackID);
        m_textInputEventCallbackID = InvalidInstancedEventCallbackID;
    }

    if (m_keyboardInputEventCallbackID != InvalidInstancedEventCallbackID)
    {
        m_applicationEventController->removeInstancedEventDispatcher(m_keyboardInputEventCallbackID);
        m_keyboardInputEventCallbackID = InvalidInstancedEventCallbackID;
    }
}

bool ego::engine::ApplicationWindowGuiViewportInputAdapter::registerInputEventCallbacks()
{
    EGO_CHECK_RETURN_FALSE(m_inputEventController);

    m_inputDeviceChangedCallbackID = m_inputEventController->addEventCallback<InputDeviceChangedEvent>(
        [this](const InputDeviceChangedEvent& _event)
        {
            handleInputDeviceChangedEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_inputDeviceChangedCallbackID != InvalidEventCallbackID);

    m_inputKeyChangedCallbackID = m_inputEventController->addEventCallback<InputKeyChangedEvent>(
        [this](const InputKeyChangedEvent& _event)
        {
            handleInputKeyChangedEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_inputKeyChangedCallbackID != InvalidEventCallbackID);

    m_inputButtonPressedCallbackID = m_inputEventController->addEventCallback<InputButtonPressedEvent>(
        [this](const InputButtonPressedEvent& _event)
        {
            handleInputButtonPressedEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_inputButtonPressedCallbackID != InvalidEventCallbackID);

    m_inputButtonReleasedCallbackID = m_inputEventController->addEventCallback<InputButtonReleasedEvent>(
        [this](const InputButtonReleasedEvent& _event)
        {
            handleInputButtonReleasedEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_inputButtonReleasedCallbackID != InvalidEventCallbackID);

    return true;
}

void ego::engine::ApplicationWindowGuiViewportInputAdapter::unregisterInputEventCallbacks()
{
    if (!m_inputEventController)
    {
        m_inputButtonReleasedCallbackID = InvalidEventCallbackID;
        m_inputButtonPressedCallbackID = InvalidEventCallbackID;
        m_inputKeyChangedCallbackID = InvalidEventCallbackID;
        m_inputDeviceChangedCallbackID = InvalidEventCallbackID;
        return;
    }

    if (m_inputButtonReleasedCallbackID != InvalidEventCallbackID)
    {
        m_inputEventController->removeEventCallback(m_inputButtonReleasedCallbackID);
        m_inputButtonReleasedCallbackID = InvalidEventCallbackID;
    }

    if (m_inputButtonPressedCallbackID != InvalidEventCallbackID)
    {
        m_inputEventController->removeEventCallback(m_inputButtonPressedCallbackID);
        m_inputButtonPressedCallbackID = InvalidEventCallbackID;
    }

    if (m_inputKeyChangedCallbackID != InvalidEventCallbackID)
    {
        m_inputEventController->removeEventCallback(m_inputKeyChangedCallbackID);
        m_inputKeyChangedCallbackID = InvalidEventCallbackID;
    }

    if (m_inputDeviceChangedCallbackID != InvalidEventCallbackID)
    {
        m_inputEventController->removeEventCallback(m_inputDeviceChangedCallbackID);
        m_inputDeviceChangedCallbackID = InvalidEventCallbackID;
    }
}

void ego::engine::ApplicationWindowGuiViewportInputAdapter::handleWindowKeyboardInputEvent(const application::ApplicationWindowKeyboardInputEvent& _event) const
{
    const SharedPointer<gui::GuiController> guiController = m_guiController.lock();
    if (!guiController || !guiController->isInitialized())
    {
        return;
    }

    guiController->processEvent(CreateKeyboardInputEvent(_event.m_inputData));
}

void ego::engine::ApplicationWindowGuiViewportInputAdapter::handleWindowTextInputEvent(const application::ApplicationWindowTextInputEvent& _event) const
{
    const SharedPointer<gui::GuiController> guiController = m_guiController.lock();
    if (!guiController || !guiController->isInitialized() || _event.m_inputData.m_codepoint == 0)
    {
        return;
    }

    guiController->processEvent(CreateTextInputEvent(_event.m_inputData));
}

void ego::engine::ApplicationWindowGuiViewportInputAdapter::handleInputDeviceChangedEvent(const InputDeviceChangedEvent& _event) const
{
    const SharedPointer<gui::GuiController> guiController = m_guiController.lock();
    if (!guiController || !guiController->isInitialized() || !IsMouseDevice(_event.m_device) || !IsMousePositionChanged(_event.m_device))
    {
        return;
    }

    guiController->processEvent(CreateMouseEvent(gui::GuiInputEventType::MouseMove, _event.m_device));
}

void ego::engine::ApplicationWindowGuiViewportInputAdapter::handleInputKeyChangedEvent(const InputKeyChangedEvent& _event) const
{
    const SharedPointer<gui::GuiController> guiController = m_guiController.lock();
    if (!guiController || !guiController->isInitialized() || !IsMouseDevice(_event.m_device) || !IsMouseWheelKey(_event.m_key))
    {
        return;
    }

    guiController->processEvent(CreateMouseWheelEvent(_event.m_device, _event.m_value - _event.m_previousValue));
}

void ego::engine::ApplicationWindowGuiViewportInputAdapter::handleInputButtonPressedEvent(const InputButtonPressedEvent& _event) const
{
    const SharedPointer<gui::GuiController> guiController = m_guiController.lock();
    if (!guiController || !guiController->isInitialized() || !IsMouseDevice(_event.m_device))
    {
        return;
    }

    gui::GuiMouseButton mouseButton;
    if (TryGetGuiMouseButton(_event.m_key, mouseButton))
    {
        guiController->processEvent(CreateMouseButtonEvent(gui::GuiInputEventType::MouseButtonDown, _event.m_device, mouseButton));
    }
}

void ego::engine::ApplicationWindowGuiViewportInputAdapter::handleInputButtonReleasedEvent(const InputButtonReleasedEvent& _event) const
{
    const SharedPointer<gui::GuiController> guiController = m_guiController.lock();
    if (!guiController || !guiController->isInitialized() || !IsMouseDevice(_event.m_device))
    {
        return;
    }

    gui::GuiMouseButton mouseButton;
    if (TryGetGuiMouseButton(_event.m_key, mouseButton))
    {
        guiController->processEvent(CreateMouseButtonEvent(gui::GuiInputEventType::MouseButtonUp, _event.m_device, mouseButton));
    }
}
