#pragma once

#include <cstdint>
#include <vector>

#include "EgoCore/Math/Color.h"
#include "EgoCore/Math/Vector.h"

#include "EgoGraphicHardware/GraphicObjects/GraphicObject.h"

#include "GuiTexture.h"

namespace ego::gui
{
    enum class TextureFilteringMode : uint32_t
    {
        Linear,
        Nearest
    };

    struct Vertex final
    {
        FloatVector2 m_position = FloatVector2Zero;
        FloatVector2 m_uv = FloatVector2Zero;
        NormalizedColorRGBA m_color = NormalizedColorRGBA(NormalizedColorWhite);
    };

    struct DrawCommand final
    {
        FloatVector4 m_clipRect = FloatVector4Zero;
        uint32_t m_textureIndex = gpu::InvalidBindlessIndex;
        TextureSamplingMode m_textureSamplingMode = TextureSamplingMode::Alpha;
        TextureFilteringMode m_textureFilteringMode = TextureFilteringMode::Linear;
        uint32_t m_firstIndex = 0;
        uint32_t m_indexCount = 0;
        int32_t m_vertexOffset = 0;
    };

    struct DrawData final
    {
        using VertexCollection = std::vector<Vertex>;
        using IndexCollection = std::vector<uint32_t>;
        using CommandCollection = std::vector<DrawCommand>;

        FloatVector2 m_viewportSize = FloatVector2Zero;
        FloatVector2 m_framebufferScale = FloatVector2One;
        VertexCollection m_vertices;
        IndexCollection m_indices;
        CommandCollection m_commands;

        bool isEmpty() const;
    };
} // namespace ego::gui
