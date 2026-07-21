#pragma once

#include "EgoCore/Handler/Handler.h"

#include "EgoGraphicHardware/GraphicObjects/AccelerationStructure.h"
#include "EgoGraphicHardware/GraphicObjects/BindingLayout.h"
#include "EgoGraphicHardware/GraphicObjects/Buffer.h"
#include "EgoGraphicHardware/GraphicObjects/CommandList.h"
#include "EgoGraphicHardware/GraphicObjects/CommandQueue.h"
#include "EgoGraphicHardware/GraphicObjects/Fence.h"
#include "EgoGraphicHardware/GraphicObjects/GpuTask.h"
#include "EgoGraphicHardware/GraphicObjects/GraphicObject.h"
#include "EgoGraphicHardware/GraphicObjects/Pipeline.h"
#include "EgoGraphicHardware/GraphicObjects/ResourceView.h"
#include "EgoGraphicHardware/GraphicObjects/Sampler.h"
#include "EgoGraphicHardware/GraphicObjects/Shader.h"
#include "EgoGraphicHardware/GraphicObjects/SwapChain.h"
#include "EgoGraphicHardware/GraphicObjects/Texture.h"

namespace ego::render
{
    // Base GPU objects.
    using RenderGraphicObject = Handler<gpu::GraphicObjectReference>;

    // Commands and synchronization.
    using RenderCommandQueue = Handler<gpu::CommandQueueReference>;
    using RenderFence = Handler<gpu::FenceReference>;
    using RenderGpuTask = Handler<gpu::GpuTaskReference>;
    using RenderCommandList = Handler<gpu::CommandListReference>;
    using RenderCopyCommandList = Handler<gpu::CopyCommandListReference>;
    using RenderComputeCommandList = Handler<gpu::ComputeCommandListReference>;
    using RenderGraphicCommandList = Handler<gpu::GraphicCommandListReference>;

    // Resource bindings and views.
    using RenderBindingLayout = Handler<gpu::BindingLayoutReference>;
    using RenderBuffer = Handler<gpu::BufferReference>;
    using RenderBufferView = Handler<gpu::BufferViewReference>;
    using RenderResourceView = Handler<gpu::ResourceViewReference>;
    using RenderAccelerationStructure = Handler<gpu::AccelerationStructureReference>;
    using RenderGeometryAccelerationStructure = Handler<gpu::GeometryAccelerationStructureReference>;
    using RenderInstanceAccelerationStructure = Handler<gpu::InstanceAccelerationStructureReference>;
    using RenderAccelerationStructureView = Handler<gpu::AccelerationStructureViewReference>;
    using RenderSampler = Handler<gpu::SamplerReference>;

    // Shaders and pipelines.
    using RenderPipeline = Handler<gpu::PipelineReference>;
    using RenderGraphicPipeline = Handler<gpu::GraphicPipelineReference>;
    using RenderComputePipeline = Handler<gpu::ComputePipelineReference>;
    using RenderRayTracingPipeline = Handler<gpu::RayTracingPipelineReference>;
    using RenderShaderCode = Handler<gpu::ShaderCodeReference>;
    using RenderShader = Handler<gpu::ShaderReference>;
    using RenderVertexShader = Handler<gpu::VertexShaderReference>;
    using RenderPixelShader = Handler<gpu::PixelShaderReference>;
    using RenderComputeShader = Handler<gpu::ComputeShaderReference>;
    using RenderRayGenerationShader = Handler<gpu::RayGenerationShaderReference>;
    using RenderMissShader = Handler<gpu::MissShaderReference>;
    using RenderClosestHitShader = Handler<gpu::ClosestHitShaderReference>;
    using RenderAnyHitShader = Handler<gpu::AnyHitShaderReference>;
    using RenderIntersectionShader = Handler<gpu::IntersectionShaderReference>;

    // Textures and presentation.
    using RenderTexture = Handler<gpu::TextureReference>;
    using RenderTexture2D = Handler<gpu::Texture2DReference>;
    using RenderTextureView = Handler<gpu::TextureViewReference>;
    using RenderSwapChain = Handler<gpu::SwapChainReference>;
} // namespace ego::render
