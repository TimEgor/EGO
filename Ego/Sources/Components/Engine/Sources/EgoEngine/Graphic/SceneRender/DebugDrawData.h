#pragma once

#include "EgoCore/Math/Color.h"

namespace ego::render
{
    struct DebugDrawPointData final
    {
        FloatVector3 m_position = FloatVector3Zero;
        NormalizedColorRGB m_color = NormalizedColorWhite;
    };

    struct DebugDrawLineData final
    {
        DebugDrawPointData m_start;
        DebugDrawPointData m_end;
    };
} // namespace ego::render
