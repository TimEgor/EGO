#include "Shader.h"

#include "EgoCore/UtilsMacros.h"

ego::gpu::ShaderCode::ShaderCode(void* _code, uint32_t _codeSize)
    : m_code(_code),
      m_codeSize(_codeSize)
{}

ego::gpu::ShaderCode::~ShaderCode()
{
    EGO_SAFE_DESTROY(m_code);
}

void* ego::gpu::ShaderCode::getCode() const
{
    return m_code;
}

uint32_t ego::gpu::ShaderCode::getCodeSize() const
{
    return m_codeSize;
}

ego::gpu::Shader::Shader(const ShaderCodePointer& _code)
    : m_code(_code)
{}

ego::gpu::ShaderCodePointer ego::gpu::Shader::getCode() const
{
    return m_code;
}

ego::gpu::VertexShader::VertexShader(const ShaderCodePointer& _code)
    : Shader(_code)
{}

ego::gpu::ShaderStage ego::gpu::VertexShader::getShaderType() const
{
    return ShaderStage::Vertex;
}

ego::gpu::PixelShader::PixelShader(const ShaderCodePointer& _code)
    : Shader(_code)
{}

ego::gpu::ShaderStage ego::gpu::PixelShader::getShaderType() const
{
    return ShaderStage::Pixel;
}

ego::gpu::ComputeShader::ComputeShader(const ShaderCodePointer& _code)
    : Shader(_code)
{}

ego::gpu::ShaderStage ego::gpu::ComputeShader::getShaderType() const
{
    return ShaderStage::Compute;
}
