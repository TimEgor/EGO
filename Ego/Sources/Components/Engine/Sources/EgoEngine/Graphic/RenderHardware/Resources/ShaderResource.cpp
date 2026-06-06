#include "ShaderResource.h"

#include "EgoEngine/Engine.h"

#include <limits>

namespace
{
    template <typename TShader>
    ego::Reference<TShader> MakeTypedShaderReference(
        const ego::gpu::ShaderReference& _shader,
        ego::gpu::ShaderStage _stage
    )
    {
        ego::gpu::Shader* shader = _shader.getObject();
        if (!shader || shader->getShaderType() != _stage)
        {
            return nullptr;
        }

        return static_cast<TShader*>(shader);
    }
}

ego::gpu::ShaderReference ego::gpu::ShaderResource::getShader() const
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

    const ShaderCodeReference code(new ShaderCode(_content.data(), static_cast<uint32_t>(_content.size())));
    m_shader = createShader(code);

    return static_cast<bool>(m_shader);
}

void ego::gpu::ShaderResource::onUnload()
{
    m_shader.reset();
}

ego::gpu::VertexShaderReference ego::gpu::VertexShaderResource::getVertexShader() const
{
    return MakeTypedShaderReference<VertexShader>(getShader(), ShaderStage::Vertex);
}

ego::gpu::ShaderStage ego::gpu::VertexShaderResource::getShaderStage() const
{
    return ShaderStage::Vertex;
}

ego::gpu::ShaderReference ego::gpu::VertexShaderResource::createShader(const ShaderCodeReference& _code)
{
    return engine::GetEngine().getGraphicDevice().createVertexShader(_code);
}

ego::gpu::PixelShaderReference ego::gpu::PixelShaderResource::getPixelShader() const
{
    return MakeTypedShaderReference<PixelShader>(getShader(), ShaderStage::Pixel);
}

ego::gpu::ShaderStage ego::gpu::PixelShaderResource::getShaderStage() const
{
    return ShaderStage::Pixel;
}

ego::gpu::ShaderReference ego::gpu::PixelShaderResource::createShader(const ShaderCodeReference& _code)
{
    return engine::GetEngine().getGraphicDevice().createPixelShader(_code);
}

ego::gpu::ComputeShaderReference ego::gpu::ComputeShaderResource::getComputeShader() const
{
    return MakeTypedShaderReference<ComputeShader>(getShader(), ShaderStage::Compute);
}

ego::gpu::ShaderStage ego::gpu::ComputeShaderResource::getShaderStage() const
{
    return ShaderStage::Compute;
}

ego::gpu::ShaderReference ego::gpu::ComputeShaderResource::createShader(const ShaderCodeReference& _code)
{
    return engine::GetEngine().getGraphicDevice().createComputeShader(_code);
}
