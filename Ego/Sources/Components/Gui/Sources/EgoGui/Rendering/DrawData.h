#pragma once

#include <cstdint>
#include <vector>

#include "EgoCore/Math/Color.h"

#include "EgoGui/Core/Geometry.h"
#include "EgoGui/Rendering/Image.h"

namespace ego::gui
{
    struct Vertex final
    {
        Position m_position = PositionZero;
        FloatVector2 m_uv = FloatVector2Zero;
        NormalizedColorRGBA m_color = NormalizedColorRGBA(NormalizedColorWhite);
    };

    struct DrawCommand final
    {
        Rect m_clipRect;
        ImageID m_imageID = InvalidImageID;
        uint32_t m_firstIndex = 0;
        uint32_t m_indexCount = 0;
        int32_t m_vertexOffset = 0;
    };

    struct DrawData final
    {
        using VertexCollection = std::vector<Vertex>;
        using IndexCollection = std::vector<uint32_t>;
        using CommandCollection = std::vector<DrawCommand>;

        Size m_viewportSize = SizeZero;
        VertexCollection m_vertices;
        IndexCollection m_indices;
        CommandCollection m_commands;

        void clear();
        bool isEmpty() const;
    };
} // namespace ego::gui
