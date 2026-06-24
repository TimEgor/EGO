#include "Material.h"

ego::render::Material::Material(const RenderVertexShader& _vertexShader, const RenderPixelShader& _pixelShader)
    : m_vertexShader(_vertexShader),
      m_pixelShader(_pixelShader)
{
}

const ego::render::RenderVertexShader& ego::render::Material::getVertexShader() const
{
    return m_vertexShader;
}

const ego::render::RenderPixelShader& ego::render::Material::getPixelShader() const
{
    return m_pixelShader;
}
