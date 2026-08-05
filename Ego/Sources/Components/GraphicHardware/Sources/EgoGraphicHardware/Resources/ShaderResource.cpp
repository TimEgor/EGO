#include "ShaderResource.h"

#include <limits>

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

namespace
{
    template <typename TShader>
    ego::SharedPointer<TShader> MakeTypedShaderPointer(const ego::gpu::ShaderPointer& _shader, ego::gpu::ShaderStage _stage)
    {
        ego::gpu::Shader* shader = _shader.getObject();
        if (!shader || shader->getShaderType() != _stage)
        {
            return nullptr;
        }

        return ego::StaticPointerCast<TShader>(_shader);
    }
} // namespace

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

    const ShaderCodePointer code = MakeIntrusive<ShaderCode>(_content.data(), static_cast<uint32_t>(_content.size()));
    m_shader = createShader(code);

    return static_cast<bool>(m_shader);
}

void ego::gpu::ShaderResource::onUnload()
{
    m_shader.reset();
}

ego::gpu::VertexShaderPointer ego::gpu::VertexShaderResource::getVertexShader() const
{
    return MakeTypedShaderPointer<VertexShader>(getShader(), ShaderStage::Vertex);
}

ego::gpu::ShaderStage ego::gpu::VertexShaderResource::getShaderStage() const
{
    return ShaderStage::Vertex;
}

ego::gpu::ShaderPointer ego::gpu::VertexShaderResource::createShader(const ShaderCodePointer& _code)
{
    return gpu::GetGraphicDevice().createVertexShader(_code);
}

ego::gpu::PixelShaderPointer ego::gpu::PixelShaderResource::getPixelShader() const
{
    return MakeTypedShaderPointer<PixelShader>(getShader(), ShaderStage::Pixel);
}

ego::gpu::ShaderStage ego::gpu::PixelShaderResource::getShaderStage() const
{
    return ShaderStage::Pixel;
}

ego::gpu::ShaderPointer ego::gpu::PixelShaderResource::createShader(const ShaderCodePointer& _code)
{
    return gpu::GetGraphicDevice().createPixelShader(_code);
}

ego::gpu::ComputeShaderPointer ego::gpu::ComputeShaderResource::getComputeShader() const
{
    return MakeTypedShaderPointer<ComputeShader>(getShader(), ShaderStage::Compute);
}

ego::gpu::ShaderStage ego::gpu::ComputeShaderResource::getShaderStage() const
{
    return ShaderStage::Compute;
}

ego::gpu::ShaderPointer ego::gpu::ComputeShaderResource::createShader(const ShaderCodePointer& _code)
{
    return gpu::GetGraphicDevice().createComputeShader(_code);
}

ego::gpu::RayGenerationShaderPointer ego::gpu::RayGenerationShaderResource::getRayGenerationShader() const
{
    return MakeTypedShaderPointer<RayGenerationShader>(getShader(), ShaderStage::RayGeneration);
}

ego::gpu::ShaderStage ego::gpu::RayGenerationShaderResource::getShaderStage() const
{
    return ShaderStage::RayGeneration;
}

ego::gpu::ShaderPointer ego::gpu::RayGenerationShaderResource::createShader(const ShaderCodePointer& _code)
{
    return gpu::GetGraphicDevice().createRayGenerationShader(_code);
}

ego::gpu::MissShaderPointer ego::gpu::MissShaderResource::getMissShader() const
{
    return MakeTypedShaderPointer<MissShader>(getShader(), ShaderStage::Miss);
}

ego::gpu::ShaderStage ego::gpu::MissShaderResource::getShaderStage() const
{
    return ShaderStage::Miss;
}

ego::gpu::ShaderPointer ego::gpu::MissShaderResource::createShader(const ShaderCodePointer& _code)
{
    return gpu::GetGraphicDevice().createMissShader(_code);
}

ego::gpu::ClosestHitShaderPointer ego::gpu::ClosestHitShaderResource::getClosestHitShader() const
{
    return MakeTypedShaderPointer<ClosestHitShader>(getShader(), ShaderStage::ClosestHit);
}

ego::gpu::ShaderStage ego::gpu::ClosestHitShaderResource::getShaderStage() const
{
    return ShaderStage::ClosestHit;
}

ego::gpu::ShaderPointer ego::gpu::ClosestHitShaderResource::createShader(const ShaderCodePointer& _code)
{
    return gpu::GetGraphicDevice().createClosestHitShader(_code);
}

ego::gpu::AnyHitShaderPointer ego::gpu::AnyHitShaderResource::getAnyHitShader() const
{
    return MakeTypedShaderPointer<AnyHitShader>(getShader(), ShaderStage::AnyHit);
}

ego::gpu::ShaderStage ego::gpu::AnyHitShaderResource::getShaderStage() const
{
    return ShaderStage::AnyHit;
}

ego::gpu::ShaderPointer ego::gpu::AnyHitShaderResource::createShader(const ShaderCodePointer& _code)
{
    return gpu::GetGraphicDevice().createAnyHitShader(_code);
}

ego::gpu::IntersectionShaderPointer ego::gpu::IntersectionShaderResource::getIntersectionShader() const
{
    return MakeTypedShaderPointer<IntersectionShader>(getShader(), ShaderStage::Intersection);
}

ego::gpu::ShaderStage ego::gpu::IntersectionShaderResource::getShaderStage() const
{
    return ShaderStage::Intersection;
}

ego::gpu::ShaderPointer ego::gpu::IntersectionShaderResource::createShader(const ShaderCodePointer& _code)
{
    return gpu::GetGraphicDevice().createIntersectionShader(_code);
}
