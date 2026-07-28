#include "ImGuiPlatformInput.h"

#include <limits>

#include "EgoGui/Input/InputEvents.h"

#include <imgui.h>

namespace
{
    inline constexpr std::array KeyboardKeyMap = {
        ImGuiKey_None,           // Cancel
        ImGuiKey_Backspace,      // Backspace
        ImGuiKey_Tab,            // Tab
        ImGuiKey_None,           // Clear
        ImGuiKey_Enter,          // Enter
        ImGuiKey_Pause,          // Pause
        ImGuiKey_Escape,         // Escape
        ImGuiKey_Space,          // Space
        ImGuiKey_PageUp,         // PageUp
        ImGuiKey_PageDown,       // PageDown
        ImGuiKey_End,            // End
        ImGuiKey_Home,           // Home
        ImGuiKey_LeftArrow,      // Left
        ImGuiKey_UpArrow,        // Up
        ImGuiKey_RightArrow,     // Right
        ImGuiKey_DownArrow,      // Down
        ImGuiKey_None,           // Select
        ImGuiKey_None,           // Print
        ImGuiKey_None,           // Execute
        ImGuiKey_PrintScreen,    // PrintScreen
        ImGuiKey_Insert,         // Insert
        ImGuiKey_Delete,         // Delete
        ImGuiKey_None,           // Help
        ImGuiKey_0,              // Number0
        ImGuiKey_1,              // Number1
        ImGuiKey_2,              // Number2
        ImGuiKey_3,              // Number3
        ImGuiKey_4,              // Number4
        ImGuiKey_5,              // Number5
        ImGuiKey_6,              // Number6
        ImGuiKey_7,              // Number7
        ImGuiKey_8,              // Number8
        ImGuiKey_9,              // Number9
        ImGuiKey_A,              // A
        ImGuiKey_B,              // B
        ImGuiKey_C,              // C
        ImGuiKey_D,              // D
        ImGuiKey_E,              // E
        ImGuiKey_F,              // F
        ImGuiKey_G,              // G
        ImGuiKey_H,              // H
        ImGuiKey_I,              // I
        ImGuiKey_J,              // J
        ImGuiKey_K,              // K
        ImGuiKey_L,              // L
        ImGuiKey_M,              // M
        ImGuiKey_N,              // N
        ImGuiKey_O,              // O
        ImGuiKey_P,              // P
        ImGuiKey_Q,              // Q
        ImGuiKey_R,              // R
        ImGuiKey_S,              // S
        ImGuiKey_T,              // T
        ImGuiKey_U,              // U
        ImGuiKey_V,              // V
        ImGuiKey_W,              // W
        ImGuiKey_X,              // X
        ImGuiKey_Y,              // Y
        ImGuiKey_Z,              // Z
        ImGuiKey_Semicolon,      // Semicolon
        ImGuiKey_Equal,          // Equal
        ImGuiKey_Comma,          // Comma
        ImGuiKey_Minus,          // Minus
        ImGuiKey_Period,         // Period
        ImGuiKey_Slash,          // Slash
        ImGuiKey_GraveAccent,    // GraveAccent
        ImGuiKey_LeftBracket,    // LeftBracket
        ImGuiKey_Backslash,      // Backslash
        ImGuiKey_RightBracket,   // RightBracket
        ImGuiKey_Apostrophe,     // Apostrophe
        ImGuiKey_Oem102,         // InternationalBackslash
        ImGuiKey_CapsLock,       // CapsLock
        ImGuiKey_ScrollLock,     // ScrollLock
        ImGuiKey_NumLock,        // NumLock
        ImGuiKey_LeftShift,      // LeftShift
        ImGuiKey_RightShift,     // RightShift
        ImGuiKey_LeftCtrl,       // LeftControl
        ImGuiKey_RightCtrl,      // RightControl
        ImGuiKey_LeftAlt,        // LeftAlt
        ImGuiKey_RightAlt,       // RightAlt
        ImGuiKey_LeftSuper,      // LeftSystem
        ImGuiKey_RightSuper,     // RightSystem
        ImGuiKey_Menu,           // Menu
        ImGuiKey_None,           // Sleep
        ImGuiKey_F1,             // F1
        ImGuiKey_F2,             // F2
        ImGuiKey_F3,             // F3
        ImGuiKey_F4,             // F4
        ImGuiKey_F5,             // F5
        ImGuiKey_F6,             // F6
        ImGuiKey_F7,             // F7
        ImGuiKey_F8,             // F8
        ImGuiKey_F9,             // F9
        ImGuiKey_F10,            // F10
        ImGuiKey_F11,            // F11
        ImGuiKey_F12,            // F12
        ImGuiKey_F13,            // F13
        ImGuiKey_F14,            // F14
        ImGuiKey_F15,            // F15
        ImGuiKey_F16,            // F16
        ImGuiKey_F17,            // F17
        ImGuiKey_F18,            // F18
        ImGuiKey_F19,            // F19
        ImGuiKey_F20,            // F20
        ImGuiKey_F21,            // F21
        ImGuiKey_F22,            // F22
        ImGuiKey_F23,            // F23
        ImGuiKey_F24,            // F24
        ImGuiKey_Keypad0,        // Numpad0
        ImGuiKey_Keypad1,        // Numpad1
        ImGuiKey_Keypad2,        // Numpad2
        ImGuiKey_Keypad3,        // Numpad3
        ImGuiKey_Keypad4,        // Numpad4
        ImGuiKey_Keypad5,        // Numpad5
        ImGuiKey_Keypad6,        // Numpad6
        ImGuiKey_Keypad7,        // Numpad7
        ImGuiKey_Keypad8,        // Numpad8
        ImGuiKey_Keypad9,        // Numpad9
        ImGuiKey_KeypadMultiply, // NumpadMultiply
        ImGuiKey_KeypadAdd,      // NumpadAdd
        ImGuiKey_None,           // NumpadSeparator
        ImGuiKey_KeypadSubtract, // NumpadSubtract
        ImGuiKey_KeypadDecimal,  // NumpadDecimal
        ImGuiKey_KeypadDivide,   // NumpadDivide
        ImGuiKey_KeypadEnter,    // NumpadEnter
        ImGuiKey_KeypadEqual,    // NumpadEqual
        ImGuiKey_AppBack,        // BrowserBack
        ImGuiKey_AppForward,     // BrowserForward
        ImGuiKey_None,           // BrowserRefresh
        ImGuiKey_None,           // BrowserStop
        ImGuiKey_None,           // BrowserSearch
        ImGuiKey_None,           // BrowserFavorites
        ImGuiKey_None,           // BrowserHome
        ImGuiKey_None,           // VolumeMute
        ImGuiKey_None,           // VolumeDown
        ImGuiKey_None,           // VolumeUp
        ImGuiKey_None,           // MediaNextTrack
        ImGuiKey_None,           // MediaPreviousTrack
        ImGuiKey_None,           // MediaStop
        ImGuiKey_None,           // MediaPlayPause
        ImGuiKey_None,           // LaunchMail
        ImGuiKey_None,           // MediaSelect
        ImGuiKey_None,           // LaunchApplication1
        ImGuiKey_None,           // LaunchApplication2
        ImGuiKey_None,           // KanaHangul
        ImGuiKey_None,           // ImeOn
        ImGuiKey_None,           // Junja
        ImGuiKey_None,           // Final
        ImGuiKey_None,           // HanjaKanji
        ImGuiKey_None,           // ImeOff
        ImGuiKey_None,           // Convert
        ImGuiKey_None,           // NonConvert
        ImGuiKey_None,           // Accept
        ImGuiKey_None,           // ModeChange
        ImGuiKey_None,           // Process
        ImGuiKey_None,           // Packet
    };

    static_assert(KeyboardKeyMap.size() == ego::KeyboardInputKeyCount);
} // namespace

void ego::gui::ImGuiPlatformInput::reset()
{
    m_mouseButtonsDown.fill(false);
}

void ego::gui::ImGuiPlatformInput::process(const PointerMoveEvent& _event)
{
    submitModifiers(_event.m_modifiers);
    submitPointerPosition(_event.m_position, _event.m_screenPosition);
}

void ego::gui::ImGuiPlatformInput::process(const PointerExitEvent& _event)
{
    submitModifiers(_event.m_modifiers);

    ImGuiIO& io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
    {
        submitPointerPosition(_event.m_position, _event.m_screenPosition);

        return;
    }

    const float invalidMousePosition = -std::numeric_limits<float>::max();
    io.AddMousePosEvent(invalidMousePosition, invalidMousePosition);
}

void ego::gui::ImGuiPlatformInput::process(const MouseButtonEvent& _event)
{
    submitModifiers(_event.m_modifiers);
    submitPointerPosition(_event.m_position, _event.m_screenPosition);

    const int button = TranslateMouseButton(_event.m_key);
    if (button < 0)
    {
        return;
    }

    const bool isPressed = _event.m_action == InputButtonAction::Pressed;
    m_mouseButtonsDown[static_cast<std::size_t>(button)] = isPressed;
    ImGui::GetIO().AddMouseButtonEvent(button, isPressed);
}

void ego::gui::ImGuiPlatformInput::process(const MouseWheelEvent& _event)
{
    submitModifiers(_event.m_modifiers);
    submitPointerPosition(_event.m_position, _event.m_screenPosition);
    ImGui::GetIO().AddMouseWheelEvent(0.0f, _event.m_wheelDelta);
}

void ego::gui::ImGuiPlatformInput::process(const KeyEvent& _event)
{
    submitModifiers(_event.m_modifiers);

    const ImGuiKey key = TranslateKeyboardKey(_event.m_key);
    if (key == ImGuiKey_None)
    {
        return;
    }

    ImGui::GetIO().AddKeyEvent(key, _event.m_action == InputButtonAction::Pressed);
}

void ego::gui::ImGuiPlatformInput::process(const TextInputEvent& _event)
{
    submitModifiers(_event.m_modifiers);

    const uint32_t codepoint = NormalizeCodepoint(_event.m_codepoint);
    if (codepoint == 0)
    {
        return;
    }

    ImGui::GetIO().AddInputCharacter(codepoint);
}

void ego::gui::ImGuiPlatformInput::process(const PointerCaptureLostEvent&)
{
    releaseMouseButtons();
}

void ego::gui::ImGuiPlatformInput::process(const ViewportDeactivatedEvent&)
{
    ImGui::GetIO().AddFocusEvent(false);
    reset();
}

ImGuiKey ego::gui::ImGuiPlatformInput::TranslateKeyboardKey(KeyboardInputKey _key)
{
    const std::size_t keyIndex = static_cast<std::size_t>(_key);
    if (keyIndex >= KeyboardKeyMap.size())
    {
        return ImGuiKey_None;
    }

    return KeyboardKeyMap[keyIndex];
}

int ego::gui::ImGuiPlatformInput::TranslateMouseButton(MouseInputKey _key)
{
    switch (_key)
    {
    case MouseInputKey::ButtonLeft:
        return ImGuiMouseButton_Left;
    case MouseInputKey::ButtonRight:
        return ImGuiMouseButton_Right;
    case MouseInputKey::ButtonMiddle:
        return ImGuiMouseButton_Middle;
    case MouseInputKey::ButtonFour:
        return 3;
    case MouseInputKey::ButtonFive:
        return 4;
    case MouseInputKey::AxisX:
    case MouseInputKey::AxisY:
    case MouseInputKey::Wheel:
    case MouseInputKey::ButtonSix:
    case MouseInputKey::ButtonSeven:
    case MouseInputKey::ButtonEight:
    case MouseInputKey::Count:
        return -1;
    }

    return -1;
}

uint32_t ego::gui::ImGuiPlatformInput::NormalizeCodepoint(uint32_t _codepoint)
{
    constexpr uint32_t FirstSurrogate = 0xD800;
    constexpr uint32_t LastSurrogate = 0xDFFF;
    constexpr uint32_t MaximumCodepoint = 0x10FFFF;
    constexpr uint32_t ReplacementCodepoint = 0xFFFD;

    if (_codepoint > MaximumCodepoint || (_codepoint >= FirstSurrogate && _codepoint <= LastSurrogate))
    {
        return ReplacementCodepoint;
    }

    return _codepoint;
}

void ego::gui::ImGuiPlatformInput::submitModifiers(const InputModifiers& _modifiers)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl, _modifiers.m_control);
    io.AddKeyEvent(ImGuiMod_Shift, _modifiers.m_shift);
    io.AddKeyEvent(ImGuiMod_Alt, _modifiers.m_alt);
    io.AddKeyEvent(ImGuiMod_Super, _modifiers.m_super);
}

void ego::gui::ImGuiPlatformInput::submitPointerPosition(const FloatVector2& _position, const FloatVector2& _screenPosition)
{
    ImGuiIO& io = ImGui::GetIO();
    const FloatVector2& pointerPosition = (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0 ? _screenPosition : _position;
    io.AddMousePosEvent(pointerPosition.m_x, pointerPosition.m_y);
}

void ego::gui::ImGuiPlatformInput::releaseMouseButtons()
{
    for (std::size_t button = 0; button < m_mouseButtonsDown.size(); ++button)
    {
        if (!m_mouseButtonsDown[button])
        {
            continue;
        }

        ImGui::GetIO().AddMouseButtonEvent(static_cast<int>(button), false);
        m_mouseButtonsDown[button] = false;
    }
}
