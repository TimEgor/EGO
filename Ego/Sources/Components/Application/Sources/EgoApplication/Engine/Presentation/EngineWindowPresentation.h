#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoEvent/EventController.h"

namespace ego
{
    class GraphicPresenter;

    EGO_POINTER(GraphicPresenter);
} // namespace ego

namespace ego::application
{
    class ApplicationWindow;
    class WindowGraphicPresenter;
    struct ApplicationWindowSizeChangedEvent;

    EGO_POINTER(ApplicationWindow);
    EGO_POINTER(WindowGraphicPresenter);
} // namespace ego::application

namespace ego::application
{
    class EngineWindowPresentation final : public NonCopyable
    {
    public:
        EngineWindowPresentation() = default;
        ~EngineWindowPresentation() override;

        bool init(const ApplicationWindowPointer& _window);
        void release();

        bool update();
        GraphicPresenterPointer getGraphicPresenterPointer() const;

    private:
        bool registerWindowEvents();
        void unregisterWindowEvents();
        void handleWindowSizeChanged(const ApplicationWindowSizeChangedEvent& _event);

        ApplicationWindowPointer m_window = nullptr;
        WindowGraphicPresenterPointer m_graphicPresenter = nullptr;
        InstancedEventCallbackID m_sizeChangedCallbackID = InvalidInstancedEventCallbackID;
        bool m_isResizePending = false;
    };

    EGO_POINTER(EngineWindowPresentation);
} // namespace ego::application
