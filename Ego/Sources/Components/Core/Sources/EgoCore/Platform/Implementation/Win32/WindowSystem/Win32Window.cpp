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

void ego::win32::Win32Window::onWindowKeyboardInput(InputButtonAction _action, WPARAM _wParam, LPARAM _lParam)
{
    WindowKeyboardInputData inputData;
    inputData.m_key = ToKeyboardInputKey(_wParam, _lParam);
    inputData.m_isRepeat = _action == InputButtonAction::Pressed && (_lParam & KeyRepeatMask) != 0;
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
