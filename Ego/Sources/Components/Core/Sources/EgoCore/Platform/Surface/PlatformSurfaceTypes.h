#pragma once

#include <cstdint>
#include <limits>
#include <string>

#include "EgoCore/Math/Vector.h"
#include "EgoCore/Platform/Input/InputTypes.h"
#include "EgoCore/Platform/Input/KeyboardInputDevice.h"

namespace ego
{
    using SurfaceSize = UInt16Vector2;
    using SurfacePoint = Int32Vector2;

    inline constexpr SurfaceSize DefaultSurfaceSize = UInt16Vector2Zero;
    inline constexpr SurfacePoint DefaultSurfacePoint = Int32Vector2Zero;
    inline constexpr int32_t AutomaticSurfacePositionCoordinate = (std::numeric_limits<int32_t>::min)();
    inline constexpr SurfacePoint AutomaticSurfacePosition = SurfacePoint(AutomaticSurfacePositionCoordinate, AutomaticSurfacePositionCoordinate);

    enum class PlatformSurfaceWindowState
    {
        Normal,
        Minimized,
        Maximized
    };

    struct SurfaceKeyboardInput final
    {
        KeyboardInputKey m_key = KeyboardInputKey::Undefined;
        bool m_isRepeat = false;
        InputButtonAction m_action = InputButtonAction::Pressed;
    };

    using SurfaceTextCodepoint = uint32_t;

    struct SurfaceTextInput final
    {
        SurfaceTextCodepoint m_codepoint = 0;
    };

    struct PlatformSurfaceDesc final
    {
        std::string m_name;
        SurfaceSize m_size = DefaultSurfaceSize;
        SurfacePoint m_position = AutomaticSurfacePosition;
        bool m_hasFrame = true;
        bool m_isVisible = false;
    };
} // namespace ego
