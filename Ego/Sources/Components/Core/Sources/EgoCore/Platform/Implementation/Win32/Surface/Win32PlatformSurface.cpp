#include "Win32PlatformSurface.h"

#include <cstdint>
#include <limits>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"
#include "Win32PlatformSurfaceController.h"

namespace
{
    constexpr int32_t KeyScanCodeOffset = 16;
    constexpr LPARAM KeyRepeatMask = static_cast<LPARAM>(1) << 30;
    constexpr LPARAM KeyExtendedMask = static_cast<LPARAM>(1) << 24;

    uint16_t ToWindowExtent(int64_t _value)
    {
        if (_value <= 0)
        {
            return 0;
        }

        const int64_t maximumExtent = static_cast<int64_t>((std::numeric_limits<uint16_t>::max)());

        return _value < maximumExtent ? static_cast<uint16_t>(_value) : (std::numeric_limits<uint16_t>::max)();
    }

    ego::KeyboardInputKey OffsetKeyboardInputKey(ego::KeyboardInputKey _firstKey, WPARAM _offset)
    {
        return static_cast<ego::KeyboardInputKey>(static_cast<ego::InputDeviceKey>(_firstKey) + static_cast<ego::InputDeviceKey>(_offset));
    }

    bool IsExtendedKeyboardKey(LPARAM _lParam)
    {
        return (_lParam & KeyExtendedMask) != 0;
    }

    WPARAM ResolveKeyboardModifierKey(WPARAM _key, LPARAM _lParam)
    {
        switch (_key)
        {
        case VK_SHIFT:
        {
            const UINT scanCode = static_cast<UINT>(_lParam >> KeyScanCodeOffset) & 0xff;
            const UINT virtualKey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
            return virtualKey == VK_RSHIFT ? VK_RSHIFT : VK_LSHIFT;
        }
        case VK_CONTROL:
            return IsExtendedKeyboardKey(_lParam) ? VK_RCONTROL : VK_LCONTROL;
        case VK_MENU:
            return IsExtendedKeyboardKey(_lParam) ? VK_RMENU : VK_LMENU;
        default:
            return _key;
        }
    }

    ego::KeyboardInputKey ToKeyboardInputKey(WPARAM _key, LPARAM _lParam)
    {
        if (_key >= '0' && _key <= '9')
        {
            return OffsetKeyboardInputKey(ego::KeyboardInputKey::Number0, _key - '0');
        }

        if (_key >= 'A' && _key <= 'Z')
        {
            return OffsetKeyboardInputKey(ego::KeyboardInputKey::A, _key - 'A');
        }

        if (_key >= VK_F1 && _key <= VK_F24)
        {
            return OffsetKeyboardInputKey(ego::KeyboardInputKey::F1, _key - VK_F1);
        }

        if (_key >= VK_NUMPAD0 && _key <= VK_NUMPAD9)
        {
            return OffsetKeyboardInputKey(ego::KeyboardInputKey::Numpad0, _key - VK_NUMPAD0);
        }

        if (_key == VK_RETURN)
        {
            return IsExtendedKeyboardKey(_lParam) ? ego::KeyboardInputKey::NumpadEnter : ego::KeyboardInputKey::Enter;
        }

        _key = ResolveKeyboardModifierKey(_key, _lParam);
        switch (_key)
        {
        case VK_CANCEL:
            return ego::KeyboardInputKey::Cancel;
        case VK_BACK:
            return ego::KeyboardInputKey::Backspace;
        case VK_TAB:
            return ego::KeyboardInputKey::Tab;
        case VK_CLEAR:
            return ego::KeyboardInputKey::Clear;
        case VK_PAUSE:
            return ego::KeyboardInputKey::Pause;
        case VK_ESCAPE:
            return ego::KeyboardInputKey::Escape;
        case VK_SPACE:
            return ego::KeyboardInputKey::Space;
        case VK_PRIOR:
            return ego::KeyboardInputKey::PageUp;
        case VK_NEXT:
            return ego::KeyboardInputKey::PageDown;
        case VK_END:
            return ego::KeyboardInputKey::End;
        case VK_HOME:
            return ego::KeyboardInputKey::Home;
        case VK_LEFT:
            return ego::KeyboardInputKey::Left;
        case VK_UP:
            return ego::KeyboardInputKey::Up;
        case VK_RIGHT:
            return ego::KeyboardInputKey::Right;
        case VK_DOWN:
            return ego::KeyboardInputKey::Down;
        case VK_SELECT:
            return ego::KeyboardInputKey::Select;
        case VK_PRINT:
            return ego::KeyboardInputKey::Print;
        case VK_EXECUTE:
            return ego::KeyboardInputKey::Execute;
        case VK_SNAPSHOT:
            return ego::KeyboardInputKey::PrintScreen;
        case VK_INSERT:
            return ego::KeyboardInputKey::Insert;
        case VK_DELETE:
            return ego::KeyboardInputKey::Delete;
        case VK_HELP:
            return ego::KeyboardInputKey::Help;
        case VK_OEM_1:
            return ego::KeyboardInputKey::Semicolon;
        case VK_OEM_PLUS:
            return ego::KeyboardInputKey::Equal;
        case VK_OEM_COMMA:
            return ego::KeyboardInputKey::Comma;
        case VK_OEM_MINUS:
            return ego::KeyboardInputKey::Minus;
        case VK_OEM_PERIOD:
            return ego::KeyboardInputKey::Period;
        case VK_OEM_2:
            return ego::KeyboardInputKey::Slash;
        case VK_OEM_3:
            return ego::KeyboardInputKey::GraveAccent;
        case VK_OEM_4:
            return ego::KeyboardInputKey::LeftBracket;
        case VK_OEM_5:
            return ego::KeyboardInputKey::Backslash;
        case VK_OEM_6:
            return ego::KeyboardInputKey::RightBracket;
        case VK_OEM_7:
            return ego::KeyboardInputKey::Apostrophe;
        case VK_OEM_102:
            return ego::KeyboardInputKey::InternationalBackslash;
        case VK_CAPITAL:
            return ego::KeyboardInputKey::CapsLock;
        case VK_SCROLL:
            return ego::KeyboardInputKey::ScrollLock;
        case VK_NUMLOCK:
            return ego::KeyboardInputKey::NumLock;
        case VK_LSHIFT:
            return ego::KeyboardInputKey::LeftShift;
        case VK_RSHIFT:
            return ego::KeyboardInputKey::RightShift;
        case VK_LCONTROL:
            return ego::KeyboardInputKey::LeftControl;
        case VK_RCONTROL:
            return ego::KeyboardInputKey::RightControl;
        case VK_LMENU:
            return ego::KeyboardInputKey::LeftAlt;
        case VK_RMENU:
            return ego::KeyboardInputKey::RightAlt;
        case VK_LWIN:
            return ego::KeyboardInputKey::LeftSystem;
        case VK_RWIN:
            return ego::KeyboardInputKey::RightSystem;
        case VK_APPS:
            return ego::KeyboardInputKey::Menu;
        case VK_SLEEP:
            return ego::KeyboardInputKey::Sleep;
        case VK_MULTIPLY:
            return ego::KeyboardInputKey::NumpadMultiply;
        case VK_ADD:
            return ego::KeyboardInputKey::NumpadAdd;
        case VK_SEPARATOR:
            return ego::KeyboardInputKey::NumpadSeparator;
        case VK_SUBTRACT:
            return ego::KeyboardInputKey::NumpadSubtract;
        case VK_DECIMAL:
            return ego::KeyboardInputKey::NumpadDecimal;
        case VK_DIVIDE:
            return ego::KeyboardInputKey::NumpadDivide;
        case VK_OEM_NEC_EQUAL:
            return ego::KeyboardInputKey::NumpadEqual;
        case VK_BROWSER_BACK:
            return ego::KeyboardInputKey::BrowserBack;
        case VK_BROWSER_FORWARD:
            return ego::KeyboardInputKey::BrowserForward;
        case VK_BROWSER_REFRESH:
            return ego::KeyboardInputKey::BrowserRefresh;
        case VK_BROWSER_STOP:
            return ego::KeyboardInputKey::BrowserStop;
        case VK_BROWSER_SEARCH:
            return ego::KeyboardInputKey::BrowserSearch;
        case VK_BROWSER_FAVORITES:
            return ego::KeyboardInputKey::BrowserFavorites;
        case VK_BROWSER_HOME:
            return ego::KeyboardInputKey::BrowserHome;
        case VK_VOLUME_MUTE:
            return ego::KeyboardInputKey::VolumeMute;
        case VK_VOLUME_DOWN:
            return ego::KeyboardInputKey::VolumeDown;
        case VK_VOLUME_UP:
            return ego::KeyboardInputKey::VolumeUp;
        case VK_MEDIA_NEXT_TRACK:
            return ego::KeyboardInputKey::MediaNextTrack;
        case VK_MEDIA_PREV_TRACK:
            return ego::KeyboardInputKey::MediaPreviousTrack;
        case VK_MEDIA_STOP:
            return ego::KeyboardInputKey::MediaStop;
        case VK_MEDIA_PLAY_PAUSE:
            return ego::KeyboardInputKey::MediaPlayPause;
        case VK_LAUNCH_MAIL:
            return ego::KeyboardInputKey::LaunchMail;
        case VK_LAUNCH_MEDIA_SELECT:
            return ego::KeyboardInputKey::MediaSelect;
        case VK_LAUNCH_APP1:
            return ego::KeyboardInputKey::LaunchApplication1;
        case VK_LAUNCH_APP2:
            return ego::KeyboardInputKey::LaunchApplication2;
        case VK_KANA:
            return ego::KeyboardInputKey::KanaHangul;
        case VK_IME_ON:
            return ego::KeyboardInputKey::ImeOn;
        case VK_JUNJA:
            return ego::KeyboardInputKey::Junja;
        case VK_FINAL:
            return ego::KeyboardInputKey::Final;
        case VK_HANJA:
            return ego::KeyboardInputKey::HanjaKanji;
        case VK_IME_OFF:
            return ego::KeyboardInputKey::ImeOff;
        case VK_CONVERT:
            return ego::KeyboardInputKey::Convert;
        case VK_NONCONVERT:
            return ego::KeyboardInputKey::NonConvert;
        case VK_ACCEPT:
            return ego::KeyboardInputKey::Accept;
        case VK_MODECHANGE:
            return ego::KeyboardInputKey::ModeChange;
        case VK_PROCESSKEY:
            return ego::KeyboardInputKey::Process;
        case VK_PACKET:
            return ego::KeyboardInputKey::Packet;
        default:
            return ego::KeyboardInputKey::Undefined;
        }
    }
} // namespace

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

bool ego::win32::Win32PlatformSurface::Win32PlatformSurfaceAccessor::OnWindowCloseRequested(Win32PlatformSurface& _surface)
{
    return _surface.onWindowCloseRequested();
}

void ego::win32::Win32PlatformSurface::Win32PlatformSurfaceAccessor::OnWindowDestroyed(Win32PlatformSurface& _surface)
{
    _surface.onWindowDestroyed();
}

void ego::win32::Win32PlatformSurface::Win32PlatformSurfaceAccessor::OnWindowActivate(Win32PlatformSurface& _surface, bool _isActive)
{
    _surface.onWindowActivate(_isActive);
}

void ego::win32::Win32PlatformSurface::Win32PlatformSurfaceAccessor::OnWindowSizeUpdate(Win32PlatformSurface& _surface)
{
    _surface.onWindowSizeUpdate();
}

void ego::win32::Win32PlatformSurface::Win32PlatformSurfaceAccessor::OnWindowPointerCaptureLost(Win32PlatformSurface& _surface)
{
    _surface.onWindowPointerCaptureLost();
}

void ego::win32::Win32PlatformSurface::Win32PlatformSurfaceAccessor::OnWindowKeyboardInput(
    Win32PlatformSurface& _surface,
    InputButtonAction _action,
    WPARAM _wParam,
    LPARAM _lParam)
{
    _surface.onWindowKeyboardInput(_action, _wParam, _lParam);
}

void ego::win32::Win32PlatformSurface::Win32PlatformSurfaceAccessor::OnWindowTextInput(Win32PlatformSurface& _surface, SurfaceTextCodepoint _codepoint)
{
    _surface.onWindowTextInput(_codepoint);
}

ego::win32::Win32PlatformSurface::~Win32PlatformSurface()
{
    release();
}

bool ego::win32::Win32PlatformSurface::init(const PlatformSurfaceDesc& _desc, Win32PlatformSurfaceController& _surfaceController, HINSTANCE _instance)
{
    EGO_CHECK_INITIALIZATION(initEvents());

    const DWORD windowStyle = _desc.m_hasFrame ? WS_OVERLAPPEDWINDOW : WS_POPUP;

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
    if (hasExplicitSize || hasExplicitPosition)
    {
        EGO_CHECK_INITIALIZATION(AdjustWindowRectEx(&windowRect, windowStyle, false, 0));
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

    HWND handle = CreateWindow(
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

    const DWORD style = static_cast<DWORD>(GetWindowLongPtr(handle, GWL_STYLE));
    const DWORD extendedStyle = static_cast<DWORD>(GetWindowLongPtr(handle, GWL_EXSTYLE));
    EGO_CHECK_RETURN_FALSE(AdjustWindowRectEx(&windowRect, style, GetMenu(handle) != nullptr, extendedStyle));

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

    const DWORD style = static_cast<DWORD>(GetWindowLongPtr(handle, GWL_STYLE));
    const DWORD extendedStyle = static_cast<DWORD>(GetWindowLongPtr(handle, GWL_EXSTYLE));
    EGO_CHECK_RETURN_FALSE(AdjustWindowRectEx(&windowRect, style, GetMenu(handle) != nullptr, extendedStyle));

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

bool ego::win32::Win32PlatformSurface::setInputPassthrough(bool _isEnabled)
{
    EGO_CHECK_RETURN_FALSE(m_handle);

    m_isInputPassthrough = _isEnabled;

    return true;
}

bool ego::win32::Win32PlatformSurface::isInputPassthrough() const
{
    return m_isInputPassthrough;
}

bool ego::win32::Win32PlatformSurface::setPointerCapture(bool _isCaptured)
{
    EGO_CHECK_RETURN_FALSE(m_handle);

    WindowData* windowData = GetWindowData(m_handle);
    EGO_CHECK_RETURN_FALSE(windowData);

    SharedPointer<Win32PlatformSurfaceController> surfaceController = windowData->m_controller.lock();
    EGO_CHECK_RETURN_FALSE(surfaceController);

    if (_isCaptured)
    {
        return surfaceController->setPointerCapture(sharedFromThis());
    }

    return !hasPointerCapture() || surfaceController->setPointerCapture(nullptr);
}

bool ego::win32::Win32PlatformSurface::hasPointerCapture() const
{
    return m_handle && GetCapture() == m_handle;
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
    inputData.m_key = ToKeyboardInputKey(_wParam, _lParam);
    inputData.m_isRepeat = _action == InputButtonAction::Pressed && (_lParam & KeyRepeatMask) != 0;
    inputData.m_action = _action;

    notifyKeyboardInput(inputData);
}

void ego::win32::Win32PlatformSurface::onWindowTextInput(SurfaceTextCodepoint _codepoint)
{
    SurfaceTextInput inputData;
    inputData.m_codepoint = _codepoint;

    notifyTextInput(inputData);
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

    m_surfaceSize.m_x = ToWindowExtent(clientWidth);
    m_surfaceSize.m_y = ToWindowExtent(clientHeight);
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
