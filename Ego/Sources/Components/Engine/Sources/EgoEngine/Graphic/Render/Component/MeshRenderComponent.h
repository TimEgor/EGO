#pragma once

#include "EgoECS/Component.h"

#include "EgoEngine/Graphic/Render/Material.h"
#include "EgoEngine/Graphic/Render/Mesh.h"

namespace ego
{
    struct MeshRenderComponent final : public ecs::Component
    {
        MeshRenderComponent() = default;
        MeshRenderComponent(const MeshHandle& _mesh, const MaterialHandle& _material);

        MeshHandle m_mesh = nullptr;
        MaterialHandle m_material = nullptr;
    };
}
