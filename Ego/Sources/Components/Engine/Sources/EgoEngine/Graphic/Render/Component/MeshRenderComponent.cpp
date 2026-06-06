#include "MeshRenderComponent.h"

ego::render::MeshRenderComponent::MeshRenderComponent(
    const RenderMesh& _mesh,
    const RenderMaterial& _material
)
    : m_mesh(_mesh),
      m_material(_material)
{}
