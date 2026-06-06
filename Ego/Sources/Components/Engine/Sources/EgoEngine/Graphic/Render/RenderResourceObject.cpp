#include "RenderResourceObject.h"

#include "EgoEngine/Graphic/RenderHardware/Resources/ShaderResource.h"

ego::render::RenderShader ego::render::CreateShaderHandler(const SharedPointer<gpu::ShaderResource>& _resource)
{
    return MakeHandler<gpu::ShaderReference>(
        _resource,
        [](const SharedPointer<gpu::ShaderResource>& _storedResource) -> gpu::ShaderReference
        {
            return _storedResource ? _storedResource->getShader() : nullptr;
        }
    );
}

ego::render::RenderVertexShader ego::render::CreateVertexShaderHandler(
    const SharedPointer<gpu::VertexShaderResource>& _resource
)
{
    return MakeHandler<gpu::VertexShaderReference>(
        _resource,
        [](const SharedPointer<gpu::VertexShaderResource>& _storedResource) -> gpu::VertexShaderReference
        {
            return _storedResource ? _storedResource->getVertexShader() : nullptr;
        }
    );
}

ego::render::RenderPixelShader ego::render::CreatePixelShaderHandler(
    const SharedPointer<gpu::PixelShaderResource>& _resource
)
{
    return MakeHandler<gpu::PixelShaderReference>(
        _resource,
        [](const SharedPointer<gpu::PixelShaderResource>& _storedResource) -> gpu::PixelShaderReference
        {
            return _storedResource ? _storedResource->getPixelShader() : nullptr;
        }
    );
}

ego::render::RenderComputeShader ego::render::CreateComputeShaderHandler(
    const SharedPointer<gpu::ComputeShaderResource>& _resource
)
{
    return MakeHandler<gpu::ComputeShaderReference>(
        _resource,
        [](const SharedPointer<gpu::ComputeShaderResource>& _storedResource) -> gpu::ComputeShaderReference
        {
            return _storedResource ? _storedResource->getComputeShader() : nullptr;
        }
    );
}

