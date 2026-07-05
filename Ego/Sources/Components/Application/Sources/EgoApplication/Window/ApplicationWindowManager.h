#pragma once

#include <vector>

#include "ApplicationWindow.h"

namespace ego
{
    class WindowSystem;
} // namespace ego

namespace ego::application
{
    class ApplicationWindowManager
    {
    public:
        ApplicationWindowManager() = default;
        virtual ~ApplicationWindowManager() = default;

        bool init();
        void release();

        ApplicationWindowPointer createApplicationWindow(const WindowDesc& _desc);
        void processWindowEvents();

        EGO_RTTI_VIRTUAL_BASE(::ego::application::ApplicationWindowManager);

    private:
        using ApplicationWindowCollection = std::vector<ApplicationWindowPointer>;

        ego::WindowSystem& getNativeWindowSystem() const;
        bool registerApplicationWindowEvents();
        void unregisterApplicationWindowEvents();
        void bindNativeWindowSystemHandlers();
        void unbindNativeWindowSystemHandlers();
        void releaseApplicationWindows();

        ApplicationWindowPointer findApplicationWindow(const ego::WindowPointer& _nativeWindow) const;
        void removeApplicationWindow(const ApplicationWindowPointer& _window);

        void handleNativeQuitRequested();
        void handleNativeWindowDestroying(const ego::WindowPointer& _nativeWindow);
        void handleNativeWindowActivation(const ego::WindowPointer& _nativeWindow, bool _isActive);
        void handleNativeWindowSizeChanged(const ego::WindowPointer& _nativeWindow, const WindowSize& _prevSize);

        void emitApplicationQuitRequested() const;
        void emitApplicationWindowDestroying(const ApplicationWindowPointer& _window) const;
        void emitApplicationWindowActivation(const ApplicationWindowPointer& _window, bool _isActive) const;
        void emitApplicationWindowSizeChanged(const ApplicationWindowPointer& _window, const WindowSize& _prevSize) const;

        ApplicationWindowCollection m_windows;
        bool m_isInitialized = false;
    };

    EGO_POINTER(ApplicationWindowManager);
} // namespace ego::application
