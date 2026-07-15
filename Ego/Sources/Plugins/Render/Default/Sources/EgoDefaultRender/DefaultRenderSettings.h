#pragma once

#include "EgoCore/Math/Vector.h"

namespace ego::render
{
    struct DefaultRenderSettings final
    {
        FloatVector4 m_clearColor = FloatVector4(0.0f, 0.0f, 0.0f, 1.0f);
        bool m_clearEnabled = true;
    };
} // namespace ego::render
