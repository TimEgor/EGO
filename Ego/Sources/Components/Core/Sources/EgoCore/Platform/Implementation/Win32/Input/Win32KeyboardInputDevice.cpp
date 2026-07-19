#include "Win32KeyboardInputDevice.h"

#include <array>
#include <cstddef>

namespace
{
    using KeyboardVirtualKeyCollection = std::array<int, ego::KeyboardInputKeyCount>;

    constexpr std::size_t ToKeyIndex(ego::KeyboardInputKey _key)
    {
        return static_cast<std::size_t>(_key);
    }

    constexpr KeyboardVirtualKeyCollection BuildKeyboardVirtualKeys()
    {
        KeyboardVirtualKeyCollection result = {};

        result[ToKeyIndex(ego::KeyboardInputKey::Cancel)] = VK_CANCEL;
        result[ToKeyIndex(ego::KeyboardInputKey::Backspace)] = VK_BACK;
        result[ToKeyIndex(ego::KeyboardInputKey::Tab)] = VK_TAB;
        result[ToKeyIndex(ego::KeyboardInputKey::Clear)] = VK_CLEAR;
        result[ToKeyIndex(ego::KeyboardInputKey::Enter)] = VK_RETURN;
        result[ToKeyIndex(ego::KeyboardInputKey::Pause)] = VK_PAUSE;
        result[ToKeyIndex(ego::KeyboardInputKey::Escape)] = VK_ESCAPE;
        result[ToKeyIndex(ego::KeyboardInputKey::Space)] = VK_SPACE;
        result[ToKeyIndex(ego::KeyboardInputKey::PageUp)] = VK_PRIOR;
        result[ToKeyIndex(ego::KeyboardInputKey::PageDown)] = VK_NEXT;
        result[ToKeyIndex(ego::KeyboardInputKey::End)] = VK_END;
        result[ToKeyIndex(ego::KeyboardInputKey::Home)] = VK_HOME;
        result[ToKeyIndex(ego::KeyboardInputKey::Left)] = VK_LEFT;
        result[ToKeyIndex(ego::KeyboardInputKey::Up)] = VK_UP;
        result[ToKeyIndex(ego::KeyboardInputKey::Right)] = VK_RIGHT;
        result[ToKeyIndex(ego::KeyboardInputKey::Down)] = VK_DOWN;
        result[ToKeyIndex(ego::KeyboardInputKey::Select)] = VK_SELECT;
        result[ToKeyIndex(ego::KeyboardInputKey::Print)] = VK_PRINT;
        result[ToKeyIndex(ego::KeyboardInputKey::Execute)] = VK_EXECUTE;
        result[ToKeyIndex(ego::KeyboardInputKey::PrintScreen)] = VK_SNAPSHOT;
        result[ToKeyIndex(ego::KeyboardInputKey::Insert)] = VK_INSERT;
        result[ToKeyIndex(ego::KeyboardInputKey::Delete)] = VK_DELETE;
        result[ToKeyIndex(ego::KeyboardInputKey::Help)] = VK_HELP;

        for (std::size_t offset = 0; offset <= ToKeyIndex(ego::KeyboardInputKey::Number9) - ToKeyIndex(ego::KeyboardInputKey::Number0); ++offset)
        {
            result[ToKeyIndex(ego::KeyboardInputKey::Number0) + offset] = '0' + static_cast<int>(offset);
        }

        for (std::size_t offset = 0; offset <= ToKeyIndex(ego::KeyboardInputKey::Z) - ToKeyIndex(ego::KeyboardInputKey::A); ++offset)
        {
            result[ToKeyIndex(ego::KeyboardInputKey::A) + offset] = 'A' + static_cast<int>(offset);
        }

        result[ToKeyIndex(ego::KeyboardInputKey::Semicolon)] = VK_OEM_1;
        result[ToKeyIndex(ego::KeyboardInputKey::Equal)] = VK_OEM_PLUS;
        result[ToKeyIndex(ego::KeyboardInputKey::Comma)] = VK_OEM_COMMA;
        result[ToKeyIndex(ego::KeyboardInputKey::Minus)] = VK_OEM_MINUS;
        result[ToKeyIndex(ego::KeyboardInputKey::Period)] = VK_OEM_PERIOD;
        result[ToKeyIndex(ego::KeyboardInputKey::Slash)] = VK_OEM_2;
        result[ToKeyIndex(ego::KeyboardInputKey::GraveAccent)] = VK_OEM_3;
        result[ToKeyIndex(ego::KeyboardInputKey::LeftBracket)] = VK_OEM_4;
        result[ToKeyIndex(ego::KeyboardInputKey::Backslash)] = VK_OEM_5;
        result[ToKeyIndex(ego::KeyboardInputKey::RightBracket)] = VK_OEM_6;
        result[ToKeyIndex(ego::KeyboardInputKey::Apostrophe)] = VK_OEM_7;
        result[ToKeyIndex(ego::KeyboardInputKey::InternationalBackslash)] = VK_OEM_102;
        result[ToKeyIndex(ego::KeyboardInputKey::CapsLock)] = VK_CAPITAL;
        result[ToKeyIndex(ego::KeyboardInputKey::ScrollLock)] = VK_SCROLL;
        result[ToKeyIndex(ego::KeyboardInputKey::NumLock)] = VK_NUMLOCK;
        result[ToKeyIndex(ego::KeyboardInputKey::LeftShift)] = VK_LSHIFT;
        result[ToKeyIndex(ego::KeyboardInputKey::RightShift)] = VK_RSHIFT;
        result[ToKeyIndex(ego::KeyboardInputKey::LeftControl)] = VK_LCONTROL;
        result[ToKeyIndex(ego::KeyboardInputKey::RightControl)] = VK_RCONTROL;
        result[ToKeyIndex(ego::KeyboardInputKey::LeftAlt)] = VK_LMENU;
        result[ToKeyIndex(ego::KeyboardInputKey::RightAlt)] = VK_RMENU;
        result[ToKeyIndex(ego::KeyboardInputKey::LeftSystem)] = VK_LWIN;
        result[ToKeyIndex(ego::KeyboardInputKey::RightSystem)] = VK_RWIN;
        result[ToKeyIndex(ego::KeyboardInputKey::Menu)] = VK_APPS;
        result[ToKeyIndex(ego::KeyboardInputKey::Sleep)] = VK_SLEEP;

        for (std::size_t offset = 0; offset <= ToKeyIndex(ego::KeyboardInputKey::F24) - ToKeyIndex(ego::KeyboardInputKey::F1); ++offset)
        {
            result[ToKeyIndex(ego::KeyboardInputKey::F1) + offset] = VK_F1 + static_cast<int>(offset);
        }

        for (std::size_t offset = 0; offset <= ToKeyIndex(ego::KeyboardInputKey::Numpad9) - ToKeyIndex(ego::KeyboardInputKey::Numpad0); ++offset)
        {
            result[ToKeyIndex(ego::KeyboardInputKey::Numpad0) + offset] = VK_NUMPAD0 + static_cast<int>(offset);
        }

        result[ToKeyIndex(ego::KeyboardInputKey::NumpadMultiply)] = VK_MULTIPLY;
        result[ToKeyIndex(ego::KeyboardInputKey::NumpadAdd)] = VK_ADD;
        result[ToKeyIndex(ego::KeyboardInputKey::NumpadSeparator)] = VK_SEPARATOR;
        result[ToKeyIndex(ego::KeyboardInputKey::NumpadSubtract)] = VK_SUBTRACT;
        result[ToKeyIndex(ego::KeyboardInputKey::NumpadDecimal)] = VK_DECIMAL;
        result[ToKeyIndex(ego::KeyboardInputKey::NumpadDivide)] = VK_DIVIDE;
        result[ToKeyIndex(ego::KeyboardInputKey::NumpadEnter)] = VK_RETURN;
        result[ToKeyIndex(ego::KeyboardInputKey::NumpadEqual)] = VK_OEM_NEC_EQUAL;
        result[ToKeyIndex(ego::KeyboardInputKey::BrowserBack)] = VK_BROWSER_BACK;
        result[ToKeyIndex(ego::KeyboardInputKey::BrowserForward)] = VK_BROWSER_FORWARD;
        result[ToKeyIndex(ego::KeyboardInputKey::BrowserRefresh)] = VK_BROWSER_REFRESH;
        result[ToKeyIndex(ego::KeyboardInputKey::BrowserStop)] = VK_BROWSER_STOP;
        result[ToKeyIndex(ego::KeyboardInputKey::BrowserSearch)] = VK_BROWSER_SEARCH;
        result[ToKeyIndex(ego::KeyboardInputKey::BrowserFavorites)] = VK_BROWSER_FAVORITES;
        result[ToKeyIndex(ego::KeyboardInputKey::BrowserHome)] = VK_BROWSER_HOME;
        result[ToKeyIndex(ego::KeyboardInputKey::VolumeMute)] = VK_VOLUME_MUTE;
        result[ToKeyIndex(ego::KeyboardInputKey::VolumeDown)] = VK_VOLUME_DOWN;
        result[ToKeyIndex(ego::KeyboardInputKey::VolumeUp)] = VK_VOLUME_UP;
        result[ToKeyIndex(ego::KeyboardInputKey::MediaNextTrack)] = VK_MEDIA_NEXT_TRACK;
        result[ToKeyIndex(ego::KeyboardInputKey::MediaPreviousTrack)] = VK_MEDIA_PREV_TRACK;
        result[ToKeyIndex(ego::KeyboardInputKey::MediaStop)] = VK_MEDIA_STOP;
        result[ToKeyIndex(ego::KeyboardInputKey::MediaPlayPause)] = VK_MEDIA_PLAY_PAUSE;
        result[ToKeyIndex(ego::KeyboardInputKey::LaunchMail)] = VK_LAUNCH_MAIL;
        result[ToKeyIndex(ego::KeyboardInputKey::MediaSelect)] = VK_LAUNCH_MEDIA_SELECT;
        result[ToKeyIndex(ego::KeyboardInputKey::LaunchApplication1)] = VK_LAUNCH_APP1;
        result[ToKeyIndex(ego::KeyboardInputKey::LaunchApplication2)] = VK_LAUNCH_APP2;
        result[ToKeyIndex(ego::KeyboardInputKey::KanaHangul)] = VK_KANA;
        result[ToKeyIndex(ego::KeyboardInputKey::ImeOn)] = VK_IME_ON;
        result[ToKeyIndex(ego::KeyboardInputKey::Junja)] = VK_JUNJA;
        result[ToKeyIndex(ego::KeyboardInputKey::Final)] = VK_FINAL;
        result[ToKeyIndex(ego::KeyboardInputKey::HanjaKanji)] = VK_HANJA;
        result[ToKeyIndex(ego::KeyboardInputKey::ImeOff)] = VK_IME_OFF;
        result[ToKeyIndex(ego::KeyboardInputKey::Convert)] = VK_CONVERT;
        result[ToKeyIndex(ego::KeyboardInputKey::NonConvert)] = VK_NONCONVERT;
        result[ToKeyIndex(ego::KeyboardInputKey::Accept)] = VK_ACCEPT;
        result[ToKeyIndex(ego::KeyboardInputKey::ModeChange)] = VK_MODECHANGE;
        result[ToKeyIndex(ego::KeyboardInputKey::Process)] = VK_PROCESSKEY;
        result[ToKeyIndex(ego::KeyboardInputKey::Packet)] = VK_PACKET;

        return result;
    }

    constexpr KeyboardVirtualKeyCollection KeyboardVirtualKeys = BuildKeyboardVirtualKeys();

    constexpr bool HasAllKeyboardVirtualKeys(const KeyboardVirtualKeyCollection& _virtualKeys)
    {
        for (const int virtualKey : _virtualKeys)
        {
            if (virtualKey == 0)
            {
                return false;
            }
        }

        return true;
    }

    static_assert(HasAllKeyboardVirtualKeys(KeyboardVirtualKeys));

    ego::InputDeviceKeyValue GetButtonValue(int _virtualKey)
    {
        const SHORT keyState = GetAsyncKeyState(_virtualKey);
        return (keyState & 0x8000) ? ego::MaxInputDeviceFixedKeyValue : ego::MinInputDeviceKeyValue;
    }
} // namespace

void ego::win32::Win32KeyboardInputDevice::update()
{
    for (InputDeviceKey keyIndex = 0; keyIndex < KeyboardInputKeyCount; ++keyIndex)
    {
        setValue(keyIndex, GetButtonValue(KeyboardVirtualKeys[keyIndex]));
    }
}
