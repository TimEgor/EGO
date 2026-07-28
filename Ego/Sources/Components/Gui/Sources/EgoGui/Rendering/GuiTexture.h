#pragma once

#include <cstdint>

namespace ego::gui
{
    using GuiFrameTextureID = uint64_t;
    inline constexpr GuiFrameTextureID InvalidGuiFrameTextureID = 0;

    enum class TextureSamplingMode : uint32_t
    {
        Alpha,
        Color,
        AlphaRGBA
    };
} // namespace ego::gui
