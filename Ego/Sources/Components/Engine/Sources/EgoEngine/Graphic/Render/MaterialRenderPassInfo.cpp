#include "MaterialRenderPassInfo.h"

bool ego::render::RayTracingMaterialHitGroup::operator==(const RayTracingMaterialHitGroup& _other) const
{
    return m_closestHitShader.getObject() == _other.m_closestHitShader.getObject() && m_anyHitShader.getObject() == _other.m_anyHitShader.getObject() &&
           m_intersectionShader.getObject() == _other.m_intersectionShader.getObject();
}

bool ego::render::RayTracingMaterialHitGroup::operator!=(const RayTracingMaterialHitGroup& _other) const
{
    return !(*this == _other);
}

ego::render::RasterizationMaterialRenderPassInfo::RasterizationMaterialRenderPassInfo(const RenderVertexShader& _vertexShader, const RenderPixelShader& _pixelShader)
    : m_vertexShader(_vertexShader),
      m_pixelShader(_pixelShader)
{
}

const ego::render::RenderVertexShader& ego::render::RasterizationMaterialRenderPassInfo::getVertexShader() const
{
    return m_vertexShader;
}

const ego::render::RenderPixelShader& ego::render::RasterizationMaterialRenderPassInfo::getPixelShader() const
{
    return m_pixelShader;
}

ego::render::RayTracingMaterialRenderPassInfo::RayTracingMaterialRenderPassInfo(const RayTracingMaterialHitGroup& _hitGroup)
    : m_hitGroup(_hitGroup)
{
}

const ego::render::RayTracingMaterialHitGroup& ego::render::RayTracingMaterialRenderPassInfo::getHitGroup() const
{
    return m_hitGroup;
}
