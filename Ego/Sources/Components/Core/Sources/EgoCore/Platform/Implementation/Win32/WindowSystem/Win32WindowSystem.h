#pragma once

#include "EgoCore/Platform/PlatformMacros.h"
#include "EgoCore/Platform/Window/WindowSystem.h"

#define EGO_WIN32_WINDOW_SYSTEM_WND_CLASS_NAME "EgoWindowCLS"

namespace ego::win32
{
    class Win32Window;

    class Win32WindowSystem final : public WindowSystem
    {
    public:
        explicit Win32WindowSystem(HINSTANCE _instance);
        ~Win32WindowSystem() override;

        bool init() override;
        void release() override;

        WindowPointer createWindow(const WindowDesc& _desc) override;
        void processEvents() override;

        HINSTANCE getInstanceHandle() const;

        EGO_RTTI_VIRTUAL(Win32WindowSystem, WindowSystem);

    private:
        friend class Win32Window;

        bool initWindowClass();

        void onWindowDestroying(const WindowPointer& _window) const;
        void onWindowActivate(const WindowPointer& _window, bool _isActive) const;
        void onWindowSizeChange(const WindowPointer& _window, const WindowSize& _prevSize) const;
        void onWindowKeyboardInput(const WindowPointer& _window, const WindowKeyboardInputData& _inputData) const;
        void onWindowTextInput(const WindowPointer& _window, const WindowTextInputData& _inputData) const;

        static LRESULT CALLBACK WndProc(HWND _hwnd, UINT _msg, WPARAM _wParam, LPARAM _lParam);

        HINSTANCE m_instance = nullptr;
        bool m_isInitialized = false;
    };

    EGO_POINTER(Win32WindowSystem);
} // namespace ego::win32
