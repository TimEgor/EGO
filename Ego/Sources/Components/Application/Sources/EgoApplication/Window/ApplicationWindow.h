#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Window/Window.h"
#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoEvent/EventController.h"

#include "EgoGraphicHardware/PresentationSurface.h"

namespace ego::application
{
    class ApplicationWindowPresentationProvider;

    class ApplicationWindow final : public PresentationSurface, public NonCopyable
    {
        friend class ApplicationWindowPresentationProvider;

    public:
        ApplicationWindow() = default;
        ~ApplicationWindow() override;

        void release();

        bool isValid() const;

        void show() override;
        void hide() override;
        bool isShown() const override;

        void* getNativeHandle() const override;
        bool isStable() const;
        bool screenToLocal(const PresentationSurfacePoint& _screenPoint, PresentationSurfacePoint& _localPoint) const override;

        const WindowSize& getWindowSize() const;
        const WindowSize& getSize() const override;
        const WindowArea& getCutoutsArea() const;

        InstancedEventID getDestroyingEventID() const;
        InstancedEventID getActivationEventID() const;
        InstancedEventID getSizeChangedEventID() const;
        InstancedEventID getKeyboardInputEventID() const;
        InstancedEventID getTextInputEventID() const;

        EGO_RTTI_VIRTUAL(::ego::application::ApplicationWindow, PresentationSurface);

    private:
        bool init(const WindowPointer& _nativeWindow, const EventControllerPointer& _eventController);
        void detachNativeWindow();
        WindowPointer getNativeWindowPointer() const;

        bool initInstancedEvents();
        void releaseInstancedEvents();

        WindowPointer m_nativeWindow = nullptr;
        EventControllerPointer m_eventController = nullptr;
        InstancedEventID m_destroyingEventID = InvalidInstancedEventID;
        InstancedEventID m_activationEventID = InvalidInstancedEventID;
        InstancedEventID m_sizeChangedEventID = InvalidInstancedEventID;
        InstancedEventID m_keyboardInputEventID = InvalidInstancedEventID;
        InstancedEventID m_textInputEventID = InvalidInstancedEventID;
    };

    EGO_POINTER(ApplicationWindow);
    EGO_WEAK_POINTER(ApplicationWindow);
} // namespace ego::application
