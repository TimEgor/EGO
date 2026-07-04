#pragma once

#include <vector>

#include "EgoCore/Platform/Window/PlatformWindowSystem.h"

#include "Window.h"

namespace ego
{
    class WindowSystem
    {
    public:
        WindowSystem() = default;
        virtual ~WindowSystem() = default;

        bool init();
        void release();

        WindowPointer createWindow(const WindowDesc& _desc);
        void processEvents();

        EGO_RTTI_VIRTUAL_BASE(WindowSystem);

    private:
        using WindowCollection = std::vector<WindowPointer>;

        static PlatformWindowDesc CreatePlatformWindowDesc(const WindowDesc& _desc);
        static WindowSize CreateWindowSize(const PlatformWindowSize& _size);

        WindowPointer findWindow(const PlatformWindowPointer& _platformWindow) const;
        void setupPlatformWindowSystemHandlers();
        void resetPlatformWindowSystemHandlers();

        void onPlatformQuitRequested();
        void onPlatformWindowDestroying(const PlatformWindowPointer& _platformWindow);
        void onPlatformWindowActivate(const PlatformWindowPointer& _platformWindow, bool _isActive);
        void onPlatformWindowSizeChange(const PlatformWindowPointer& _platformWindow, const PlatformWindowSize& _prevSize);

        void emitQuitRequested() const;
        void emitWindowDestroying(const WindowPointer& _window) const;
        void emitWindowActivate(const WindowPointer& _window, bool _isActive) const;
        void emitWindowSizeChange(const WindowPointer& _window, const WindowSize& _prevSize) const;

        PlatformWindowSystemPointer m_platformWindowSystem = nullptr;
        WindowCollection m_windows;
    };

    EGO_POINTER(WindowSystem);
} // namespace ego
