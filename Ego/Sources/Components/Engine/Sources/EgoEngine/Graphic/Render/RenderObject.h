#pragma once

#include "EgoEngine/Graphic/Render/Material.h"
#include "EgoEngine/Graphic/Render/Mesh.h"
#include "EgoEngine/Graphic/Render/RenderGpuObject.h"
#include "EgoEngine/Graphic/Render/RenderResourceObject.h"

namespace ego::render
{
    // Render domain objects.
    using RenderMaterial = MaterialHandler;
    using RenderMesh = MeshHandler;
} // namespace ego::render
