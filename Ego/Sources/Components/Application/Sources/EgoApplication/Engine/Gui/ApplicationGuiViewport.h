#pragma once

#include <cstdint>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Input/InputTypes.h"

#include "EgoEvent/EventController.h"

#include "EgoGui/Viewport/ViewportProvider.h"

#include "EgoApplication/Presentation/PresenterProvider.h"

namespace ego
{
    struct InputDeviceChangedEvent;
    struct InputKeyChangedEvent;
    struct InputKeyEvent;
} // namespace ego

namespace ego::application
{
    struct PresentationSurfaceActivationEvent;
    struct PresentationSurfaceDestroyingEvent;
    struct PresentationSurfaceKeyboardInputEvent;
    struct PresentationSurfaceTextInputEvent;

    class ApplicationGuiViewport final : public NonCopyable
    {
    public:
        ApplicationGuiViewport() = default;
        ~ApplicationGuiViewport() override;

        bool init(const Presentation& _presentation);
        void release();

        gui::ViewportUpdate poll(const GraphicPresenterPointer& _graphicPresenter);
        const PresentationSurfacePointer& getSurfacePointer() const;

    private:
        struct CallbackIDs final
        {
            InstancedEventCallbackID m_surfaceDestroying = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_surfaceActivation = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_surfaceKeyboardInput = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_surfaceTextInput = InvalidInstancedEventCallbackID;
            EventCallbackID m_mouseChanged = InvalidEventCallbackID;
            EventCallbackID m_mouseWheel = InvalidEventCallbackID;
            EventCallbackID m_mouseButtonPressed = InvalidEventCallbackID;
            EventCallbackID m_mouseButtonReleased = InvalidEventCallbackID;
        };

        bool registerEventCallbacks(const PresentationSurfaceEventIDs& _eventIDs);
        void unregisterEventCallbacks();

        void handleSurfaceDestroyingEvent(const PresentationSurfaceDestroyingEvent& _event);
        void handleSurfaceActivationEvent(const PresentationSurfaceActivationEvent& _event);
        void handleSurfaceKeyboardInputEvent(const PresentationSurfaceKeyboardInputEvent& _event);
        void handleSurfaceTextInputEvent(const PresentationSurfaceTextInputEvent& _event);
        void handleMouseChangedEvent(const InputDeviceChangedEvent& _event);
        void handleMouseWheelEvent(const InputKeyChangedEvent& _event);
        void handleMouseButtonPressedEvent(const InputKeyEvent& _event);
        void handleMouseButtonReleasedEvent(const InputKeyEvent& _event);
        void handleMouseButtonEvent(const InputKeyEvent& _event, InputButtonAction _action);

        void updateSize();
        void resetInput();
        void updateModifiers(const PresentationSurfaceKeyboardInputEvent& _event);
        bool enqueuePointerInput(gui::PointerMoveEvent _event);
        bool enqueuePointerInput(gui::MouseButtonEvent _event);
        bool enqueuePointerInput(gui::MouseWheelEvent _event);
        bool preparePointerInput(gui::Position& _position, bool& _emitPointerExit);
        bool convertPointerPosition(gui::Position& _position, bool& _isInsideSurface) const;

        static EventControllerPointer GetEventControllerPointer();

        PresentationSurfacePointer m_surface = nullptr;
        gui::ViewportUpdateStatus m_status = gui::ViewportUpdateStatus::CloseRequested;
        gui::Size m_size = gui::SizeZero;
        gui::InputEventCollection m_input;
        gui::InputModifiers m_modifiers;
        uint8_t m_pressedKeyboardModifiers = 0;
        uint8_t m_pressedMouseButtons = 0;
        bool m_isSurfaceActive = false;
        bool m_isPointerInsideSurface = false;
        CallbackIDs m_callbackIDs;
    };
} // namespace ego::application
