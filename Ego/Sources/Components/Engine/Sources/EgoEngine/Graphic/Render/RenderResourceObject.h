#pragma once

#include "EgoCore/Handler/Handler.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoEngine/Graphic/Render/RenderGpuObject.h"

namespace ego::gpu
{
    class ShaderResource;
    class VertexShaderResource;
    class PixelShaderResource;
    class ComputeShaderResource;
}

namespace ego::render
{
    RenderShader CreateShaderHandler(const SharedPointer<gpu::ShaderResource>& _resource);
    RenderVertexShader CreateVertexShaderHandler(const SharedPointer<gpu::VertexShaderResource>& _resource);
    RenderPixelShader CreatePixelShaderHandler(const SharedPointer<gpu::PixelShaderResource>& _resource);
    RenderComputeShader CreateComputeShaderHandler(const SharedPointer<gpu::ComputeShaderResource>& _resource);
}
