#pragma once

#include "EgoGraphicHardware/GraphicObjects/Format.h"

namespace ego::gui::default_gui_render
{
    inline constexpr auto DefaultGuiRenderTargetFormat = gpu::GraphicResourceFormat::R8G8B8A8UNorm;
    inline constexpr auto DefaultGuiRenderConfigPath = "DefaultGuiRender.xml";
    inline constexpr auto GuiVertexShaderPath = "DefaultGuiRender/Shaders/GuiVS.hlsl";
    inline constexpr auto GuiPixelShaderPath = "DefaultGuiRender/Shaders/GuiPS.hlsl";
} // namespace ego::gui::default_gui_render
