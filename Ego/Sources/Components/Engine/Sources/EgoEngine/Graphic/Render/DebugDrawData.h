#pragma once

#include "EgoMath/Vector.h"

namespace ego::render
{
    struct DebugDrawPointData final
    {
        FloatVector3 m_position = FloatVector3Zero;
        FloatVector4 m_color = FloatVector4One;
    };

    struct DebugDrawLineData final
    {
        DebugDrawPointData m_start;
        DebugDrawPointData m_end;
    };
} // namespace ego::render
