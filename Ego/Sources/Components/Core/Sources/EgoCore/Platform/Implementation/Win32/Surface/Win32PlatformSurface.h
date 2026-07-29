#pragma once

#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Platform/PlatformMacros.h"
#include "EgoCore/Platform/Surface/PlatformSurface.h"

namespace ego::win32
{
    class Win32PlatformSurfaceController;

    class Win32PlatformSurface final : public PlatformSurface, public EnableSharedFromThis<Win32PlatformSurface>
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
            static bool ProcessWindowMessage(
                Win32PlatformSurface& _surface,
                UINT _msg,
                WPARAM _wParam,
                LPARAM _lParam,
                bool _notifyPointerCaptureLost,
                LRESULT& _result);
        };

        Win32PlatformSurface() = default;
        ~Win32PlatformSurface() override;

        bool isValid() const override;
        void* getNativeHandle() const override;

        bool show(bool _activate = true) override;
        void hide() override;
        bool isShown() const override;

        bool minimize() override;
        bool maximize() override;
        bool restore() override;
        PlatformSurfaceWindowState getWindowState() const override;

        bool mapFromScreen(const SurfacePoint& _screenPoint, SurfacePoint& _surfacePoint) const override;
        bool mapToScreen(const SurfacePoint& _surfacePoint, SurfacePoint& _screenPoint) const override;

        bool setPosition(const SurfacePoint& _position) override;
        bool getPosition(SurfacePoint& _position) const override;

        bool setSize(const SurfaceSize& _size) override;
        const SurfaceSize& getSize() const override;

        bool setInputTransparent(bool _isTransparent) override;
        bool isInputTransparent() const override;

        bool capturePointer() override;
        bool releasePointer() override;
        bool hasPointerCapture() const override;

        bool setCaptionArea(const SurfacePoint& _position, const SurfaceSize& _size) override;

        EGO_RTTI_VIRTUAL(Win32PlatformSurface, PlatformSurface);

    private:
        bool init(const PlatformSurfaceDesc& _desc, Win32PlatformSurfaceController& _surfaceController, HINSTANCE _instance);
        void release();
        void invalidate();

        bool processWindowMessage(UINT _msg, WPARAM _wParam, LPARAM _lParam, bool _notifyPointerCaptureLost, LRESULT& _result);
        void adjustMaximizedClientRect(LPARAM _lParam) const;

        bool onWindowCloseRequested();
        void onWindowDestroyed();

        void onWindowActivate(bool _isActive);
        void onWindowSizeUpdate();

        void onWindowPointerCaptureLost();
        void onWindowKeyboardInput(InputButtonAction _action, WPARAM _wParam, LPARAM _lParam);
        void onWindowTextInput(SurfaceTextCodepoint _codepoint);

        LRESULT resolveHitTest(LPARAM _lParam) const;
        LRESULT resolveResizeHitTest(const SurfacePoint& _screenPoint) const;
        bool isCaptionPoint(const SurfacePoint& _point) const;

        void updateSizes();

        void setWindowData(Win32PlatformSurfaceController& _surfaceController);
        void clearWindowData();

        static WindowData* GetWindowData(HWND _handle);

        SurfacePoint m_captionPosition = DefaultSurfacePoint;
        SurfaceSize m_captionSize = DefaultSurfaceSize;
        SurfaceSize m_surfaceSize = DefaultSurfaceSize;

        HWND m_handle = nullptr;

        bool m_hasFrame = true;
        bool m_isShown = false;
        bool m_isInputTransparent = false;
    };

    EGO_POINTER(Win32PlatformSurface);
    EGO_WEAK_POINTER(Win32PlatformSurface);
} // namespace ego::win32
