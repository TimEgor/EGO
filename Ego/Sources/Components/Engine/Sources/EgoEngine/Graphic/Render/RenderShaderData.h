#pragma once

#include <cstdint>

#include "EgoMath/Matrix.h"
#include "EgoMath/Vector.h"

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Shader.h"

namespace ego
{
    inline constexpr uint32_t RenderBindlessRootConstantsOffset = 0;
    inline constexpr uint32_t RenderBindlessRootConstantsSize = sizeof(uint32_t) * 4;
    inline constexpr gpu::ShaderStageFlag RenderBindlessRootConstantsStageFlag = gpu::ShaderStageFlagAllGraphics;

    struct RenderBindlessRootConstants final
    {
        uint32_t m_cameraDataIndex = 0;
        uint32_t m_objectDataIndex = 0;
        uint32_t m_objectIndex = 0;
        uint32_t m_padding = 0;
    };

    struct CameraShaderData final
    {
        FloatMatrix4x4 m_view = FloatMatrix4x4Identity;
        FloatMatrix4x4 m_projection = FloatMatrix4x4Identity;
        FloatMatrix4x4 m_viewProjection = FloatMatrix4x4Identity;
        FloatVector4 m_position = FloatVector4(0.0f, 0.0f, 0.0f, 1.0f);
    };

    struct ObjectShaderData final
    {
        FloatMatrix4x4 m_model = FloatMatrix4x4Identity;
        FloatMatrix4x4 m_modelViewProjection = FloatMatrix4x4Identity;
    };

    static_assert(sizeof(RenderBindlessRootConstants) == RenderBindlessRootConstantsSize);
}
