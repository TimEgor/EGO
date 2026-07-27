#pragma once

#include <cstdint>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Input/InputTypes.h"

#include "EgoGui/Viewport/ViewportProvider.h"

#include "EgoApplication/Presentation/PresenterProvider.h"

namespace ego::application
{
    class ApplicationGuiViewport final : public NonCopyable
    {
    public:
        ApplicationGuiViewport() = default;
        ~ApplicationGuiViewport() override;

        bool init(const Presentation& _presentation);
        void release();

        gui::ViewportUpdate poll();
        bool show(bool _activate);
        bool setPosition(gui::Position& _position);
        bool setSize(gui::Size& _size);
        bool setInputPassthrough(bool _isEnabled);

        const Presentation& getPresentation() const;

        bool enqueuePointerExit(const gui::Position& _screenPosition);
        bool enqueueMouseButtonInput(gui::MouseButtonEvent _event);
        bool enqueuePointerInput(gui::PointerMoveEvent _event);
        bool enqueuePointerInput(gui::MouseWheelEvent _event);
        bool hasPressedMouseButtons() const;

    private:
        struct SurfaceEventCallbackIDs final
        {
            InstancedEventCallbackID m_closeRequested = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_destroying = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_activation = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_pointerCaptureLost = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_keyboardInput = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_textInput = InvalidInstancedEventCallbackID;
        };

        bool registerSurfaceEvents();
        void unregisterSurfaceEvents();

        void handleSurfaceCloseRequested(const PlatformSurfaceCloseRequestedEvent& _event);
        void handleSurfaceDestroying(const PlatformSurfaceDestroyingEvent& _event);
        void handleSurfaceActivation(const PlatformSurfaceActivationEvent& _event);
        void handleSurfacePointerCaptureLost(const PlatformSurfacePointerCaptureLostEvent& _event);
        void handleSurfaceKeyboardInput(const PlatformSurfaceKeyboardInputEvent& _event);
        void handleSurfaceTextInput(const PlatformSurfaceTextInputEvent& _event);

        void updateBounds();
        void resetInput();
        void updateModifiers(const SurfaceKeyboardInput& _input);
        bool enqueuePointerInput(gui::MouseButtonEvent _event);
        bool preparePointerInput(gui::Position& _position, bool& _emitPointerExit);
        bool convertPointerPosition(gui::Position& _position, bool& _isInsideSurface) const;

        static EventControllerPointer GetEventControllerPointer();

        Presentation m_presentation;
        gui::ViewportUpdateStatus m_status = gui::ViewportUpdateStatus::CloseRequested;
        gui::Position m_position = gui::PositionZero;
        gui::Size m_size = gui::SizeZero;
        gui::Position m_requestedPosition = gui::PositionZero;
        gui::Size m_requestedSize = gui::SizeZero;
        gui::InputEventCollection m_input;
        gui::InputModifiers m_modifiers;
        SurfaceEventCallbackIDs m_surfaceEventCallbackIDs;
        uint8_t m_pressedKeyboardModifiers = 0;
        uint8_t m_pressedMouseButtons = 0;
        bool m_isPointerInsideSurface = false;
        bool m_hasPositionRequest = false;
        bool m_hasSizeRequest = false;
    };
} // namespace ego::application
