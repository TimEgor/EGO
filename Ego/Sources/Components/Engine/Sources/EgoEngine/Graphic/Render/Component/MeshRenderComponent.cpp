#include "MeshRenderComponent.h"

ego::MeshRenderComponent::MeshRenderComponent(
    const MeshHandle& _mesh,
    const MaterialHandle& _material
)
    : m_mesh(_mesh)
    , m_material(_material)
{}
