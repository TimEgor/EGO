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
    using RenderGraphicObject = Handler<gpu::GraphicObjectPointer>;

    // Commands and synchronization.
    using RenderCommandQueue = Handler<gpu::CommandQueuePointer>;
    using RenderFence = Handler<gpu::FencePointer>;
    using RenderGpuTask = Handler<gpu::GpuTaskPointer>;
    using RenderCommandList = Handler<gpu::CommandListPointer>;
    using RenderCopyCommandList = Handler<gpu::CopyCommandListPointer>;
    using RenderComputeCommandList = Handler<gpu::ComputeCommandListPointer>;
    using RenderGraphicCommandList = Handler<gpu::GraphicCommandListPointer>;

    // Resource bindings and views.
    using RenderBindingLayout = Handler<gpu::BindingLayoutPointer>;
    using RenderBuffer = Handler<gpu::BufferPointer>;
    using RenderBufferView = Handler<gpu::BufferViewPointer>;
    using RenderResourceView = Handler<gpu::ResourceViewPointer>;
    using RenderAccelerationStructure = Handler<gpu::AccelerationStructurePointer>;
    using RenderGeometryAccelerationStructure = Handler<gpu::GeometryAccelerationStructurePointer>;
    using RenderInstanceAccelerationStructure = Handler<gpu::InstanceAccelerationStructurePointer>;
    using RenderAccelerationStructureView = Handler<gpu::AccelerationStructureViewPointer>;
    using RenderSampler = Handler<gpu::SamplerPointer>;

    // Shaders and pipelines.
    using RenderPipeline = Handler<gpu::PipelinePointer>;
    using RenderGraphicPipeline = Handler<gpu::GraphicPipelinePointer>;
    using RenderComputePipeline = Handler<gpu::ComputePipelinePointer>;
    using RenderRayTracingPipeline = Handler<gpu::RayTracingPipelinePointer>;
    using RenderShaderCode = Handler<gpu::ShaderCodePointer>;
    using RenderShader = Handler<gpu::ShaderPointer>;
    using RenderVertexShader = Handler<gpu::VertexShaderPointer>;
    using RenderPixelShader = Handler<gpu::PixelShaderPointer>;
    using RenderComputeShader = Handler<gpu::ComputeShaderPointer>;
    using RenderRayGenerationShader = Handler<gpu::RayGenerationShaderPointer>;
    using RenderMissShader = Handler<gpu::MissShaderPointer>;
    using RenderClosestHitShader = Handler<gpu::ClosestHitShaderPointer>;
    using RenderAnyHitShader = Handler<gpu::AnyHitShaderPointer>;
    using RenderIntersectionShader = Handler<gpu::IntersectionShaderPointer>;

    // Textures and presentation.
    using RenderTexture = Handler<gpu::TexturePointer>;
    using RenderTexture2D = Handler<gpu::Texture2DPointer>;
    using RenderTextureView = Handler<gpu::TextureViewPointer>;
    using RenderSwapChain = Handler<gpu::SwapChainPointer>;
} // namespace ego::render
