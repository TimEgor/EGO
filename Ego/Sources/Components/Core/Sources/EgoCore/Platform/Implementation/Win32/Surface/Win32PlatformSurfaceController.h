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
        void release();

        PlatformSurfacePointer createSurface(const PlatformSurfaceDesc& _desc) override;
        bool destroySurface(const PlatformSurfacePointer& _surface) override;
        PlatformSurfacePointer findSurfaceAtPoint(const SurfacePoint& _point) const override;
        bool capturePointer(const PlatformSurfacePointer& _surface) override;
        bool releasePointer() override;
        PlatformSurfacePointer getPointerCapture() const override;
        void processEvents() override;

        EGO_RTTI_VIRTUAL(Win32PlatformSurfaceController, PlatformSurfaceController);

    private:
        bool initWindowClass();
        Win32PlatformSurfacePointer findWindow(HWND _handle) const;
        bool processWindowMessage(const Win32PlatformSurfacePointer& _window, UINT _msg, WPARAM _wParam, LPARAM _lParam, LRESULT& _result);

        void onWindowDestroyed(const Win32PlatformSurfacePointer& _window);
        bool onWindowPointerCaptureLost(const Win32PlatformSurfacePointer& _window);

        static LRESULT CALLBACK WndProc(HWND _hwnd, UINT _msg, WPARAM _wParam, LPARAM _lParam);

        HINSTANCE m_instance = nullptr;
        HWND m_pointerCaptureHandle = nullptr;
        bool m_isUpdatingPointerCapture = false;
        bool m_isInitialized = false;
    };

    EGO_POINTER(Win32PlatformSurfaceController);
} // namespace ego::win32
