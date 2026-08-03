#pragma once

#include <cstdint>

#include "EgoCore/Math/Vector.h"
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

        gui::ViewportState getState() const;
        gui::ViewportUpdate poll();
        bool show(bool _activate);
        bool setPosition(FloatVector2& _position);
        bool setSize(FloatVector2& _size);
        bool setInputEnabled(bool _isEnabled);
        bool setInputTransparent(bool _isTransparent);
        void setFocused(bool _isFocused);

        const Presentation& getPresentation() const;

        bool enqueuePointerExit(const FloatVector2& _screenPosition);
        bool enqueueMouseButtonInput(gui::MouseButtonEvent _event);
        bool enqueuePointerInput(gui::PointerMoveEvent _event);
        bool enqueuePointerInput(gui::MouseWheelEvent _event);
        bool hasPressedMouseButtons() const;

    private:
        void release();

        struct SurfaceEventCallbackIDs final
        {
            InstancedEventCallbackID m_closeRequested = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_activation = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_pointerCaptureLost = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_keyboardInput = InvalidInstancedEventCallbackID;
            InstancedEventCallbackID m_textInput = InvalidInstancedEventCallbackID;
        };

        bool registerSurfaceEvents();
        void unregisterSurfaceEvents();

        void handleSurfaceCloseRequested(const PlatformSurfaceCloseRequestedEvent& _event);
        void handleSurfaceActivation(const PlatformSurfaceActivationEvent& _event);
        void handleSurfacePointerCaptureLost(const PlatformSurfacePointerCaptureLostEvent& _event);
        void handleSurfaceKeyboardInput(const PlatformSurfaceKeyboardInputEvent& _event);
        void handleSurfaceTextInput(const PlatformSurfaceTextInputEvent& _event);

        void updateBounds();
        void resetInput();
        void updateModifiers(const SurfaceKeyboardInput& _input);
        bool enqueuePointerInput(gui::MouseButtonEvent _event);
        bool preparePointerInput(FloatVector2& _position, bool& _emitPointerExit);
        bool convertPointerPosition(FloatVector2& _position, bool& _isInsideSurface) const;

        static bool AreEqual(const FloatVector2& _first, const FloatVector2& _second);
        static EventControllerPointer GetEventControllerPointer();

        Presentation m_presentation;
        gui::ViewportUpdateStatus m_status = gui::ViewportUpdateStatus::CloseRequested;
        FloatVector2 m_position = FloatVector2Zero;
        FloatVector2 m_size = FloatVector2Zero;
        FloatVector2 m_requestedPosition = FloatVector2Zero;
        FloatVector2 m_requestedSize = FloatVector2Zero;
        gui::InputEventCollection m_input;
        gui::InputModifiers m_modifiers;
        SurfaceEventCallbackIDs m_surfaceEventCallbackIDs;
        uint8_t m_pressedKeyboardModifiers = 0;
        uint8_t m_pressedMouseButtons = 0;
        bool m_isPointerInsideSurface = false;
        bool m_hasPositionRequest = false;
        bool m_hasSizeRequest = false;
        bool m_isFocused = false;
        bool m_isInputTransparent = false;
    };
} // namespace ego::application
