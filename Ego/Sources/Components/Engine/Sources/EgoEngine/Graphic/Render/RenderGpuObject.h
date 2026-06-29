#pragma once

#include "EgoCore/Handler/Handler.h"

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/AccelerationStructure.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/BindingLayout.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Buffer.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/CommandList.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/CommandQueue.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Fence.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/GpuTask.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/GraphicObject.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Pipeline.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/ResourceView.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Sampler.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Shader.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/SwapChain.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Texture.h"

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
