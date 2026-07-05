#include "Win32WindowSystem.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/UtilsMacros.h"

#include "Win32Window.h"

ego::win32::Win32WindowSystem::Win32WindowSystem(HINSTANCE _instance)
    : m_instance(_instance)
{
}

ego::win32::Win32WindowSystem::~Win32WindowSystem()
{
    release();
}

bool ego::win32::Win32WindowSystem::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    EGO_CHECK_INITIALIZATION(WindowSystem::init());
    EGO_CHECK_INITIALIZATION(initWindowClass());

    m_isInitialized = true;

    return true;
}

void ego::win32::Win32WindowSystem::release()
{
    if (!m_isInitialized)
    {
        return;
    }

    UnregisterClass(EGO_WIN32_WINDOW_SYSTEM_WND_CLASS_NAME, m_instance);
    WindowSystem::release();

    m_isInitialized = false;
}

ego::WindowPointer ego::win32::Win32WindowSystem::createWindow(const WindowDesc& _desc)
{
    Win32WindowPointer window = new Win32Window(*this, m_instance);
    EGO_CHECK_RETURN_NULL(window && window->init(_desc));

    return window;
}

void ego::win32::Win32WindowSystem::processEvents()
{
    MSG msg{0};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            notifyQuitRequested();
            break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

HINSTANCE ego::win32::Win32WindowSystem::getInstanceHandle() const
{
    return m_instance;
}

void ego::win32::Win32WindowSystem::onWindowDestroying(const WindowPointer& _window) const
{
    notifyWindowDestroying(_window);
}

void ego::win32::Win32WindowSystem::onWindowActivate(const WindowPointer& _window, bool _isActive) const
{
    notifyWindowActivate(_window, _isActive);
}

void ego::win32::Win32WindowSystem::onWindowSizeChange(
    const WindowPointer& _window,
    const WindowSize& _prevSize) const
{
    notifyWindowSizeChange(_window, _prevSize);
}

bool ego::win32::Win32WindowSystem::initWindowClass()
{
    WNDCLASS wndClass{};
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wndClass.hInstance = m_instance;
    wndClass.lpfnWndProc = WndProc;
    wndClass.lpszClassName = EGO_WIN32_WINDOW_SYSTEM_WND_CLASS_NAME;
    wndClass.lpszMenuName = nullptr;
    wndClass.style = 0;

    if (!RegisterClass(&wndClass))
    {
        EGO_ASSERT_FAIL_MESSAGE("WNDCLASS registration has been failed.");
        return false;
    }

    return true;
}

LRESULT ego::win32::Win32WindowSystem::WndProc(HWND _hwnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    auto windowData = reinterpret_cast<Win32WindowWeakPointer*>(GetWindowLongPtr(_hwnd, GWLP_USERDATA));

    if (windowData)
    {
        Win32WindowPointer win32Window = windowData->lock();
        if (win32Window)
        {
            LRESULT result = 0;
            if (win32Window->processWindowMessage(_msg, _wParam, _lParam, result))
            {
                return result;
            }
        }
    }

    return DefWindowProc(_hwnd, _msg, _wParam, _lParam);
}
