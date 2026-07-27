#pragma once

#include <unordered_map>
#include <vector>

#include "EgoCore/Event/EventController.h"
#include "EgoCore/Patterns/NonCopyable.h"

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
    class ApplicationGuiViewport;

    class ApplicationGuiViewportProvider final : public gui::ViewportProvider, public NonCopyable
    {
    public:
        ApplicationGuiViewportProvider() = default;
        ~ApplicationGuiViewportProvider() override;

        bool init(const Presentation& _primaryPresentation);
        void release();

        bool createViewport(const gui::ViewportCreateRequest& _request) override;
        void destroyViewport(gui::ViewportID _viewportID) override;
        gui::ViewportUpdate pollViewport(gui::ViewportID _viewportID) override;
        bool showViewport(gui::ViewportID _viewportID, bool _activate) override;
        bool setViewportPosition(gui::ViewportID _viewportID, gui::Position& _position) override;
        bool setViewportSize(gui::ViewportID _viewportID, gui::Size& _size) override;
        bool setViewportInputPassthrough(gui::ViewportID _viewportID, bool _isEnabled) override;
        gui::ViewportID findViewportAtScreenPosition(const gui::Position& _position) const override;

    private:
        struct CallbackIDs final
        {
            EventCallbackID m_mouseChanged = InvalidEventCallbackID;
            EventCallbackID m_mouseWheel = InvalidEventCallbackID;
            EventCallbackID m_mouseButtonPressed = InvalidEventCallbackID;
            EventCallbackID m_mouseButtonReleased = InvalidEventCallbackID;
        };

        using ViewportPointer = SharedPointer<ApplicationGuiViewport>;
        using ViewportMap = std::unordered_map<gui::ViewportID, ViewportPointer>;
        using SurfaceCollection = std::vector<PlatformSurfacePointer>;

        bool registerInputEvents();
        void unregisterInputEvents();
        void handleMouseChangedEvent(const InputDeviceChangedEvent& _event);
        void handleMouseWheelEvent(const InputKeyChangedEvent& _event);
        void handleMouseButtonPressedEvent(const InputKeyEvent& _event);
        void handleMouseButtonReleasedEvent(const InputKeyEvent& _event);
        void handleMouseButtonEvent(const InputKeyEvent& _event, InputButtonAction _action);

        void retireViewport(ViewportPointer& _viewport);
        void advanceViewportRetirement();
        void releaseSurfaces(SurfaceCollection& _surfaces);
        void releaseViewport(ViewportPointer& _viewport);
        ViewportPointer findViewport(gui::ViewportID _viewportID) const;
        gui::ViewportID findPointerInputViewport(const gui::Position& _position) const;
        bool setPointerCapture(const ViewportPointer& _viewport);
        void clearPointerCapture(const ViewportPointer& _viewport);
        void updatePointerViewport(gui::ViewportID _viewportID, const gui::Position& _position);

        static EventControllerPointer GetEventControllerPointer();
        static PresenterProviderPointer GetPresenterProvider();
        static PresentationDesc CreateViewportPresentationDesc(const gui::ViewportCreateRequest& _request);

        Presentation m_primaryPresentation;
        ViewportMap m_viewports;
        SurfaceCollection m_retiringSurfaces;
        SurfaceCollection m_releasableSurfaces;
        gui::ViewportID m_primaryViewportID = gui::InvalidViewportID;
        gui::ViewportID m_pointerViewportID = gui::InvalidViewportID;
        CallbackIDs m_callbackIDs;
    };

    EGO_POINTER(ApplicationGuiViewportProvider);
} // namespace ego::application
