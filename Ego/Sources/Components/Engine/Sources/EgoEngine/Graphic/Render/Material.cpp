#include "Material.h"

ego::render::Material::Material(const RenderGraphicPipeline& _pipeline)
    : m_pipeline(_pipeline)
{}

const ego::render::RenderGraphicPipeline& ego::render::Material::getPipeline() const
{
    return m_pipeline;
}
