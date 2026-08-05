#pragma once

#include "EgoCore/RTTI/RTTI.h"

#include "EgoECS/Component.h"

#include "EgoEngine/Graphic/SceneRender/RenderObject.h"

namespace ego::render
{
    struct MeshRenderComponent final : public ecs::Component
    {
        MeshRenderComponent() = default;
        MeshRenderComponent(const RenderMesh& _mesh, const RenderMaterial& _material);

        RenderMesh m_mesh = nullptr;
        RenderMaterial m_material = nullptr;

        EGO_RTTI_VIRTUAL(MeshRenderComponent, ecs::Component);
    };
} // namespace ego::render
