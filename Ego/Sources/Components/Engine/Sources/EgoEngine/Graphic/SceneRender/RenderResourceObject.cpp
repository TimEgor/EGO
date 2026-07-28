#include "RenderResourceObject.h"

#include "EgoGraphicHardware/Resources/ShaderResource.h"

ego::render::RenderShader ego::render::CreateShaderHandler(const SharedPointer<gpu::ShaderResource>& _resource)
{
    return MakeHandler<gpu::ShaderPointer>(
        _resource,
        [](const SharedPointer<gpu::ShaderResource>& _storedResource) -> gpu::ShaderPointer
        {
            return _storedResource ? _storedResource->getShader() : nullptr;
        });
}

ego::render::RenderVertexShader ego::render::CreateVertexShaderHandler(const SharedPointer<gpu::VertexShaderResource>& _resource)
{
    return MakeHandler<gpu::VertexShaderPointer>(
        _resource,
        [](const SharedPointer<gpu::VertexShaderResource>& _storedResource) -> gpu::VertexShaderPointer
        {
            return _storedResource ? _storedResource->getVertexShader() : nullptr;
        });
}

ego::render::RenderPixelShader ego::render::CreatePixelShaderHandler(const SharedPointer<gpu::PixelShaderResource>& _resource)
{
    return MakeHandler<gpu::PixelShaderPointer>(
        _resource,
        [](const SharedPointer<gpu::PixelShaderResource>& _storedResource) -> gpu::PixelShaderPointer
        {
            return _storedResource ? _storedResource->getPixelShader() : nullptr;
        });
}

ego::render::RenderComputeShader ego::render::CreateComputeShaderHandler(const SharedPointer<gpu::ComputeShaderResource>& _resource)
{
    return MakeHandler<gpu::ComputeShaderPointer>(
        _resource,
        [](const SharedPointer<gpu::ComputeShaderResource>& _storedResource) -> gpu::ComputeShaderPointer
        {
            return _storedResource ? _storedResource->getComputeShader() : nullptr;
        });
}

ego::render::RenderRayGenerationShader ego::render::CreateRayGenerationShaderHandler(const SharedPointer<gpu::RayGenerationShaderResource>& _resource)
{
    return MakeHandler<gpu::RayGenerationShaderPointer>(
        _resource,
        [](const SharedPointer<gpu::RayGenerationShaderResource>& _storedResource) -> gpu::RayGenerationShaderPointer
        {
            return _storedResource ? _storedResource->getRayGenerationShader() : nullptr;
        });
}

ego::render::RenderMissShader ego::render::CreateMissShaderHandler(const SharedPointer<gpu::MissShaderResource>& _resource)
{
    return MakeHandler<gpu::MissShaderPointer>(
        _resource,
        [](const SharedPointer<gpu::MissShaderResource>& _storedResource) -> gpu::MissShaderPointer
        {
            return _storedResource ? _storedResource->getMissShader() : nullptr;
        });
}

ego::render::RenderClosestHitShader ego::render::CreateClosestHitShaderHandler(const SharedPointer<gpu::ClosestHitShaderResource>& _resource)
{
    return MakeHandler<gpu::ClosestHitShaderPointer>(
        _resource,
        [](const SharedPointer<gpu::ClosestHitShaderResource>& _storedResource) -> gpu::ClosestHitShaderPointer
        {
            return _storedResource ? _storedResource->getClosestHitShader() : nullptr;
        });
}

ego::render::RenderAnyHitShader ego::render::CreateAnyHitShaderHandler(const SharedPointer<gpu::AnyHitShaderResource>& _resource)
{
    return MakeHandler<gpu::AnyHitShaderPointer>(
        _resource,
        [](const SharedPointer<gpu::AnyHitShaderResource>& _storedResource) -> gpu::AnyHitShaderPointer
        {
            return _storedResource ? _storedResource->getAnyHitShader() : nullptr;
        });
}

ego::render::RenderIntersectionShader ego::render::CreateIntersectionShaderHandler(const SharedPointer<gpu::IntersectionShaderResource>& _resource)
{
    return MakeHandler<gpu::IntersectionShaderPointer>(
        _resource,
        [](const SharedPointer<gpu::IntersectionShaderResource>& _storedResource) -> gpu::IntersectionShaderPointer
        {
            return _storedResource ? _storedResource->getIntersectionShader() : nullptr;
        });
}
