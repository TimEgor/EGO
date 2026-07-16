#pragma once

#include <cstdint>

#include "EgoEvent/EventController.h"

#include "EgoGui/Viewport/GuiViewportBackend.h"

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

    class ApplicationWindowGuiViewportEventSource final
    {
    public:
        ApplicationWindowGuiViewportEventSource() = default;
        ~ApplicationWindowGuiViewportEventSource();

        bool init(const ApplicationWindowPointer& _window);
        void release();

        void drainEvents(gui::GuiViewportEventCollection& _events);

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
        void handleMouseButtonEvent(const InputKeyEvent& _event, gui::GuiInputEventType _type);

        bool enqueuePointerEvent(gui::GuiInputEvent _event);
        bool convertPointerPosition(gui::GuiInputEvent& _event, bool& _isInsideWindow) const;

        ApplicationWindowPointer m_window = nullptr;
        EventControllerPointer m_eventController = nullptr;
        CallbackIDs m_callbackIDs;

        gui::GuiViewportEventCollection m_events;
        uint32_t m_pressedMouseButtonCount = 0;
        bool m_isWindowActive = false;
        bool m_isPointerInsideWindow = false;
    };

    EGO_POINTER(ApplicationWindowGuiViewportEventSource);
} // namespace ego::application
