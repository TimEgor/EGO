#include "Win32PlatformSurface.h"

#include <cstdint>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Platform/Implementation/Win32/Input/Win32MouseInputDevice.h"
#include "EgoCore/UtilsMacros.h"
#include "Win32PlatformSurfaceController.h"
#include "Win32SurfaceUtils.h"

#include <dwmapi.h>
#include <Windowsx.h>

ego::SharedPointer<ego::win32::Win32PlatformSurface> ego::win32::Win32PlatformSurface::Win32PlatformSurfaceAccessor::Create(
    const PlatformSurfaceDesc& _desc,
    Win32PlatformSurfaceController& _surfaceController,
    HINSTANCE _instance)
{
    SharedPointer<Win32PlatformSurface> surface = MakePointer<Win32PlatformSurface>();
    EGO_CHECK_RETURN_NULL(surface && surface->init(_desc, _surfaceController, _instance));

    return surface;
}

void ego::win32::Win32PlatformSurface::Win32PlatformSurfaceAccessor::Release(Win32PlatformSurface& _surface)
{
    _surface.release();
}

const ego::win32::Win32PlatformSurface::WindowData* ego::win32::Win32PlatformSurface::Win32PlatformSurfaceAccessor::GetWindowData(HWND _handle)
{
    return Win32PlatformSurface::GetWindowData(_handle);
}

bool ego::win32::Win32PlatformSurface::Win32PlatformSurfaceAccessor::ProcessWindowMessage(
    Win32PlatformSurface& _surface,
    UINT _msg,
    WPARAM _wParam,
    LPARAM _lParam,
    bool _notifyPointerCaptureLost,
    LRESULT& _result)
{
    return _surface.processWindowMessage(_msg, _wParam, _lParam, _notifyPointerCaptureLost, _result);
}

ego::win32::Win32PlatformSurface::~Win32PlatformSurface()
{
    release();
}

bool ego::win32::Win32PlatformSurface::init(const PlatformSurfaceDesc& _desc, Win32PlatformSurfaceController& _surfaceController, HINSTANCE _instance)
{
    EGO_CHECK_INITIALIZATION(initEvents());

    m_hasFrame = _desc.m_hasFrame;
    const DWORD windowStyle = m_hasFrame ? WS_OVERLAPPEDWINDOW : WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    const DWORD extendedWindowStyle = m_hasFrame ? 0 : WS_EX_WINDOWEDGE;

    int windowWidth = _desc.m_size.m_x;
    int windowHeight = _desc.m_size.m_y;
    const bool hasExplicitSize = windowWidth != 0 && windowHeight != 0;
    const bool hasAutomaticPositionX = _desc.m_position.m_x == AutomaticSurfacePositionCoordinate;
    const bool hasAutomaticPositionY = _desc.m_position.m_y == AutomaticSurfacePositionCoordinate;
    EGO_CHECK_INITIALIZATION(hasAutomaticPositionX == hasAutomaticPositionY);

    const bool hasExplicitPosition = !hasAutomaticPositionX;
    RECT windowRect;
    windowRect.left = 0;
    windowRect.top = 0;
    windowRect.right = windowWidth;
    windowRect.bottom = windowHeight;
    if (m_hasFrame && (hasExplicitSize || hasExplicitPosition))
    {
        EGO_CHECK_INITIALIZATION(AdjustWindowRectEx(&windowRect, windowStyle, false, extendedWindowStyle));
    }

    if (hasExplicitSize)
    {
        windowWidth = windowRect.right - windowRect.left;
        windowHeight = windowRect.bottom - windowRect.top;
    }

    int windowPositionX = CW_USEDEFAULT;
    int windowPositionY = CW_USEDEFAULT;
    if (hasExplicitPosition)
    {
        windowPositionX = _desc.m_position.m_x + windowRect.left;
        windowPositionY = _desc.m_position.m_y + windowRect.top;
    }

    HWND handle = CreateWindowEx(
        extendedWindowStyle,
        EGO_WIN32_WINDOW_CLASS_NAME,
        _desc.m_name.c_str(),
        windowStyle,
        windowPositionX,
        windowPositionY,
        windowWidth,
        windowHeight,
        nullptr,
        nullptr,
        _instance,
        nullptr);
    EGO_CHECK_INITIALIZATION(handle);

    m_handle = handle;
    setWindowData(_surfaceController);

    if (!m_hasFrame)
    {
        const DWMNCRENDERINGPOLICY renderingPolicy = DWMNCRP_DISABLED;
        EGO_CHECK_INITIALIZATION(SUCCEEDED(DwmSetWindowAttribute(handle, DWMWA_NCRENDERING_POLICY, &renderingPolicy, sizeof(renderingPolicy))));

        constexpr BOOL enableNonClientPaint = false;
        EGO_CHECK_INITIALIZATION(SUCCEEDED(DwmSetWindowAttribute(handle, DWMWA_ALLOW_NCPAINT, &enableNonClientPaint, sizeof(enableNonClientPaint))));

        const DWORD frameWindowStyle = windowStyle | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
        EGO_CHECK_INITIALIZATION(SetWindowLongPtr(handle, GWL_STYLE, frameWindowStyle));
        EGO_CHECK_INITIALIZATION(SetWindowPos(handle, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER));
    }

    if (_desc.m_isVisible)
    {
        show();
        UpdateWindow(handle);
    }

    updateSizes();

    return true;
}

void ego::win32::Win32PlatformSurface::release()
{
    if (m_handle)
    {
        DestroyWindow(m_handle);
    }

    releaseEvents();

    m_captionPosition = DefaultSurfacePoint;
    m_captionSize = DefaultSurfaceSize;
    m_surfaceSize = DefaultSurfaceSize;
    m_hasFrame = true;
    m_isShown = false;
    m_isInputTransparent = false;
}

void ego::win32::Win32PlatformSurface::invalidate()
{
    clearWindowData();

    m_handle = nullptr;
    m_isShown = false;
}

bool ego::win32::Win32PlatformSurface::isValid() const
{
    return m_handle;
}

void* ego::win32::Win32PlatformSurface::getNativeHandle() const
{
    return m_handle;
}

bool ego::win32::Win32PlatformSurface::show(bool _activate)
{
    const HWND handle = m_handle;
    EGO_ASSERT(handle);
    EGO_CHECK_RETURN_FALSE(handle);

    const int showCommand = _activate ? SW_SHOW : SW_SHOWNOACTIVATE;
    ShowWindow(handle, showCommand);

    if (m_handle == handle)
    {
        m_isShown = true;
    }

    return m_handle == handle;
}

void ego::win32::Win32PlatformSurface::hide()
{
    const HWND handle = m_handle;
    EGO_ASSERT(handle);
    EGO_CHECK_RETURN(handle);

    ShowWindow(handle, SW_HIDE);

    if (m_handle == handle)
    {
        m_isShown = false;
    }
}

bool ego::win32::Win32PlatformSurface::isShown() const
{
    return m_handle && m_isShown;
}

bool ego::win32::Win32PlatformSurface::minimize()
{
    const HWND handle = m_handle;
    EGO_CHECK_RETURN_FALSE(handle);

    ShowWindow(handle, SW_MINIMIZE);

    return m_handle == handle && IsIconic(handle) != FALSE;
}

bool ego::win32::Win32PlatformSurface::maximize()
{
    const HWND handle = m_handle;
    EGO_CHECK_RETURN_FALSE(handle);

    ShowWindow(handle, SW_MAXIMIZE);

    return m_handle == handle && IsZoomed(handle) != FALSE;
}

bool ego::win32::Win32PlatformSurface::restore()
{
    const HWND handle = m_handle;
    EGO_CHECK_RETURN_FALSE(handle);

    ShowWindow(handle, SW_RESTORE);

    return m_handle == handle && IsIconic(handle) == FALSE && IsZoomed(handle) == FALSE;
}

ego::PlatformSurfaceWindowState ego::win32::Win32PlatformSurface::getWindowState() const
{
    if (m_handle && IsIconic(m_handle))
    {
        return PlatformSurfaceWindowState::Minimized;
    }

    if (m_handle && IsZoomed(m_handle))
    {
        return PlatformSurfaceWindowState::Maximized;
    }

    return PlatformSurfaceWindowState::Normal;
}

bool ego::win32::Win32PlatformSurface::mapFromScreen(const SurfacePoint& _screenPoint, SurfacePoint& _surfacePoint) const
{
    const HWND handle = m_handle;
    EGO_CHECK_RETURN_FALSE(handle);

    POINT point;
    point.x = _screenPoint.m_x;
    point.y = _screenPoint.m_y;
    EGO_CHECK_RETURN_FALSE(ScreenToClient(handle, &point));

    _surfacePoint = SurfacePoint(point.x, point.y);

    return true;
}

bool ego::win32::Win32PlatformSurface::mapToScreen(const SurfacePoint& _surfacePoint, SurfacePoint& _screenPoint) const
{
    const HWND handle = m_handle;
    EGO_CHECK_RETURN_FALSE(handle);

    POINT point;
    point.x = _surfacePoint.m_x;
    point.y = _surfacePoint.m_y;
    EGO_CHECK_RETURN_FALSE(ClientToScreen(handle, &point));

    _screenPoint = SurfacePoint(point.x, point.y);

    return true;
}

bool ego::win32::Win32PlatformSurface::setPosition(const SurfacePoint& _position)
{
    const HWND handle = m_handle;
    EGO_CHECK_RETURN_FALSE(handle);

    RECT windowRect;
    windowRect.left = _position.m_x;
    windowRect.top = _position.m_y;
    windowRect.right = _position.m_x;
    windowRect.bottom = _position.m_y;

    if (m_hasFrame)
    {
        const DWORD style = static_cast<DWORD>(GetWindowLongPtr(handle, GWL_STYLE));
        const DWORD extendedStyle = static_cast<DWORD>(GetWindowLongPtr(handle, GWL_EXSTYLE));
        EGO_CHECK_RETURN_FALSE(AdjustWindowRectEx(&windowRect, style, GetMenu(handle) != nullptr, extendedStyle));
    }

    return SetWindowPos(handle, nullptr, windowRect.left, windowRect.top, 0, 0, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER) != FALSE;
}

bool ego::win32::Win32PlatformSurface::getPosition(SurfacePoint& _position) const
{
    return mapToScreen(DefaultSurfacePoint, _position);
}

bool ego::win32::Win32PlatformSurface::setSize(const SurfaceSize& _size)
{
    const HWND handle = m_handle;
    EGO_CHECK_RETURN_FALSE(handle);

    RECT windowRect;
    windowRect.left = 0;
    windowRect.top = 0;
    windowRect.right = _size.m_x;
    windowRect.bottom = _size.m_y;

    if (m_hasFrame)
    {
        const DWORD style = static_cast<DWORD>(GetWindowLongPtr(handle, GWL_STYLE));
        const DWORD extendedStyle = static_cast<DWORD>(GetWindowLongPtr(handle, GWL_EXSTYLE));
        EGO_CHECK_RETURN_FALSE(AdjustWindowRectEx(&windowRect, style, GetMenu(handle) != nullptr, extendedStyle));
    }

    return SetWindowPos(
               handle,
               nullptr,
               0,
               0,
               windowRect.right - windowRect.left,
               windowRect.bottom - windowRect.top,
               SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER) != FALSE;
}

const ego::SurfaceSize& ego::win32::Win32PlatformSurface::getSize() const
{
    return m_surfaceSize;
}

bool ego::win32::Win32PlatformSurface::setInputTransparent(bool _isTransparent)
{
    EGO_CHECK_RETURN_FALSE(m_handle);

    m_isInputTransparent = _isTransparent;

    return true;
}

bool ego::win32::Win32PlatformSurface::isInputTransparent() const
{
    return m_isInputTransparent;
}

bool ego::win32::Win32PlatformSurface::capturePointer()
{
    EGO_CHECK_RETURN_FALSE(m_handle);

    WindowData* windowData = GetWindowData(m_handle);
    EGO_CHECK_RETURN_FALSE(windowData);

    SharedPointer<Win32PlatformSurfaceController> surfaceController = windowData->m_controller.lock();
    EGO_CHECK_RETURN_FALSE(surfaceController);

    return surfaceController->capturePointer(sharedFromThis());
}

bool ego::win32::Win32PlatformSurface::releasePointer()
{
    EGO_CHECK_RETURN_FALSE(m_handle);

    if (!hasPointerCapture())
    {
        return true;
    }

    WindowData* windowData = GetWindowData(m_handle);
    EGO_CHECK_RETURN_FALSE(windowData);

    SharedPointer<Win32PlatformSurfaceController> surfaceController = windowData->m_controller.lock();
    EGO_CHECK_RETURN_FALSE(surfaceController);

    return surfaceController->releasePointer();
}

bool ego::win32::Win32PlatformSurface::hasPointerCapture() const
{
    return m_handle && GetCapture() == m_handle;
}

bool ego::win32::Win32PlatformSurface::setCaptionArea(const SurfacePoint& _position, const SurfaceSize& _size)
{
    EGO_CHECK_RETURN_FALSE(m_handle && !m_hasFrame);

    m_captionPosition = _position;
    m_captionSize = _size;

    return true;
}

bool ego::win32::Win32PlatformSurface::processWindowMessage(UINT _msg, WPARAM _wParam, LPARAM _lParam, bool _notifyPointerCaptureLost, LRESULT& _result)
{
    if (!isValid())
    {
        return false;
    }

    switch (_msg)
    {
    case WM_NCCALCSIZE:
    {
        if (!m_hasFrame)
        {
            if (_wParam)
            {
                adjustMaximizedClientRect(_lParam);
            }

            _result = 0;

            return true;
        }

        break;
    }

    case WM_NCHITTEST:
    {
        if (m_isInputTransparent)
        {
            _result = HTTRANSPARENT;

            return true;
        }

        if (!m_hasFrame)
        {
            _result = resolveHitTest(_lParam);

            return true;
        }

        break;
    }

    case WM_CLOSE:
    {
        if (onWindowCloseRequested())
        {
            _result = 0;

            return true;
        }

        break;
    }

    case WM_DESTROY:
    {
        onWindowDestroyed();
        _result = 0;

        return true;
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

    case WM_CAPTURECHANGED:
    case WM_CANCELMODE:
    {
        if (_notifyPointerCaptureLost)
        {
            onWindowPointerCaptureLost();
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
        onWindowKeyboardInput(InputButtonAction::Pressed, _wParam, _lParam);
        break;
    }

    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        onWindowKeyboardInput(InputButtonAction::Released, _wParam, _lParam);
        break;
    }

    case WM_CHAR:
    case WM_SYSCHAR:
    {
        onWindowTextInput(static_cast<SurfaceTextCodepoint>(_wParam));
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

        onWindowTextInput(static_cast<SurfaceTextCodepoint>(_wParam));
        _result = 0;

        return true;
    }
    }

    return false;
}

void ego::win32::Win32PlatformSurface::adjustMaximizedClientRect(LPARAM _lParam) const
{
    EGO_ASSERT(m_handle);

    if (!_lParam || !IsZoomed(m_handle))
    {
        return;
    }

    const HMONITOR monitor = MonitorFromWindow(m_handle, MONITOR_DEFAULTTONEAREST);
    if (!monitor)
    {
        return;
    }

    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (!GetMonitorInfo(monitor, &monitorInfo))
    {
        return;
    }

    NCCALCSIZE_PARAMS& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(_lParam);
    params.rgrc[0] = monitorInfo.rcWork;
}

bool ego::win32::Win32PlatformSurface::onWindowCloseRequested()
{
    return notifyCloseRequested();
}

void ego::win32::Win32PlatformSurface::onWindowDestroyed()
{
    invalidate();
}

void ego::win32::Win32PlatformSurface::onWindowActivate(bool _isActive)
{
    notifyActivation(_isActive);
}

void ego::win32::Win32PlatformSurface::onWindowSizeUpdate()
{
    const SurfaceSize previousSize = getSize();

    updateSizes();
    notifySizeChanged(previousSize);
}

void ego::win32::Win32PlatformSurface::onWindowPointerCaptureLost()
{
    notifyPointerCaptureLost();
}

void ego::win32::Win32PlatformSurface::onWindowKeyboardInput(InputButtonAction _action, WPARAM _wParam, LPARAM _lParam)
{
    SurfaceKeyboardInput inputData;
    inputData.m_key = Win32SurfaceUtils::ToKeyboardKey(_wParam, _lParam);
    inputData.m_isRepeat = _action == InputButtonAction::Pressed && Win32SurfaceUtils::IsRepeatedKey(_lParam);
    inputData.m_action = _action;

    notifyKeyboardInput(inputData);
}

void ego::win32::Win32PlatformSurface::onWindowTextInput(SurfaceTextCodepoint _codepoint)
{
    SurfaceTextInput inputData;
    inputData.m_codepoint = _codepoint;

    notifyTextInput(inputData);
}

LRESULT ego::win32::Win32PlatformSurface::resolveHitTest(LPARAM _lParam) const
{
    EGO_ASSERT(m_handle);

    const SurfacePoint screenPoint(GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));
    if (!IsZoomed(m_handle))
    {
        const LRESULT resizeResult = resolveResizeHitTest(screenPoint);
        if (resizeResult != HTCLIENT)
        {
            return resizeResult;
        }
    }

    SurfacePoint point;
    if (!mapFromScreen(screenPoint, point))
    {
        return HTCLIENT;
    }

    return isCaptionPoint(point) ? HTCAPTION : HTCLIENT;
}

LRESULT ego::win32::Win32PlatformSurface::resolveResizeHitTest(const SurfacePoint& _screenPoint) const
{
    RECT windowRect;
    if (!GetWindowRect(m_handle, &windowRect))
    {
        return HTCLIENT;
    }

    const UINT dpi = GetDpiForWindow(m_handle);
    const int borderWidth = GetSystemMetricsForDpi(SM_CXSIZEFRAME, dpi) + GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
    const int borderHeight = GetSystemMetricsForDpi(SM_CYSIZEFRAME, dpi) + GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);

    const int horizontalIndex =
        static_cast<int>(_screenPoint.m_x >= windowRect.left + borderWidth) + static_cast<int>(_screenPoint.m_x >= windowRect.right - borderWidth);
    const int verticalIndex =
        static_cast<int>(_screenPoint.m_y >= windowRect.top + borderHeight) + static_cast<int>(_screenPoint.m_y >= windowRect.bottom - borderHeight);

    static constexpr LRESULT TopBorderResults[] = {HTTOPLEFT, HTTOP, HTTOPRIGHT};
    static constexpr LRESULT SideBorderResults[] = {HTLEFT, HTCLIENT, HTRIGHT};
    static constexpr LRESULT BottomBorderResults[] = {HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT};
    static constexpr const LRESULT* BorderResults[] = {TopBorderResults, SideBorderResults, BottomBorderResults};

    return BorderResults[verticalIndex][horizontalIndex];
}

bool ego::win32::Win32PlatformSurface::isCaptionPoint(const SurfacePoint& _point) const
{
    const int64_t right = static_cast<int64_t>(m_captionPosition.m_x) + m_captionSize.m_x;
    const int64_t bottom = static_cast<int64_t>(m_captionPosition.m_y) + m_captionSize.m_y;

    return _point.m_x >= m_captionPosition.m_x && _point.m_y >= m_captionPosition.m_y && static_cast<int64_t>(_point.m_x) < right &&
           static_cast<int64_t>(_point.m_y) < bottom;
}

void ego::win32::Win32PlatformSurface::updateSizes()
{
    RECT clientRect;
    if (!GetClientRect(m_handle, &clientRect))
    {
        return;
    }

    const int64_t clientWidth = static_cast<int64_t>(clientRect.right) - clientRect.left;
    const int64_t clientHeight = static_cast<int64_t>(clientRect.bottom) - clientRect.top;

    m_surfaceSize.m_x = Win32SurfaceUtils::ToExtent(clientWidth);
    m_surfaceSize.m_y = Win32SurfaceUtils::ToExtent(clientHeight);
}

void ego::win32::Win32PlatformSurface::setWindowData(Win32PlatformSurfaceController& _surfaceController)
{
    EGO_ASSERT(m_handle);
    EGO_ASSERT(!GetWindowData(m_handle));

    WindowData* windowData = new WindowData{_surfaceController.weakFromThis(), weakFromThis()};
    SetWindowLongPtr(m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowData));
}

void ego::win32::Win32PlatformSurface::clearWindowData()
{
    WindowData* windowData = GetWindowData(m_handle);
    EGO_SAFE_DESTROY(windowData);

    SetWindowLongPtr(m_handle, GWLP_USERDATA, 0);
}

ego::win32::Win32PlatformSurface::WindowData* ego::win32::Win32PlatformSurface::GetWindowData(HWND _handle)
{
    return reinterpret_cast<WindowData*>(GetWindowLongPtr(_handle, GWLP_USERDATA));
}
