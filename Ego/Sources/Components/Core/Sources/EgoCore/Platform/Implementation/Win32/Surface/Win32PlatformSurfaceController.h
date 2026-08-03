#pragma once

#include "EgoCore/Platform/PlatformMacros.h"
#include "EgoCore/Platform/Surface/PlatformSurfaceController.h"

#include "Win32PlatformSurface.h"

#define EGO_WIN32_WINDOW_CLASS_NAME "EgoWindowCLS"

namespace ego::win32
{
    class Win32PlatformSurfaceController final : public PlatformSurfaceController, public EnableSharedFromThis<Win32PlatformSurfaceController>
    {
    public:
        explicit Win32PlatformSurfaceController(HINSTANCE _instance);
        ~Win32PlatformSurfaceController() override;

        bool init();

        PlatformSurfacePointer createSurface(const PlatformSurfaceDesc& _desc) override;
        bool destroySurface(const PlatformSurfacePointer& _surface) override;
        PlatformSurfacePointer findSurfaceAtPoint(const SurfacePoint& _point) const override;
        bool capturePointer(const PlatformSurfacePointer& _surface) override;
        bool releasePointer() override;
        PlatformSurfacePointer getPointerCapture() const override;
        void processEvents() override;

        EGO_RTTI_VIRTUAL(Win32PlatformSurfaceController, PlatformSurfaceController);

    private:
        void release();

        bool initWindowClass();
        Win32PlatformSurfacePointer findWindow(HWND _handle) const;

        void onSurfaceDestroyed(const Win32PlatformSurface& _surface);
        bool onSurfacePointerCaptureLost(const Win32PlatformSurface& _surface);

        static LRESULT CALLBACK WndProc(HWND _hwnd, UINT _msg, WPARAM _wParam, LPARAM _lParam);

        HINSTANCE m_instance = nullptr;
        HWND m_pointerCaptureHandle = nullptr;
        bool m_isUpdatingPointerCapture = false;
        bool m_isInitialized = false;
    };

    EGO_POINTER(Win32PlatformSurfaceController);
} // namespace ego::win32
