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
    class RayGenerationShaderResource;
    class MissShaderResource;
    class ClosestHitShaderResource;
    class AnyHitShaderResource;
    class IntersectionShaderResource;
} // namespace ego::gpu

namespace ego::render
{
    RenderShader CreateShaderHandler(const SharedPointer<gpu::ShaderResource>& _resource);
    RenderVertexShader CreateVertexShaderHandler(const SharedPointer<gpu::VertexShaderResource>& _resource);
    RenderPixelShader CreatePixelShaderHandler(const SharedPointer<gpu::PixelShaderResource>& _resource);
    RenderComputeShader CreateComputeShaderHandler(const SharedPointer<gpu::ComputeShaderResource>& _resource);
    RenderRayGenerationShader CreateRayGenerationShaderHandler(const SharedPointer<gpu::RayGenerationShaderResource>& _resource);
    RenderMissShader CreateMissShaderHandler(const SharedPointer<gpu::MissShaderResource>& _resource);
    RenderClosestHitShader CreateClosestHitShaderHandler(const SharedPointer<gpu::ClosestHitShaderResource>& _resource);
    RenderAnyHitShader CreateAnyHitShaderHandler(const SharedPointer<gpu::AnyHitShaderResource>& _resource);
    RenderIntersectionShader CreateIntersectionShaderHandler(const SharedPointer<gpu::IntersectionShaderResource>& _resource);
} // namespace ego::render
