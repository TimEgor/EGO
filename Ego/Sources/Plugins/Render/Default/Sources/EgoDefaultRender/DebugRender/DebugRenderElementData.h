#pragma once

#include <cstdint>
#include <vector>

#include "EgoCore/Math/Color.h"

#include "EgoEngine/Graphic/SceneRender/MaterialRenderPassInfo.h"

#include "DebugElementBufferPool.h"

namespace ego::render
{
    struct DebugPointRenderData final
    {
        struct PointData final
        {
            FloatVector3 m_position = FloatVector3Zero;
            FloatVector3 m_color = FloatVector3One;

            PointData() = default;

            PointData(const FloatVector3& _position, const NormalizedColorRGB& _color)
                : m_position(_position),
                  m_color(static_cast<float>(_color.getR()), static_cast<float>(_color.getG()), static_cast<float>(_color.getB()))
            {
            }
        };

        using PointCollection = std::vector<PointData>;

        RasterizationMaterialRenderPassInfoPointer m_materialInfo = nullptr;
        DebugElementBufferPool m_pointData;
        PointCollection m_pointCommands;
        uint32_t m_pointCount = 0;
    };

    struct DebugLineRenderData final
    {
        struct VertexData final
        {
            FloatVector3 m_position = FloatVector3Zero;
            FloatVector3 m_color = FloatVector3One;

            VertexData() = default;

            VertexData(const FloatVector3& _position, const NormalizedColorRGB& _color)
                : m_position(_position),
                  m_color(static_cast<float>(_color.getR()), static_cast<float>(_color.getG()), static_cast<float>(_color.getB()))
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

        RasterizationMaterialRenderPassInfoPointer m_materialInfo = nullptr;
        DebugElementBufferPool m_lineData;
        LineCollection m_lineCommands;
        uint32_t m_lineCount = 0;
    };
} // namespace ego::render
