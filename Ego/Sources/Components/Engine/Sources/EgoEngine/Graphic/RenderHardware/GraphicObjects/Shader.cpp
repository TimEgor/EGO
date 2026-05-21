#include "Shader.h"

#include "EgoCore/UtilsMacros.h"

#include <cstring>

ego::gpu::ShaderCode::ShaderCode(const void* _code, uint32_t _codeSize)
{
    if (!_code || !_codeSize)
    {
        return;
    }

    m_code = new uint8_t[_codeSize];
    std::memcpy(m_code, _code, _codeSize);
    m_codeSize = _codeSize;
}

ego::gpu::ShaderCode::~ShaderCode()
{
    EGO_SAFE_DESTROY_ARRAY(m_code);
}

void* ego::gpu::ShaderCode::getCode() const
{
    return m_code;
}

uint32_t ego::gpu::ShaderCode::getCodeSize() const
{
    return m_codeSize;
}

ego::gpu::Shader::Shader(const ShaderCodeReference& _code)
    : m_code(_code)
{}

ego::gpu::ShaderCodeReference ego::gpu::Shader::getCode() const
{
    return m_code;
}

ego::gpu::VertexShader::VertexShader(const ShaderCodeReference& _code)
    : Shader(_code)
{}

ego::gpu::ShaderStage ego::gpu::VertexShader::getShaderType() const
{
    return ShaderStage::Vertex;
}

ego::gpu::PixelShader::PixelShader(const ShaderCodeReference& _code)
    : Shader(_code)
{}

ego::gpu::ShaderStage ego::gpu::PixelShader::getShaderType() const
{
    return ShaderStage::Pixel;
}

ego::gpu::ComputeShader::ComputeShader(const ShaderCodeReference& _code)
    : Shader(_code)
{}

ego::gpu::ShaderStage ego::gpu::ComputeShader::getShaderType() const
{
    return ShaderStage::Compute;
}
