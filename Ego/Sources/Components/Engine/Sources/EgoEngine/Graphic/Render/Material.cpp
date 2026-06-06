#include "Material.h"

ego::render::Material::Material(
    const RenderGraphicPipeline& _pipeline,
    ResourceViewCollection _resourceViews,
    SamplerCollection _samplers
)
    : m_pipeline(_pipeline),
      m_resourceViews(std::move(_resourceViews)),
      m_samplers(std::move(_samplers))
{}

const ego::render::RenderGraphicPipeline& ego::render::Material::getPipeline() const
{
    return m_pipeline;
}

const ego::render::Material::ResourceViewCollection& ego::render::Material::getResourceViews() const
{
    return m_resourceViews;
}

const ego::render::Material::SamplerCollection& ego::render::Material::getSamplers() const
{
    return m_samplers;
}
