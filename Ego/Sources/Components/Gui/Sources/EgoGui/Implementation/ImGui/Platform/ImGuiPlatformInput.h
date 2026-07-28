#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "EgoCore/Math/Vector.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Platform/Input/KeyboardInputDevice.h"
#include "EgoCore/Platform/Input/MouseInputDevice.h"

#include "EgoGui/Input/InputEventSink.h"

enum ImGuiKey : int;

namespace ego::gui
{
    struct InputModifiers;

    class ImGuiPlatformInput final : public NonCopyable, public InputEventSink
    {
    public:
        void reset();

    private:
        void process(const PointerMoveEvent& _event) override;
        void process(const PointerExitEvent& _event) override;
        void process(const MouseButtonEvent& _event) override;
        void process(const MouseWheelEvent& _event) override;
        void process(const KeyEvent& _event) override;
        void process(const TextInputEvent& _event) override;
        void process(const PointerCaptureLostEvent& _event) override;
        void process(const ViewportDeactivatedEvent& _event) override;

        static ImGuiKey TranslateKeyboardKey(KeyboardInputKey _key);
        static int TranslateMouseButton(MouseInputKey _key);
        static uint32_t NormalizeCodepoint(uint32_t _codepoint);

        void submitModifiers(const InputModifiers& _modifiers);
        void submitPointerPosition(const FloatVector2& _position, const FloatVector2& _screenPosition);
        void releaseMouseButtons();

        static constexpr std::size_t MouseButtonCount = 5;

        std::array<bool, MouseButtonCount> m_mouseButtonsDown{};
    };
} // namespace ego::gui
