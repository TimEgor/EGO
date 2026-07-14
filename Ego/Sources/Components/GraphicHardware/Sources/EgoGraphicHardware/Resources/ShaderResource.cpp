#include "ShaderResource.h"

#include <limits>

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

namespace
{
    template <typename TShader>
    ego::Reference<TShader> MakeTypedShaderReference(const ego::gpu::ShaderReference& _shader, ego::gpu::ShaderStage _stage)
    {
        ego::gpu::Shader* shader = _shader.getObject();
        if (!shader || shader->getShaderType() != _stage)
        {
            return nullptr;
        }

        return static_cast<TShader*>(shader);
    }
} // namespace

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

    return m_shader != nullptr;
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
    return gpu::GetGraphicDevice().createVertexShader(_code);
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
    return gpu::GetGraphicDevice().createPixelShader(_code);
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
    return gpu::GetGraphicDevice().createComputeShader(_code);
}

ego::gpu::RayGenerationShaderReference ego::gpu::RayGenerationShaderResource::getRayGenerationShader() const
{
    return MakeTypedShaderReference<RayGenerationShader>(getShader(), ShaderStage::RayGeneration);
}

ego::gpu::ShaderStage ego::gpu::RayGenerationShaderResource::getShaderStage() const
{
    return ShaderStage::RayGeneration;
}

ego::gpu::ShaderReference ego::gpu::RayGenerationShaderResource::createShader(const ShaderCodeReference& _code)
{
    return gpu::GetGraphicDevice().createRayGenerationShader(_code);
}

ego::gpu::MissShaderReference ego::gpu::MissShaderResource::getMissShader() const
{
    return MakeTypedShaderReference<MissShader>(getShader(), ShaderStage::Miss);
}

ego::gpu::ShaderStage ego::gpu::MissShaderResource::getShaderStage() const
{
    return ShaderStage::Miss;
}

ego::gpu::ShaderReference ego::gpu::MissShaderResource::createShader(const ShaderCodeReference& _code)
{
    return gpu::GetGraphicDevice().createMissShader(_code);
}

ego::gpu::ClosestHitShaderReference ego::gpu::ClosestHitShaderResource::getClosestHitShader() const
{
    return MakeTypedShaderReference<ClosestHitShader>(getShader(), ShaderStage::ClosestHit);
}

ego::gpu::ShaderStage ego::gpu::ClosestHitShaderResource::getShaderStage() const
{
    return ShaderStage::ClosestHit;
}

ego::gpu::ShaderReference ego::gpu::ClosestHitShaderResource::createShader(const ShaderCodeReference& _code)
{
    return gpu::GetGraphicDevice().createClosestHitShader(_code);
}

ego::gpu::AnyHitShaderReference ego::gpu::AnyHitShaderResource::getAnyHitShader() const
{
    return MakeTypedShaderReference<AnyHitShader>(getShader(), ShaderStage::AnyHit);
}

ego::gpu::ShaderStage ego::gpu::AnyHitShaderResource::getShaderStage() const
{
    return ShaderStage::AnyHit;
}

ego::gpu::ShaderReference ego::gpu::AnyHitShaderResource::createShader(const ShaderCodeReference& _code)
{
    return gpu::GetGraphicDevice().createAnyHitShader(_code);
}

ego::gpu::IntersectionShaderReference ego::gpu::IntersectionShaderResource::getIntersectionShader() const
{
    return MakeTypedShaderReference<IntersectionShader>(getShader(), ShaderStage::Intersection);
}

ego::gpu::ShaderStage ego::gpu::IntersectionShaderResource::getShaderStage() const
{
    return ShaderStage::Intersection;
}

ego::gpu::ShaderReference ego::gpu::IntersectionShaderResource::createShader(const ShaderCodeReference& _code)
{
    return gpu::GetGraphicDevice().createIntersectionShader(_code);
}
