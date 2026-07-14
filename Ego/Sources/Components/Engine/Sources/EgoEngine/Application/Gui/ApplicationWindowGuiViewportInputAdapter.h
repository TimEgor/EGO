#pragma once

#include "EgoCore/Reference/Pointer.h"

#include "EgoEvent/EventController.h"

#include "EgoGui/GuiViewportInputAdapter.h"

#include "EgoApplication/Window/ApplicationWindow.h"

namespace ego::application
{
    struct ApplicationWindowKeyboardInputEvent;
    struct ApplicationWindowTextInputEvent;

} // namespace ego::application

namespace ego
{
    struct InputButtonPressedEvent;
    struct InputButtonReleasedEvent;
    struct InputDeviceChangedEvent;
    struct InputKeyChangedEvent;
} // namespace ego

namespace ego::gui
{
    class GuiController;
} // namespace ego::gui

namespace ego::engine
{
    class ApplicationWindowGuiViewportInputAdapter final : public gui::GuiViewportInputAdapter
    {
    public:
        ApplicationWindowGuiViewportInputAdapter() = default;
        ~ApplicationWindowGuiViewportInputAdapter() override;

        bool init(
            const application::ApplicationWindowPointer& _window,
            const EventControllerPointer& _applicationEventController,
            const EventControllerPointer& _inputEventController);
        bool setGuiController(const SharedPointer<gui::GuiController>& _guiController);
        void release();

        bool adaptInputEvent(gui::GuiInputEvent& _event, const gui::GuiViewport& _viewport) const override;

        EGO_RTTI_VIRTUAL(ApplicationWindowGuiViewportInputAdapter, gui::GuiViewportInputAdapter);

    private:
        bool registerWindowInputEventCallbacks();
        void unregisterWindowInputEventCallbacks();
        bool registerInputEventCallbacks();
        void unregisterInputEventCallbacks();

        void handleWindowKeyboardInputEvent(const application::ApplicationWindowKeyboardInputEvent& _event) const;
        void handleWindowTextInputEvent(const application::ApplicationWindowTextInputEvent& _event) const;
        void handleInputDeviceChangedEvent(const InputDeviceChangedEvent& _event) const;
        void handleInputKeyChangedEvent(const InputKeyChangedEvent& _event) const;
        void handleInputButtonPressedEvent(const InputButtonPressedEvent& _event) const;
        void handleInputButtonReleasedEvent(const InputButtonReleasedEvent& _event) const;

        application::ApplicationWindowPointer m_window = nullptr;
        EventControllerPointer m_applicationEventController = nullptr;
        EventControllerPointer m_inputEventController = nullptr;
        WeakPointer<gui::GuiController> m_guiController;
        InstancedEventCallbackID m_keyboardInputEventCallbackID = InvalidInstancedEventCallbackID;
        InstancedEventCallbackID m_textInputEventCallbackID = InvalidInstancedEventCallbackID;
        EventCallbackID m_inputDeviceChangedCallbackID = InvalidEventCallbackID;
        EventCallbackID m_inputKeyChangedCallbackID = InvalidEventCallbackID;
        EventCallbackID m_inputButtonPressedCallbackID = InvalidEventCallbackID;
        EventCallbackID m_inputButtonReleasedCallbackID = InvalidEventCallbackID;
    };

    EGO_POINTER(ApplicationWindowGuiViewportInputAdapter);
} // namespace ego::engine
