#pragma once

#include "EgoCore/FileName/FileName.h"

#include "EgoGraphicHardware/GraphicObjects/Format.h"

namespace ego::gui::default_gui_render
{
    inline constexpr auto DefaultGuiRenderTargetFormat = gpu::GraphicResourceFormat::R8G8B8A8UNorm;
    inline const FileName DefaultGuiRenderConfigPath("DefaultGuiRender.xml");
    inline const FileName GuiVertexShaderPath("DefaultGuiRender/Shaders/GuiVS.hlsl");
    inline const FileName GuiPixelShaderPath("DefaultGuiRender/Shaders/GuiPS.hlsl");
} // namespace ego::gui::default_gui_render
