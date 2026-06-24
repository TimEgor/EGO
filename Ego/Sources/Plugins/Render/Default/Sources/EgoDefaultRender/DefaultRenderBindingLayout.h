#pragma once

#include "EgoEngine/Graphic/Render/RenderGpuObject.h"

namespace ego
{
    class GraphicDevice;
}

namespace ego::render
{
    RenderBindingLayout CreateDefaultRenderBindlessBindingLayout(GraphicDevice& _graphicDevice);
}
