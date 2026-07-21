#pragma once

#include "EgoEngine/Graphic/SceneRender/Material.h"
#include "EgoEngine/Graphic/SceneRender/Mesh.h"
#include "EgoEngine/Graphic/SceneRender/RenderGpuObject.h"
#include "EgoEngine/Graphic/SceneRender/RenderResourceObject.h"

namespace ego::render
{
    // Render domain objects.
    using RenderMaterial = MaterialHandler;
    using RenderMesh = MeshHandler;
} // namespace ego::render
