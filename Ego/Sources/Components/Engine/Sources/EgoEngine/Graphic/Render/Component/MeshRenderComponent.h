#pragma once

#include "EgoECS/Component.h"

#include "EgoEngine/Graphic/Render/RenderObject.h"

namespace ego::render
{
    struct MeshRenderComponent final : public ecs::Component
    {
        MeshRenderComponent() = default;
        MeshRenderComponent(const RenderMesh& _mesh, const RenderMaterial& _material);

        RenderMesh m_mesh = nullptr;
        RenderMaterial m_material = nullptr;
    };
} // namespace ego::render
