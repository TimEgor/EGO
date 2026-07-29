#include "Win32SurfaceUtils.h"

ego::KeyboardInputKey ego::win32::Win32SurfaceUtils::ToKeyboardKey(WPARAM _key, LPARAM _lParam)
{
    if (_key >= '0' && _key <= '9')
    {
        return OffsetKeyboardKey(KeyboardInputKey::Number0, _key - '0');
    }

    if (_key >= 'A' && _key <= 'Z')
    {
        return OffsetKeyboardKey(KeyboardInputKey::A, _key - 'A');
    }

    if (_key >= VK_F1 && _key <= VK_F24)
    {
        return OffsetKeyboardKey(KeyboardInputKey::F1, _key - VK_F1);
    }

    if (_key >= VK_NUMPAD0 && _key <= VK_NUMPAD9)
    {
        return OffsetKeyboardKey(KeyboardInputKey::Numpad0, _key - VK_NUMPAD0);
    }

    if (_key == VK_RETURN)
    {
        return IsExtendedKey(_lParam) ? KeyboardInputKey::NumpadEnter : KeyboardInputKey::Enter;
    }

    _key = ResolveModifierKey(_key, _lParam);
    switch (_key)
    {
    case VK_CANCEL:
        return KeyboardInputKey::Cancel;
    case VK_BACK:
        return KeyboardInputKey::Backspace;
    case VK_TAB:
        return KeyboardInputKey::Tab;
    case VK_CLEAR:
        return KeyboardInputKey::Clear;
    case VK_PAUSE:
        return KeyboardInputKey::Pause;
    case VK_ESCAPE:
        return KeyboardInputKey::Escape;
    case VK_SPACE:
        return KeyboardInputKey::Space;
    case VK_PRIOR:
        return KeyboardInputKey::PageUp;
    case VK_NEXT:
        return KeyboardInputKey::PageDown;
    case VK_END:
        return KeyboardInputKey::End;
    case VK_HOME:
        return KeyboardInputKey::Home;
    case VK_LEFT:
        return KeyboardInputKey::Left;
    case VK_UP:
        return KeyboardInputKey::Up;
    case VK_RIGHT:
        return KeyboardInputKey::Right;
    case VK_DOWN:
        return KeyboardInputKey::Down;
    case VK_SELECT:
        return KeyboardInputKey::Select;
    case VK_PRINT:
        return KeyboardInputKey::Print;
    case VK_EXECUTE:
        return KeyboardInputKey::Execute;
    case VK_SNAPSHOT:
        return KeyboardInputKey::PrintScreen;
    case VK_INSERT:
        return KeyboardInputKey::Insert;
    case VK_DELETE:
        return KeyboardInputKey::Delete;
    case VK_HELP:
        return KeyboardInputKey::Help;
    case VK_OEM_1:
        return KeyboardInputKey::Semicolon;
    case VK_OEM_PLUS:
        return KeyboardInputKey::Equal;
    case VK_OEM_COMMA:
        return KeyboardInputKey::Comma;
    case VK_OEM_MINUS:
        return KeyboardInputKey::Minus;
    case VK_OEM_PERIOD:
        return KeyboardInputKey::Period;
    case VK_OEM_2:
        return KeyboardInputKey::Slash;
    case VK_OEM_3:
        return KeyboardInputKey::GraveAccent;
    case VK_OEM_4:
        return KeyboardInputKey::LeftBracket;
    case VK_OEM_5:
        return KeyboardInputKey::Backslash;
    case VK_OEM_6:
        return KeyboardInputKey::RightBracket;
    case VK_OEM_7:
        return KeyboardInputKey::Apostrophe;
    case VK_OEM_102:
        return KeyboardInputKey::InternationalBackslash;
    case VK_CAPITAL:
        return KeyboardInputKey::CapsLock;
    case VK_SCROLL:
        return KeyboardInputKey::ScrollLock;
    case VK_NUMLOCK:
        return KeyboardInputKey::NumLock;
    case VK_LSHIFT:
        return KeyboardInputKey::LeftShift;
    case VK_RSHIFT:
        return KeyboardInputKey::RightShift;
    case VK_LCONTROL:
        return KeyboardInputKey::LeftControl;
    case VK_RCONTROL:
        return KeyboardInputKey::RightControl;
    case VK_LMENU:
        return KeyboardInputKey::LeftAlt;
    case VK_RMENU:
        return KeyboardInputKey::RightAlt;
    case VK_LWIN:
        return KeyboardInputKey::LeftSystem;
    case VK_RWIN:
        return KeyboardInputKey::RightSystem;
    case VK_APPS:
        return KeyboardInputKey::Menu;
    case VK_SLEEP:
        return KeyboardInputKey::Sleep;
    case VK_MULTIPLY:
        return KeyboardInputKey::NumpadMultiply;
    case VK_ADD:
        return KeyboardInputKey::NumpadAdd;
    case VK_SEPARATOR:
        return KeyboardInputKey::NumpadSeparator;
    case VK_SUBTRACT:
        return KeyboardInputKey::NumpadSubtract;
    case VK_DECIMAL:
        return KeyboardInputKey::NumpadDecimal;
    case VK_DIVIDE:
        return KeyboardInputKey::NumpadDivide;
    case VK_OEM_NEC_EQUAL:
        return KeyboardInputKey::NumpadEqual;
    case VK_BROWSER_BACK:
        return KeyboardInputKey::BrowserBack;
    case VK_BROWSER_FORWARD:
        return KeyboardInputKey::BrowserForward;
    case VK_BROWSER_REFRESH:
        return KeyboardInputKey::BrowserRefresh;
    case VK_BROWSER_STOP:
        return KeyboardInputKey::BrowserStop;
    case VK_BROWSER_SEARCH:
        return KeyboardInputKey::BrowserSearch;
    case VK_BROWSER_FAVORITES:
        return KeyboardInputKey::BrowserFavorites;
    case VK_BROWSER_HOME:
        return KeyboardInputKey::BrowserHome;
    case VK_VOLUME_MUTE:
        return KeyboardInputKey::VolumeMute;
    case VK_VOLUME_DOWN:
        return KeyboardInputKey::VolumeDown;
    case VK_VOLUME_UP:
        return KeyboardInputKey::VolumeUp;
    case VK_MEDIA_NEXT_TRACK:
        return KeyboardInputKey::MediaNextTrack;
    case VK_MEDIA_PREV_TRACK:
        return KeyboardInputKey::MediaPreviousTrack;
    case VK_MEDIA_STOP:
        return KeyboardInputKey::MediaStop;
    case VK_MEDIA_PLAY_PAUSE:
        return KeyboardInputKey::MediaPlayPause;
    case VK_LAUNCH_MAIL:
        return KeyboardInputKey::LaunchMail;
    case VK_LAUNCH_MEDIA_SELECT:
        return KeyboardInputKey::MediaSelect;
    case VK_LAUNCH_APP1:
        return KeyboardInputKey::LaunchApplication1;
    case VK_LAUNCH_APP2:
        return KeyboardInputKey::LaunchApplication2;
    case VK_KANA:
        return KeyboardInputKey::KanaHangul;
    case VK_IME_ON:
        return KeyboardInputKey::ImeOn;
    case VK_JUNJA:
        return KeyboardInputKey::Junja;
    case VK_FINAL:
        return KeyboardInputKey::Final;
    case VK_HANJA:
        return KeyboardInputKey::HanjaKanji;
    case VK_IME_OFF:
        return KeyboardInputKey::ImeOff;
    case VK_CONVERT:
        return KeyboardInputKey::Convert;
    case VK_NONCONVERT:
        return KeyboardInputKey::NonConvert;
    case VK_ACCEPT:
        return KeyboardInputKey::Accept;
    case VK_MODECHANGE:
        return KeyboardInputKey::ModeChange;
    case VK_PROCESSKEY:
        return KeyboardInputKey::Process;
    case VK_PACKET:
        return KeyboardInputKey::Packet;
    default:
        return KeyboardInputKey::Undefined;
    }
}

bool ego::win32::Win32SurfaceUtils::IsRepeatedKey(LPARAM _lParam)
{
    return (_lParam & KeyRepeatMask) != 0;
}

ego::KeyboardInputKey ego::win32::Win32SurfaceUtils::OffsetKeyboardKey(KeyboardInputKey _firstKey, WPARAM _offset)
{
    return static_cast<KeyboardInputKey>(static_cast<InputDeviceKey>(_firstKey) + static_cast<InputDeviceKey>(_offset));
}

bool ego::win32::Win32SurfaceUtils::IsExtendedKey(LPARAM _lParam)
{
    return (_lParam & KeyExtendedMask) != 0;
}

WPARAM ego::win32::Win32SurfaceUtils::ResolveModifierKey(WPARAM _key, LPARAM _lParam)
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
        return IsExtendedKey(_lParam) ? VK_RCONTROL : VK_LCONTROL;
    case VK_MENU:
        return IsExtendedKey(_lParam) ? VK_RMENU : VK_LMENU;
    default:
        return _key;
    }
}
