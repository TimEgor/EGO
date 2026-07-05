#pragma once

#include <cstdint>
#include <vector>

#include "GuiTypes.h"

namespace ego::gui
{
    struct GuiVertex final
    {
        GuiPosition m_position = GuiPositionZero;
        FloatVector2 m_uv = FloatVector2Zero;
        GuiColor m_color = GuiColorWhite;
    };

    struct GuiDrawCommand final
    {
        GuiRect m_clipRect;
        GuiTextureID m_textureId = InvalidGuiTextureID;
        uint32_t m_firstIndex = 0;
        uint32_t m_indexCount = 0;
        int32_t m_vertexOffset = 0;
    };

    struct GuiDrawData final
    {
        using VertexCollection = std::vector<GuiVertex>;
        using IndexCollection = std::vector<uint32_t>;
        using CommandCollection = std::vector<GuiDrawCommand>;

        GuiSize m_viewportSize = GuiSizeZero;
        VertexCollection m_vertices;
        IndexCollection m_indices;
        CommandCollection m_commands;

        void clear();
        bool isEmpty() const;
    };
} // namespace ego::gui
