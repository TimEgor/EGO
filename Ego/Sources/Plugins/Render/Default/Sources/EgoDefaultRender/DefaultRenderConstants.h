#pragma once

#include <cstdint>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Math/Vector.h"

#include "EgoGraphicHardware/GraphicObjects/Format.h"

namespace ego::render
{
    inline constexpr auto DefaultRenderTargetFormat = gpu::GraphicResourceFormat::R8G8B8A8UNorm;
    inline constexpr uint32_t DefaultRenderConstantBufferAlignment = 256;
    inline constexpr uint32_t DefaultRenderVertexStride = sizeof(float) * 7;
    inline constexpr uint32_t DefaultRenderRayTracingPayloadSize = sizeof(FloatVector3);
    inline constexpr uint32_t DefaultRenderRayTracingAttributeSize = sizeof(FloatVector2);
    inline const FileName DefaultRenderConfigPath("DefaultRender.xml");
} // namespace ego::render
