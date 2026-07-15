#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoEvent/EventController.h"

namespace ego
{
    class InputController;

    EGO_POINTER(InputController);
} // namespace ego

namespace ego::application
{
    class ApplicationWindow;

    EGO_POINTER(ApplicationWindow);
} // namespace ego::application

namespace ego::gui
{
    class GuiViewport;

    EGO_POINTER(GuiViewport);
} // namespace ego::gui

namespace ego::engine
{
    class EngineSession;

    EGO_POINTER(EngineSession);
} // namespace ego::engine

namespace ego::application
{
    class ApplicationWindowGuiViewportInputAdapter;
    class ApplicationWindowInputKeyProvider;

    EGO_POINTER(ApplicationWindowGuiViewportInputAdapter);
    EGO_POINTER(ApplicationWindowInputKeyProvider);

    class EngineWindowInputBinding final : public NonCopyable
    {
    public:
        EngineWindowInputBinding() = default;
        ~EngineWindowInputBinding() override;

        bool init(const engine::EngineSessionPointer& _engineSession, const EventControllerPointer& _eventController, const ApplicationWindowPointer& _window);
        void release();

        void updateInputDevices();

    private:
        InputControllerPointer m_inputController = nullptr;
        gui::GuiViewportPointer m_guiViewport = nullptr;
        ApplicationWindowGuiViewportInputAdapterPointer m_guiViewportInputAdapter = nullptr;
        ApplicationWindowInputKeyProviderPointer m_inputKeyProvider = nullptr;
    };

    EGO_POINTER(EngineWindowInputBinding);
} // namespace ego::application
