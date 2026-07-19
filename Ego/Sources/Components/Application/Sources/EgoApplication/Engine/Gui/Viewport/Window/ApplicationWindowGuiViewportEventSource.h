#pragma once

#include <cstdint>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Input/InputTypes.h"

#include "EgoEvent/EventController.h"

#include "EgoGui/Viewport/ViewportBackend.h"

#include "EgoApplication/Window/ApplicationWindow.h"

namespace ego
{
    struct InputDeviceChangedEvent;
    struct InputKeyChangedEvent;
    struct InputKeyEvent;
} // namespace ego

namespace ego::application
{
    struct ApplicationWindowActivationEvent;
    struct ApplicationWindowKeyboardInputEvent;
    struct ApplicationWindowTextInputEvent;

    class ApplicationWindowGuiViewportEventSource final : public NonCopyable
    {
    public:
        ApplicationWindowGuiViewportEventSource() = default;
        ~ApplicationWindowGuiViewportEventSource() override;

        bool init(const ApplicationWindowPointer& _window);
        void release();

        void drainInput(gui::InputEventCollection& _input);

    private:
        struct CallbackIDs final
        {
            InstancedEventCallbackID m_keyboardInput = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_textInput = InvalidInstancedEventCallbackID;
            EventCallbackID m_windowActivation = InvalidEventCallbackID;
            EventCallbackID m_mouseChanged = InvalidEventCallbackID;
            EventCallbackID m_mouseWheel = InvalidEventCallbackID;
            EventCallbackID m_mouseButtonPressed = InvalidEventCallbackID;
            EventCallbackID m_mouseButtonReleased = InvalidEventCallbackID;
        };

        bool registerEventCallbacks();
        void unregisterEventCallbacks();

        void handleWindowActivationEvent(const ApplicationWindowActivationEvent& _event);
        void handleWindowKeyboardInputEvent(const ApplicationWindowKeyboardInputEvent& _event);
        void handleWindowTextInputEvent(const ApplicationWindowTextInputEvent& _event);
        void handleMouseChangedEvent(const InputDeviceChangedEvent& _event);
        void handleMouseWheelEvent(const InputKeyChangedEvent& _event);
        void handleMouseButtonEvent(const InputKeyEvent& _event, InputButtonAction _action);

        void updateModifiers(const WindowKeyboardInputData& _inputData);
        bool enqueuePointerInput(gui::PointerMoveEvent _event);
        bool enqueuePointerInput(gui::MouseButtonEvent _event);
        bool enqueuePointerInput(gui::MouseWheelEvent _event);
        bool preparePointerInput(gui::Position& _position, bool& _emitPointerExit);
        bool convertPointerPosition(gui::Position& _position, bool& _isInsideWindow) const;

        ApplicationWindowPointer m_window = nullptr;
        EventControllerPointer m_eventController = nullptr;
        CallbackIDs m_callbackIDs;

        gui::InputEventCollection m_input;
        gui::InputModifiers m_modifiers;
        uint8_t m_pressedKeyboardModifiers = 0;
        uint8_t m_pressedMouseButtons = 0;
        bool m_isWindowActive = false;
        bool m_isPointerInsideWindow = false;
    };

    EGO_POINTER(ApplicationWindowGuiViewportEventSource);
} // namespace ego::application
