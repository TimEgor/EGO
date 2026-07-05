#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoCore/Platform/Window/WindowTypes.h"

#include "EgoRuntime/Event/EventController.h"

#include "EgoGraphicHardware/PresentationSurface.h"

namespace ego
{
    class Window;
    EGO_POINTER(Window);
} // namespace ego

namespace ego::application
{
    class ApplicationWindowManager;

    class ApplicationWindow final : public PresentationSurface
    {
        friend class ApplicationWindowManager;

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

        EGO_RTTI_VIRTUAL(::ego::application::ApplicationWindow, PresentationSurface);

    private:
        bool init(const ego::WindowPointer& _nativeWindow);
        void detachNativeWindow();
        ego::WindowPointer getNativeWindowPointer() const;

        bool initInstancedEvents();
        void releaseInstancedEvents();

        ego::WindowPointer m_nativeWindow = nullptr;
        InstancedEventID m_sizeEventID = InvalidInstancedEventID;
    };

    EGO_POINTER(ApplicationWindow)
} // namespace ego::application
