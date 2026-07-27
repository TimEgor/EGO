#pragma once

#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Platform/PlatformMacros.h"
#include "EgoCore/Platform/Surface/PlatformSurface.h"

namespace ego::win32
{
    class Win32PlatformSurfaceController;

    class Win32PlatformSurface final :
        public PlatformSurface,
        public EnableSharedFromThis<Win32PlatformSurface>
    {
    public:
        struct WindowData final
        {
            WeakPointer<Win32PlatformSurfaceController> m_controller;
            WeakPointer<Win32PlatformSurface> m_surface;
        };

        class Win32PlatformSurfaceAccessor final : public NonInstanceable
        {
            friend class Win32PlatformSurfaceController;

            static SharedPointer<Win32PlatformSurface> Create(
                const PlatformSurfaceDesc& _desc,
                Win32PlatformSurfaceController& _surfaceController,
                HINSTANCE _instance);
            static void Release(Win32PlatformSurface& _surface);

            static const WindowData* GetWindowData(HWND _handle);

            static bool OnWindowCloseRequested(Win32PlatformSurface& _surface);
            static void OnWindowDestroying(Win32PlatformSurface& _surface);

            static void OnWindowActivate(Win32PlatformSurface& _surface, bool _isActive);
            static void OnWindowSizeUpdate(Win32PlatformSurface& _surface);

            static void OnWindowPointerCaptureLost(Win32PlatformSurface& _surface);
            static void OnWindowKeyboardInput(Win32PlatformSurface& _surface, InputButtonAction _action, WPARAM _wParam, LPARAM _lParam);
            static void OnWindowTextInput(Win32PlatformSurface& _surface, SurfaceTextCodepoint _codepoint);
        };

        Win32PlatformSurface() = default;
        ~Win32PlatformSurface() override;

        bool isValid() const override;
        void* getNativeHandle() const override;

        bool show(bool _activate = true) override;
        void hide() override;
        bool isShown() const override;

        bool mapFromScreen(const SurfacePoint& _screenPoint, SurfacePoint& _surfacePoint) const override;
        bool mapToScreen(const SurfacePoint& _surfacePoint, SurfacePoint& _screenPoint) const override;

        bool setPosition(const SurfacePoint& _position) override;
        bool getPosition(SurfacePoint& _position) const override;

        bool setSize(const SurfaceSize& _size) override;
        const SurfaceSize& getSize() const override;

        bool setInputPassthrough(bool _isEnabled) override;
        bool isInputPassthrough() const;

        bool setPointerCapture(bool _isCaptured) override;
        bool hasPointerCapture() const override;

        EGO_RTTI_VIRTUAL(Win32PlatformSurface, PlatformSurface);

    private:
        bool init(const PlatformSurfaceDesc& _desc, Win32PlatformSurfaceController& _surfaceController, HINSTANCE _instance);
        void release();
        void invalidate();

        bool onWindowCloseRequested();
        void onWindowDestroying();

        void onWindowActivate(bool _isActive);
        void onWindowSizeUpdate();

        void onWindowPointerCaptureLost();
        void onWindowKeyboardInput(InputButtonAction _action, WPARAM _wParam, LPARAM _lParam);
        void onWindowTextInput(SurfaceTextCodepoint _codepoint);

        void updateSizes();

        void setWindowData(Win32PlatformSurfaceController& _surfaceController);
        void clearWindowData();

        static WindowData* GetWindowData(HWND _handle);

        SurfaceSize m_surfaceSize = DefaultSurfaceSize;

        HWND m_handle = nullptr;

        bool m_isShown = false;
        bool m_isInputPassthrough = false;
    };

    EGO_POINTER(Win32PlatformSurface);
    EGO_WEAK_POINTER(Win32PlatformSurface);
} // namespace ego::win32
