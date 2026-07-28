#include "Shader.h"

#include <cstring>

#include "EgoCore/UtilsMacros.h"

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

ego::gpu::Shader::Shader(const ShaderCodePointer& _code)
    : m_code(_code)
{
}

ego::gpu::ShaderCodePointer ego::gpu::Shader::getCode() const
{
    return m_code;
}

ego::gpu::VertexShader::VertexShader(const ShaderCodePointer& _code)
    : Shader(_code)
{
}

ego::gpu::ShaderStage ego::gpu::VertexShader::getShaderType() const
{
    return ShaderStage::Vertex;
}

ego::gpu::PixelShader::PixelShader(const ShaderCodePointer& _code)
    : Shader(_code)
{
}

ego::gpu::ShaderStage ego::gpu::PixelShader::getShaderType() const
{
    return ShaderStage::Pixel;
}

ego::gpu::ComputeShader::ComputeShader(const ShaderCodePointer& _code)
    : Shader(_code)
{
}

ego::gpu::ShaderStage ego::gpu::ComputeShader::getShaderType() const
{
    return ShaderStage::Compute;
}

ego::gpu::RayGenerationShader::RayGenerationShader(const ShaderCodePointer& _code)
    : Shader(_code)
{
}

ego::gpu::ShaderStage ego::gpu::RayGenerationShader::getShaderType() const
{
    return ShaderStage::RayGeneration;
}

ego::gpu::MissShader::MissShader(const ShaderCodePointer& _code)
    : Shader(_code)
{
}

ego::gpu::ShaderStage ego::gpu::MissShader::getShaderType() const
{
    return ShaderStage::Miss;
}

ego::gpu::ClosestHitShader::ClosestHitShader(const ShaderCodePointer& _code)
    : Shader(_code)
{
}

ego::gpu::ShaderStage ego::gpu::ClosestHitShader::getShaderType() const
{
    return ShaderStage::ClosestHit;
}

ego::gpu::AnyHitShader::AnyHitShader(const ShaderCodePointer& _code)
    : Shader(_code)
{
}

ego::gpu::ShaderStage ego::gpu::AnyHitShader::getShaderType() const
{
    return ShaderStage::AnyHit;
}

ego::gpu::IntersectionShader::IntersectionShader(const ShaderCodePointer& _code)
    : Shader(_code)
{
}

ego::gpu::ShaderStage ego::gpu::IntersectionShader::getShaderType() const
{
    return ShaderStage::Intersection;
}
