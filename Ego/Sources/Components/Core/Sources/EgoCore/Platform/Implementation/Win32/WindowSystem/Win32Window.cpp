#include "Win32Window.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Platform/Implementation/Win32/Input/Win32MouseInputDevice.h"
#include "EgoCore/UtilsMacros.h"
#include "Win32WindowSystem.h"

namespace
{
    constexpr int32_t KeyScanCodeOffset = 16;
    constexpr LPARAM KeyRepeatMask = static_cast<LPARAM>(1) << 30;
    constexpr LPARAM KeyExtendedMask = static_cast<LPARAM>(1) << 24;

    uint32_t GetKeyScanCode(LPARAM _lParam)
    {
        return static_cast<uint32_t>((_lParam >> KeyScanCodeOffset) & 0xff);
    }
} // namespace

ego::win32::Win32Window::Win32Window(Win32WindowSystem& _windowSystem, HINSTANCE _instance)
    : m_windowSystem(_windowSystem),
      m_instance(_instance)
{
}

ego::win32::Win32Window::~Win32Window()
{
    release();
}

bool ego::win32::Win32Window::init(const WindowDesc& _desc)
{
    const WindowSize requestedWindowSize = _desc.m_size;
    HWND handle = CreateWindow(
        EGO_WIN32_WINDOW_SYSTEM_WND_CLASS_NAME,
        _desc.m_title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        requestedWindowSize.m_x,
        requestedWindowSize.m_y,
        nullptr,
        nullptr,
        m_instance,
        nullptr);
    EGO_CHECK_INITIALIZATION(handle);

    {
        std::scoped_lock locker(m_mutex);

        m_windowSize = requestedWindowSize;
        m_handle = handle;
    }

    auto windowData = new Win32WindowWeakPointer(weakFromThis());
    SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowData));

    ShowCursor(true);

    if (_desc.m_showOnInit)
    {
        show();
        UpdateWindow(handle);
    }

    updateSizes();
    setSizeStabilization(true);

    return true;
}

void ego::win32::Win32Window::release()
{
    HWND handle = nullptr;
    {
        std::shared_lock locker(m_mutex);
        handle = m_handle;
    }

    if (handle)
    {
        DestroyWindow(handle);
    }
}

bool ego::win32::Win32Window::isValid() const
{
    std::shared_lock locker(m_mutex);

    return m_handle;
}

void ego::win32::Win32Window::show()
{
    HWND handle = nullptr;
    {
        std::shared_lock locker(m_mutex);
        handle = m_handle;
    }

    EGO_ASSERT(handle);
    EGO_CHECK_RETURN(handle);

    ShowWindow(handle, SW_NORMAL);

    {
        std::scoped_lock locker(m_mutex);
        if (m_handle == handle)
        {
            m_isShown = true;
        }
    }
}

void ego::win32::Win32Window::hide()
{
    HWND handle = nullptr;
    {
        std::shared_lock locker(m_mutex);
        handle = m_handle;
    }

    EGO_ASSERT(handle);
    EGO_CHECK_RETURN(handle);

    ShowWindow(handle, SW_HIDE);

    {
        std::scoped_lock locker(m_mutex);
        if (m_handle == handle)
        {
            m_isShown = false;
        }
    }
}

bool ego::win32::Win32Window::isShown() const
{
    std::shared_lock locker(m_mutex);

    return m_handle && m_isShown;
}

void* ego::win32::Win32Window::getNativeHandle() const
{
    return m_handle;
}

const ego::WindowSize& ego::win32::Win32Window::getWindowSize() const
{
    std::shared_lock locker(m_mutex);

    return m_windowSize;
}

const ego::WindowSize& ego::win32::Win32Window::getClientAreaSize() const
{
    std::shared_lock locker(m_mutex);

    return m_clientAreaSize;
}

const ego::WindowArea& ego::win32::Win32Window::getCutoutsArea() const
{
    std::shared_lock locker(m_mutex);

    return m_cutoutsArea;
}

HWND ego::win32::Win32Window::getHandle() const
{
    return m_handle;
}

bool ego::win32::Win32Window::isStable() const
{
    std::shared_lock locker(m_mutex);

    return m_handle && m_isSizeStable;
}

bool ego::win32::Win32Window::screenToClient(const WindowPoint& _screenPoint, WindowPoint& _clientPoint) const
{
    HWND handle = nullptr;
    {
        std::shared_lock locker(m_mutex);
        handle = m_handle;
    }

    EGO_CHECK_RETURN_FALSE(handle);

    POINT point;
    point.x = _screenPoint.m_x;
    point.y = _screenPoint.m_y;
    EGO_CHECK_RETURN_FALSE(ScreenToClient(handle, &point));

    _clientPoint = WindowPoint(point.x, point.y);
    return true;
}

bool ego::win32::Win32Window::processWindowMessage(UINT _msg, WPARAM _wParam, LPARAM _lParam, LRESULT& _result)
{
    if (!isValid())
    {
        return false;
    }

    switch (_msg)
    {
    case WM_DESTROY:
    {
        onWindowDestroying();
        _result = 0;
        return true;
    }

    case WM_ENTERSIZEMOVE:
    {
        onWindowTransformationStart();
        break;
    }

    case WM_EXITSIZEMOVE:
    {
        onWindowTransformationEnd();
        break;
    }

    case WM_SIZE:
    {
        onWindowSizeUpdate();
        break;
    }

    case WM_ACTIVATE:
    {
        onWindowActivate(LOWORD(_wParam) != WA_INACTIVE);
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
        onWindowKeyboardInput(WindowKeyboardInputAction::Pressed, _wParam, _lParam);
        break;
    }

    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        onWindowKeyboardInput(WindowKeyboardInputAction::Released, _wParam, _lParam);
        break;
    }

    case WM_CHAR:
    case WM_SYSCHAR:
    {
        onWindowTextInput(static_cast<WindowTextCodepoint>(_wParam));
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

        onWindowTextInput(static_cast<WindowTextCodepoint>(_wParam));
        _result = 0;
        return true;
    }
    }

    return false;
}

void ego::win32::Win32Window::onWindowDestroying()
{
    m_windowSystem.onWindowDestroying(sharedFromThis());
    invalidate();
}

void ego::win32::Win32Window::onWindowTransformationStart()
{
    setSizeStabilization(false);
}

void ego::win32::Win32Window::onWindowTransformationEnd()
{
    setSizeStabilization(true);
}

void ego::win32::Win32Window::onWindowSizeUpdate()
{
    const WindowSize prevWindowSize = getWindowSize();

    updateSizes();
    m_windowSystem.onWindowSizeChange(sharedFromThis(), prevWindowSize);
}

void ego::win32::Win32Window::onWindowActivate(bool _isActive)
{
    m_windowSystem.onWindowActivate(sharedFromThis(), _isActive);
}

void ego::win32::Win32Window::onWindowKeyboardInput(WindowKeyboardInputAction _action, WPARAM _wParam, LPARAM _lParam)
{
    WindowKeyboardInputData inputData;
    inputData.m_key = static_cast<WindowKeyboardKey>(_wParam);
    inputData.m_scanCode = GetKeyScanCode(_lParam);
    inputData.m_isRepeat = _action == WindowKeyboardInputAction::Pressed && (_lParam & KeyRepeatMask) != 0;
    inputData.m_isExtended = (_lParam & KeyExtendedMask) != 0;
    inputData.m_action = _action;

    m_windowSystem.onWindowKeyboardInput(sharedFromThis(), inputData);
}

void ego::win32::Win32Window::onWindowTextInput(WindowTextCodepoint _codepoint)
{
    WindowTextInputData inputData;
    inputData.m_codepoint = _codepoint;

    m_windowSystem.onWindowTextInput(sharedFromThis(), inputData);
}

void ego::win32::Win32Window::updateSizes()
{
    std::scoped_lock locker(m_mutex);

    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);
    m_windowSize.m_x = static_cast<uint16_t>(windowRect.right - windowRect.left);
    m_windowSize.m_y = static_cast<uint16_t>(windowRect.bottom - windowRect.top);

    RECT clientSize;
    GetClientRect(m_handle, &clientSize);
    m_clientAreaSize.m_x = static_cast<uint16_t>(clientSize.right - clientSize.left);
    m_clientAreaSize.m_y = static_cast<uint16_t>(clientSize.bottom - clientSize.top);

    m_cutoutsArea.m_top = static_cast<WindowArea::ValueType>(clientSize.top);
    m_cutoutsArea.m_bottom = static_cast<WindowArea::ValueType>(m_windowSize.m_y - clientSize.bottom);
    m_cutoutsArea.m_right = static_cast<WindowArea::ValueType>(m_windowSize.m_x - clientSize.right);
    m_cutoutsArea.m_left = static_cast<WindowArea::ValueType>(clientSize.left);
}

void ego::win32::Win32Window::invalidate()
{
    std::scoped_lock locker(m_mutex);

    auto windowData = reinterpret_cast<Win32WindowWeakPointer*>(GetWindowLongPtr(m_handle, GWLP_USERDATA));
    EGO_SAFE_DESTROY(windowData);

    SetWindowLongPtr(m_handle, GWLP_USERDATA, 0);

    m_handle = nullptr;
    m_isShown = false;
}

void ego::win32::Win32Window::setSizeStabilization(bool _state)
{
    std::scoped_lock locker(m_mutex);

    m_isSizeStable = _state;
}
