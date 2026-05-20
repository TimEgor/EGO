#include "ShaderResource.h"

#include "EgoEngine/Engine.h"

#include <limits>

ego::gpu::ShaderPointer ego::gpu::ShaderResource::getShader() const
{
    return m_shader;
}

bool ego::gpu::ShaderResource::onLoad(ego::FileContent&& _content, ResourceLoadingContext&)
{
    if (_content.empty() || _content.size() > (std::numeric_limits<uint32_t>::max)())
    {
        m_shader.reset();
        return false;
    }

    const ShaderCodePointer code(new ShaderCode(_content.data(), static_cast<uint32_t>(_content.size())));
    m_shader = createShader(code);

    return static_cast<bool>(m_shader);
}

void ego::gpu::ShaderResource::onUnload()
{
    m_shader.reset();
}

ego::gpu::VertexShaderPointer ego::gpu::VertexShaderResource::getVertexShader() const
{
    return StaticPointerCast<VertexShader>(m_shader);
}

ego::gpu::ShaderPointer ego::gpu::VertexShaderResource::createShader(const ShaderCodePointer& _code)
{
    return engine::GetEngine().getGraphicDevice().createVertexShader(_code);
}

ego::gpu::PixelShaderPointer ego::gpu::PixelShaderResource::getPixelShader() const
{
    return StaticPointerCast<PixelShader>(m_shader);
}

ego::gpu::ShaderPointer ego::gpu::PixelShaderResource::createShader(const ShaderCodePointer& _code)
{
    return engine::GetEngine().getGraphicDevice().createPixelShader(_code);
}

ego::gpu::ComputeShaderPointer ego::gpu::ComputeShaderResource::getComputeShader() const
{
    return StaticPointerCast<ComputeShader>(m_shader);
}

ego::gpu::ShaderPointer ego::gpu::ComputeShaderResource::createShader(const ShaderCodePointer& _code)
{
    return engine::GetEngine().getGraphicDevice().createComputeShader(_code);
}
