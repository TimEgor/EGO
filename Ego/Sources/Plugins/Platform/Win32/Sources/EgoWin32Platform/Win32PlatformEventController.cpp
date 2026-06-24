#include "Win32PlatformEventController.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Platform/Platform.h"

#include "EgoWin32Platform/Window/Win32Window.h"

void ego::win32::Win32WindowEventController::updateNativeEvents()
{
    UpdateNativeWindowEvents(nullptr);
    engine::GetEngine().getPlatform().getFileSystem()->updateDirectoryWatches();
}

LRESULT ego::win32::Win32WindowEventController::WndProc(HWND _hwnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    auto windowData = reinterpret_cast<Win32WindowWeakPointer*>(GetWindowLongPtr(_hwnd, 0));

    if (windowData)
    {
        Win32WindowPointer win32Window = windowData->lock();
        if (win32Window && win32Window->isValid())
        {
            switch (_msg)
            {
            case WM_DESTROY:
            {
                Win32Window::Accessor::OnWindowDestroying(*win32Window);
                return 0;
            }

            case WM_ENTERSIZEMOVE:
            {
                Win32Window::Accessor::OnWindowTransformationStart(*win32Window);
                break;
            }

            case WM_EXITSIZEMOVE:
            {
                Win32Window::Accessor::OnWindowTransformationEnd(*win32Window);
                break;
            }
            case WM_SIZE:
            {
                const WindowSize prevWindowSize = win32Window->getWindowSize();
                Win32Window::Accessor::OnWindowSizeUpdate(*win32Window);
                OnWindowSizeChange(win32Window, prevWindowSize);
                break;
            }
            case WM_ACTIVATE:
            {
                const bool activationState = LOWORD(_wParam);
                OnWindowActivate(win32Window, activationState);
                break;
            }
            }
        }
    }

    return DefWindowProc(_hwnd, _msg, _wParam, _lParam);
}

void ego::win32::Win32WindowEventController::UpdateNativeWindowEvents(HWND _hWnd)
{
    MSG msg{0};
    while (PeekMessage(&msg, _hWnd, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            ego::engine::GetEngine().stop();
            break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
