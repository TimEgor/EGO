#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoEvent/EventController.h"

namespace ego
{
    class WindowGraphicPresenter;

    EGO_POINTER(WindowGraphicPresenter);
} // namespace ego

namespace ego::gpu
{
    class GraphicHardwareSubsystem;

    EGO_POINTER(GraphicHardwareSubsystem);
} // namespace ego::gpu

namespace ego::application
{
    class ApplicationWindow;
    struct ApplicationWindowSizeChangedEvent;

    EGO_POINTER(ApplicationWindow);
} // namespace ego::application

namespace ego::engine
{
    class EngineSession;

    EGO_POINTER(EngineSession);

    class EngineWindowPresentation final : public NonCopyable
    {
    public:
        EngineWindowPresentation() = default;
        ~EngineWindowPresentation() override;

        bool init(
            const EngineSessionPointer& _engineSession,
            const gpu::GraphicHardwareSubsystemPointer& _graphicHardwareSubsystem,
            const EventControllerPointer& _eventController,
            const application::ApplicationWindowPointer& _window,
            bool _makePrimary);
        void release();

        bool update();

    private:
        bool registerWindowEvents();
        void unregisterWindowEvents();
        void handleWindowSizeChanged(const application::ApplicationWindowSizeChangedEvent& _event);

        EngineSessionPointer m_engineSession = nullptr;
        application::ApplicationWindowPointer m_window = nullptr;
        WindowGraphicPresenterPointer m_graphicPresenter = nullptr;
        EventControllerPointer m_eventController = nullptr;
        InstancedEventCallbackID m_sizeChangedCallbackID = InvalidInstancedEventCallbackID;
        bool m_isResizePending = false;
    };

    EGO_POINTER(EngineWindowPresentation);
} // namespace ego::engine
