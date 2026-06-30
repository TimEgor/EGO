#pragma once

#include "EgoCore/PlatformMacros.h"

#include "EgoApplication/Window/WindowSystem.h"

#define EGO_WIN32_WINDOW_SYSTEM_WND_CLASS_NAME "EgoApplicationWindowCLS"

namespace ego::win32
{
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
        bool initWindowClass();

        static LRESULT CALLBACK WndProc(HWND _hwnd, UINT _msg, WPARAM _wParam, LPARAM _lParam);

        HINSTANCE m_instance = nullptr;
        bool m_isInitialized = false;
    };

    EGO_POINTER(Win32WindowSystem);
} // namespace ego::win32
