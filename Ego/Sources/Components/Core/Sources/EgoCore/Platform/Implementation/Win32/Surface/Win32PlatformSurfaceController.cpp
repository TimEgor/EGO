#include "Win32PlatformSurfaceController.h"

#include <cstring>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Platform/Implementation/Win32/Input/Win32MouseInputDevice.h"
#include "EgoCore/UtilsMacros.h"

#include "Win32PlatformSurface.h"

namespace
{
    using SurfaceAccessor = ego::win32::Win32PlatformSurface::Win32PlatformSurfaceAccessor;
} // namespace

ego::win32::Win32PlatformSurfaceController::Win32PlatformSurfaceController(HINSTANCE _instance)
    : m_instance(_instance)
{
}

ego::win32::Win32PlatformSurfaceController::~Win32PlatformSurfaceController()
{
    release();
}

bool ego::win32::Win32PlatformSurfaceController::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    EGO_CHECK_INITIALIZATION(initWindowClass());

    m_isInitialized = true;

    return true;
}

void ego::win32::Win32PlatformSurfaceController::release()
{
    if (!m_isInitialized)
    {
        return;
    }

    setPointerCapture(nullptr);
    UnregisterClass(EGO_WIN32_WINDOW_CLASS_NAME, m_instance);

    m_isInitialized = false;
}

ego::PlatformSurfacePointer ego::win32::Win32PlatformSurfaceController::createSurface(const PlatformSurfaceDesc& _desc)
{
    return SurfaceAccessor::Create(_desc, *this, m_instance);
}

bool ego::win32::Win32PlatformSurfaceController::destroySurface(const PlatformSurfacePointer& _surface)
{
    EGO_CHECK_RETURN_FALSE(_surface);

    const Win32PlatformSurfacePointer window = findWindow(static_cast<HWND>(_surface->getNativeHandle()));
    EGO_CHECK_RETURN_FALSE(window && window.get() == _surface.get());

    SurfaceAccessor::Release(*window);

    return !window->isValid();
}

ego::PlatformSurfacePointer ego::win32::Win32PlatformSurfaceController::findSurfaceAtPoint(const SurfacePoint& _point) const
{
    POINT point;
    point.x = _point.m_x;
    point.y = _point.m_y;

    HWND windowHandle = GetAncestor(WindowFromPoint(point), GA_ROOT);
    while (windowHandle)
    {
        RECT windowRect;
        const bool containsPoint = IsWindowVisible(windowHandle) && IsWindowEnabled(windowHandle) && !IsIconic(windowHandle) &&
                                   GetWindowRect(windowHandle, &windowRect) && PtInRect(&windowRect, point);
        if (containsPoint)
        {
            const Win32PlatformSurfacePointer window = findWindow(windowHandle);
            if (!window)
            {
                return nullptr;
            }

            if (!window->isInputPassthrough())
            {
                return window;
            }
        }

        windowHandle = GetWindow(windowHandle, GW_HWNDNEXT);
    }

    return nullptr;
}

bool ego::win32::Win32PlatformSurfaceController::setPointerCapture(const PlatformSurfacePointer& _surface)
{
    HWND targetHandle = nullptr;
    if (_surface)
    {
        targetHandle = static_cast<HWND>(_surface->getNativeHandle());
        EGO_CHECK_RETURN_FALSE(targetHandle && findWindow(targetHandle).get() == _surface.get());
    }

    const HWND captureHandle = GetCapture();
    if (captureHandle == targetHandle)
    {
        m_pointerCaptureHandle = targetHandle;
        return true;
    }

    if (!targetHandle && !findWindow(captureHandle))
    {
        m_pointerCaptureHandle = nullptr;
        return true;
    }

    m_isUpdatingPointerCapture = true;
    if (targetHandle)
    {
        SetCapture(targetHandle);
    }
    else
    {
        ReleaseCapture();
    }
    m_isUpdatingPointerCapture = false;

    const bool isUpdated = GetCapture() == targetHandle;
    m_pointerCaptureHandle = isUpdated ? targetHandle : nullptr;

    return isUpdated;
}

ego::PlatformSurfacePointer ego::win32::Win32PlatformSurfaceController::getPointerCapture() const
{
    return findWindow(GetCapture());
}

void ego::win32::Win32PlatformSurfaceController::processEvents()
{
    MSG msg{0};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

HINSTANCE ego::win32::Win32PlatformSurfaceController::getInstanceHandle() const
{
    return m_instance;
}

ego::win32::Win32PlatformSurfacePointer ego::win32::Win32PlatformSurfaceController::findWindow(HWND _handle) const
{
    EGO_CHECK_RETURN_NULL(_handle);

    _handle = GetAncestor(_handle, GA_ROOT);
    EGO_CHECK_RETURN_NULL(_handle);

    DWORD processID = 0;
    EGO_CHECK_RETURN_NULL(GetWindowThreadProcessId(_handle, &processID) != 0);
    EGO_CHECK_RETURN_NULL(processID == GetCurrentProcessId());

    char className[sizeof(EGO_WIN32_WINDOW_CLASS_NAME)]{};
    const int classNameLength = GetClassName(_handle, className, static_cast<int>(sizeof(className)));
    EGO_CHECK_RETURN_NULL(classNameLength == static_cast<int>(sizeof(EGO_WIN32_WINDOW_CLASS_NAME) - 1));
    EGO_CHECK_RETURN_NULL(strcmp(className, EGO_WIN32_WINDOW_CLASS_NAME) == 0);

    const Win32PlatformSurface::WindowData* windowData = SurfaceAccessor::GetWindowData(_handle);

    return windowData ? windowData->m_surface.lock() : nullptr;
}

bool ego::win32::Win32PlatformSurfaceController::processWindowMessage(
    const Win32PlatformSurfacePointer& _window,
    UINT _msg,
    WPARAM _wParam,
    LPARAM _lParam,
    LRESULT& _result)
{
    if (!_window || !_window->isValid())
    {
        return false;
    }

    switch (_msg)
    {
    case WM_CLOSE:
    {
        if (SurfaceAccessor::OnWindowCloseRequested(*_window))
        {
            _result = 0;

            return true;
        }

        break;
    }

    case WM_NCHITTEST:
    {
        if (_window->isInputPassthrough())
        {
            _result = HTTRANSPARENT;

            return true;
        }

        break;
    }

    case WM_DESTROY:
    {
        onWindowDestroying(_window);
        SurfaceAccessor::OnWindowDestroying(*_window);
        _result = 0;

        return true;
    }

    case WM_SIZE:
    {
        SurfaceAccessor::OnWindowSizeUpdate(*_window);
        break;
    }

    case WM_ACTIVATE:
    {
        SurfaceAccessor::OnWindowActivate(*_window, LOWORD(_wParam) != WA_INACTIVE);
        break;
    }

    case WM_CAPTURECHANGED:
    case WM_CANCELMODE:
    {
        if (onWindowPointerCaptureLost(_window))
        {
            SurfaceAccessor::OnWindowPointerCaptureLost(*_window);
        }

        break;
    }

    case WM_MOUSEWHEEL:
    {
        const InputDeviceKeyValue wheelDelta = static_cast<InputDeviceKeyValue>(GET_WHEEL_DELTA_WPARAM(_wParam)) / WHEEL_DELTA;
        Win32MouseInputDevice::AddWheelDelta(wheelDelta);
        _result = 0;

        return true;
    }

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        SurfaceAccessor::OnWindowKeyboardInput(*_window, InputButtonAction::Pressed, _wParam, _lParam);
        break;
    }

    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        SurfaceAccessor::OnWindowKeyboardInput(*_window, InputButtonAction::Released, _wParam, _lParam);
        break;
    }

    case WM_CHAR:
    case WM_SYSCHAR:
    {
        SurfaceAccessor::OnWindowTextInput(*_window, static_cast<SurfaceTextCodepoint>(_wParam));
        _result = 0;

        return true;
    }

    case WM_UNICHAR:
    {
        if (_wParam == UNICODE_NOCHAR)
        {
            _result = TRUE;
            return true;
        }

        SurfaceAccessor::OnWindowTextInput(*_window, static_cast<SurfaceTextCodepoint>(_wParam));
        _result = 0;

        return true;
    }
    }

    return false;
}

void ego::win32::Win32PlatformSurfaceController::onWindowDestroying(const Win32PlatformSurfacePointer& _window)
{
    if (_window && _window->getNativeHandle() == m_pointerCaptureHandle)
    {
        setPointerCapture(nullptr);
    }
}

bool ego::win32::Win32PlatformSurfaceController::onWindowPointerCaptureLost(const Win32PlatformSurfacePointer& _window)
{
    if (m_isUpdatingPointerCapture || !_window || _window->getNativeHandle() != m_pointerCaptureHandle)
    {
        return false;
    }

    if (GetCapture() == m_pointerCaptureHandle)
    {
        m_isUpdatingPointerCapture = true;
        ReleaseCapture();
        m_isUpdatingPointerCapture = false;
    }

    m_pointerCaptureHandle = nullptr;

    return true;
}

bool ego::win32::Win32PlatformSurfaceController::initWindowClass()
{
    WNDCLASS wndClass{};
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wndClass.hInstance = m_instance;
    wndClass.lpfnWndProc = WndProc;
    wndClass.lpszClassName = EGO_WIN32_WINDOW_CLASS_NAME;
    wndClass.lpszMenuName = nullptr;
    wndClass.style = 0;

    if (!RegisterClass(&wndClass))
    {
        EGO_ASSERT_FAIL_MESSAGE("WNDCLASS registration has been failed.");
        return false;
    }

    return true;
}

LRESULT ego::win32::Win32PlatformSurfaceController::WndProc(HWND _hwnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    const Win32PlatformSurface::WindowData* windowData = SurfaceAccessor::GetWindowData(_hwnd);
    if (windowData)
    {
        Win32PlatformSurfaceControllerPointer controller = windowData->m_controller.lock();
        Win32PlatformSurfacePointer win32Surface = windowData->m_surface.lock();

        if (controller && win32Surface)
        {
            LRESULT result = 0;
            if (controller->processWindowMessage(win32Surface, _msg, _wParam, _lParam, result))
            {
                return result;
            }
        }
    }

    return DefWindowProc(_hwnd, _msg, _wParam, _lParam);
}
