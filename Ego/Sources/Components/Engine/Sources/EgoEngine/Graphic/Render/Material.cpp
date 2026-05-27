#include "Material.h"

ego::Material::Material(
    const gpu::GraphicPipelineReference& _pipeline,
    ResourceViewCollection _resourceViews,
    SamplerCollection _samplers
)
    : m_pipeline(_pipeline),
      m_resourceViews(std::move(_resourceViews)),
      m_samplers(std::move(_samplers))
{}

const ego::gpu::GraphicPipelineReference& ego::Material::getPipeline() const
{
    return m_pipeline;
}

const ego::Material::ResourceViewCollection& ego::Material::getResourceViews() const
{
    return m_resourceViews;
}

const ego::Material::SamplerCollection& ego::Material::getSamplers() const
{
    return m_samplers;
}

ego::MaterialHandle ego::CreateMaterialHandle(const MaterialReference& _material)
{
    return MakeHandle<Material>(_material);
}
