#pragma once

#include <unordered_map>
#include <vector>

#include "EgoCore/Event/EventController.h"
#include "EgoCore/Math/Vector.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGui/Viewport/ViewportProvider.h"

#include "EgoRuntime/Presentation/PresenterProvider.h"

namespace ego
{
    struct InputDeviceChangedEvent;
    struct InputKeyChangedEvent;
    struct InputKeyEvent;
} // namespace ego

namespace ego::runtime
{
    class RuntimeGuiViewport;

    class RuntimeGuiViewportProvider final : public gui::ViewportProvider, public NonCopyable
    {
    public:
        RuntimeGuiViewportProvider() = default;
        ~RuntimeGuiViewportProvider() override;

        bool init(const Presentation& _primaryPresentation);

        bool createViewport(const gui::ViewportCreateRequest& _request) override;
        void destroyViewport(gui::ViewportID _viewportID) override;
        gui::ViewportState getViewportState(gui::ViewportID _viewportID) const override;
        gui::ViewportUpdate pollViewport(gui::ViewportID _viewportID) override;
        bool showViewport(gui::ViewportID _viewportID, bool _activate) override;
        bool setViewportPosition(gui::ViewportID _viewportID, FloatVector2& _position) override;
        bool setViewportSize(gui::ViewportID _viewportID, FloatVector2& _size) override;
        bool setViewportInputTransparent(gui::ViewportID _viewportID, bool _isTransparent) override;

    private:
        void release();

        struct CallbackIDs final
        {
            EventCallbackID m_mouseChanged = InvalidEventCallbackID;
            EventCallbackID m_mouseWheel = InvalidEventCallbackID;
            EventCallbackID m_mouseButtonPressed = InvalidEventCallbackID;
            EventCallbackID m_mouseButtonReleased = InvalidEventCallbackID;
        };

        using ViewportPointer = SharedPointer<RuntimeGuiViewport>;
        using ViewportMap = std::unordered_map<gui::ViewportID, ViewportPointer>;
        using ModalViewportStack = std::vector<gui::ViewportID>;
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
        void removeModalViewport(gui::ViewportID _viewportID);
        bool updateViewportInputState();
        ViewportPointer findViewport(gui::ViewportID _viewportID) const;
        gui::ViewportID findViewportAtScreenPosition(const FloatVector2& _position) const;
        gui::ViewportID findPointerInputViewport(const FloatVector2& _position) const;
        bool capturePointer(const ViewportPointer& _viewport);
        void releasePointer(const ViewportPointer& _viewport);
        void setFocusedViewport(gui::ViewportID _viewportID);
        void updatePointerViewport(gui::ViewportID _viewportID, const FloatVector2& _position);

        static EventControllerPointer GetEventControllerPointer();
        static PresenterProviderPointer GetPresenterProvider();
        PresentationDesc createViewportPresentationDesc(const gui::ViewportCreateRequest& _request) const;

        Presentation m_primaryPresentation;
        ViewportMap m_viewports;
        ModalViewportStack m_modalViewportStack;
        SurfaceCollection m_retiringSurfaces;
        SurfaceCollection m_releasableSurfaces;
        gui::ViewportID m_primaryViewportID = gui::InvalidViewportID;
        gui::ViewportID m_pointerViewportID = gui::InvalidViewportID;
        CallbackIDs m_callbackIDs;
    };

    EGO_POINTER(RuntimeGuiViewportProvider);
} // namespace ego::runtime
