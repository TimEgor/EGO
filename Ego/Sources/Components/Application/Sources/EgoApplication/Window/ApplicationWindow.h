#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Window/PresentationSurface.h"
#include "EgoCore/Platform/Window/WindowTypes.h"
#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoEvent/EventController.h"

namespace ego
{
    class Window;
    EGO_POINTER(Window);
} // namespace ego

namespace ego::application
{
    class ApplicationWindowController;

    class ApplicationWindow final
        : public PresentationSurface
        , public NonCopyable
    {
        friend class ApplicationWindowController;

    public:
        ApplicationWindow() = default;
        ~ApplicationWindow() override;

        void release();

        bool isValid() const;

        void show();
        void hide();
        bool isShown() const;

        void* getNativeHandle() const override;
        bool isStable() const;
        bool screenToClient(const WindowPoint& _screenPoint, WindowPoint& _clientPoint) const;

        const WindowSize& getWindowSize() const;
        const WindowSize& getClientAreaSize() const override;
        const WindowArea& getCutoutsArea() const;

        InstancedEventID getSizeEventID() const;
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
        InstancedEventID m_sizeEventID = InvalidInstancedEventID;
        InstancedEventID m_keyboardInputEventID = InvalidInstancedEventID;
        InstancedEventID m_textInputEventID = InvalidInstancedEventID;
    };

    EGO_POINTER(ApplicationWindow);
    EGO_WEAK_POINTER(ApplicationWindow);
} // namespace ego::application
