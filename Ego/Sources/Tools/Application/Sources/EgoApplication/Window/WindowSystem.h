#pragma once

#include "Window.h"

namespace ego
{
    class WindowSystem
    {
    public:
        WindowSystem() = default;
        virtual ~WindowSystem() = default;

        virtual bool init();
        virtual void release();

        virtual WindowPointer createWindow(const WindowDesc& _desc) = 0;
        virtual void processEvents() = 0;

        EGO_RTTI_VIRTUAL_BASE(WindowSystem);

    protected:
        static void EmitQuitRequested();
        static void EmitWindowDestroying(const WindowPointer& _window);
        static void EmitWindowActivate(const WindowPointer& _window, bool _isActive);
        static void EmitWindowSizeChange(const WindowPointer& _window, const WindowSize& _prevSize);
    };

    EGO_POINTER(WindowSystem);
} // namespace ego
