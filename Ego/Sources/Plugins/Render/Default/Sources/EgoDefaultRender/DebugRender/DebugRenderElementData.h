#pragma once

#include <cstdint>
#include <vector>

#include "EgoMath/Vector.h"

#include "EgoEngine/Graphic/Render/RenderObject.h"

#include "DebugElementBufferPool.h"

namespace ego::render
{
    struct DebugPointRenderData final
    {
        struct PointData final
        {
            FloatVector3 m_position = FloatVector3Zero;
            FloatVector4 m_color = FloatVector4One;

            PointData() = default;

            PointData(const FloatVector3& _position, const FloatVector4& _color)
                : m_position(_position),
                  m_color(_color)
            {
            }
        };

        using PointCollection = std::vector<PointData>;

        RenderGraphicPipeline m_pipeline = nullptr;
        DebugElementBufferPool m_pointData;
        PointCollection m_pointCommands;
        uint32_t m_pointCount = 0;
    };

    struct DebugLineRenderData final
    {
        struct VertexData final
        {
            FloatVector3 m_position = FloatVector3Zero;
            FloatVector4 m_color = FloatVector4One;

            VertexData() = default;

            VertexData(const FloatVector3& _position, const FloatVector4& _color)
                : m_position(_position),
                  m_color(_color)
            {
            }
        };

        struct LineData final
        {
            VertexData m_start;
            VertexData m_end;

            LineData() = default;

            LineData(const VertexData& _start, const VertexData& _end)
                : m_start(_start),
                  m_end(_end)
            {
            }
        };

        using LineCollection = std::vector<LineData>;

        RenderGraphicPipeline m_pipeline = nullptr;
        DebugElementBufferPool m_lineData;
        LineCollection m_lineCommands;
        uint32_t m_lineCount = 0;
    };
} // namespace ego::render
