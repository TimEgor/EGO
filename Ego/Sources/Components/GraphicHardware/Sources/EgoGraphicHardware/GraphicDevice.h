#pragma once

#include <string>

#include "EgoCore/Platform/Surface/PlatformSurface.h"
#include "EgoCore/Pointer/Pointer.h"

#include "GraphicObjects/AccelerationStructure.h"
#include "GraphicObjects/BindingLayout.h"
#include "GraphicObjects/Buffer.h"
#include "GraphicObjects/CommandList.h"
#include "GraphicObjects/CommandQueue.h"
#include "GraphicObjects/Fence.h"
#include "GraphicObjects/GpuTask.h"
#include "GraphicObjects/Pipeline.h"
#include "GraphicObjects/Sampler.h"
#include "GraphicObjects/Shader.h"
#include "GraphicObjects/SwapChain.h"
#include "GraphicObjects/Texture.h"

namespace ego::gpu
{
    using GpuBufferTicket = GpuResourceTicket<BufferPointer>;
    using GpuTexture2DTicket = GpuResourceTicket<Texture2DPointer>;
    using GpuGeometryAccelerationStructureTicket = GpuResourceTicket<GeometryAccelerationStructurePointer>;
    using GpuInstanceAccelerationStructureTicket = GpuResourceTicket<InstanceAccelerationStructurePointer>;
} // namespace ego::gpu

namespace ego
{
    class GraphicDevice : public gpu::GraphicObject
    {
    public:
        struct InitData final
        {
            bool m_debugEnable = false;
            bool m_gpuValidation = false;
        };

        struct Capabilities final
        {
            uint32_t m_maxColorAttachments = 0;
            bool m_supportsPresentation = true;
            bool m_supportsIndependentComputeQueue = false;
            bool m_supportsMeshShaders = false;
            bool m_supportsRayTracing = false;
            bool m_supportsSamplerAnisotropy = false;
            bool m_supportsBindlessResources = false;
            uint32_t m_bindlessResourceDescriptorCount = 0;
            uint32_t m_bindlessSamplerDescriptorCount = 0;
        };

        GraphicDevice() = default;

        virtual bool init(const InitData& _initData) = 0;
        virtual void release() = 0;

        virtual std::string getResourceProviderName() const = 0;

        virtual gpu::CommandQueuePointer createCommandQueue(const gpu::CommandQueueDesc& _desc) = 0;
        virtual gpu::GraphicCommandListPointer createGraphicCommandList() = 0;
        virtual gpu::ComputeCommandListPointer createComputeCommandList() = 0;
        virtual gpu::CopyCommandListPointer createCopyCommandList() = 0;

        virtual gpu::BufferPointer createBuffer(const gpu::BufferDesc& _desc) = 0;
        gpu::GpuBufferTicket createAndUploadBuffer(
            const gpu::BufferDesc& _desc,
            const gpu::InitialGraphicResourceData& _initialData,
            const gpu::GpuOperationOptions& _options = gpu::GpuOperationOptions());
        virtual gpu::GpuTaskPointer uploadBuffer(
            const gpu::BufferPointer& _buffer,
            const gpu::InitialGraphicResourceData& _initialData,
            const gpu::GpuOperationOptions& _options = gpu::GpuOperationOptions()) = 0;

        virtual gpu::Texture2DPointer createTexture2D(const gpu::Texture2DDesc& _desc) = 0;
        gpu::GpuTexture2DTicket createAndUploadTexture2D(
            const gpu::Texture2DDesc& _desc,
            const gpu::InitialGraphicResourceData& _initialData,
            const gpu::GpuOperationOptions& _options = gpu::GpuOperationOptions());
        virtual gpu::GpuTaskPointer uploadTexture2D(
            const gpu::Texture2DPointer& _texture,
            const gpu::InitialGraphicResourceData& _initialData,
            const gpu::GpuOperationOptions& _options = gpu::GpuOperationOptions()) = 0;

        virtual gpu::VertexShaderPointer createVertexShader(const gpu::ShaderCodePointer& _code) = 0;
        virtual gpu::PixelShaderPointer createPixelShader(const gpu::ShaderCodePointer& _code) = 0;
        virtual gpu::ComputeShaderPointer createComputeShader(const gpu::ShaderCodePointer& _code) = 0;
        virtual gpu::RayGenerationShaderPointer createRayGenerationShader(const gpu::ShaderCodePointer& _code) = 0;
        virtual gpu::MissShaderPointer createMissShader(const gpu::ShaderCodePointer& _code) = 0;
        virtual gpu::ClosestHitShaderPointer createClosestHitShader(const gpu::ShaderCodePointer& _code) = 0;
        virtual gpu::AnyHitShaderPointer createAnyHitShader(const gpu::ShaderCodePointer& _code) = 0;
        virtual gpu::IntersectionShaderPointer createIntersectionShader(const gpu::ShaderCodePointer& _code) = 0;
        virtual gpu::BindingLayoutPointer createBindingLayout(const gpu::BindingLayoutDesc& _desc) = 0;
        virtual gpu::SamplerPointer createSampler(const gpu::SamplerDesc& _desc) = 0;

        virtual gpu::GpuGeometryAccelerationStructureTicket buildGeometryAccelerationStructure(
            const gpu::MeshAccelerationStructureBuildDesc& _desc,
            const gpu::GpuOperationOptions& _options = gpu::GpuOperationOptions()) = 0;
        virtual gpu::GpuInstanceAccelerationStructureTicket buildInstanceAccelerationStructure(
            const gpu::InstanceAccelerationStructureBuildDesc& _desc,
            const gpu::GpuOperationOptions& _options = gpu::GpuOperationOptions()) = 0;
        virtual gpu::AccelerationStructureViewPointer createAccelerationStructureView(
            const gpu::InstanceAccelerationStructurePointer& _accelerationStructure) = 0;

        virtual gpu::BufferViewPointer createBufferView(const gpu::BufferPointer& _buffer, const gpu::BufferViewDesc& _desc) = 0;
        virtual gpu::TextureViewPointer createTextureView(const gpu::TexturePointer& _texture, const gpu::TextureViewDesc& _desc) = 0;

        virtual gpu::GraphicPipelinePointer createGraphicPipeline(const gpu::GraphicPipelineDesc& _desc) = 0;
        virtual gpu::ComputePipelinePointer createComputePipeline(const gpu::ComputePipelineDesc& _desc) = 0;
        virtual gpu::RayTracingPipelinePointer createRayTracingPipeline(const gpu::RayTracingPipelineDesc& _desc) = 0;

        virtual gpu::FencePointer createFence(gpu::Fence::FenceValue _initialValue = 0) = 0;

        virtual gpu::SwapChainPointer createSwapChain(
            const gpu::SwapChainDesc& _swapChainDesc,
            const PlatformSurface& _surface,
            const gpu::CommandQueuePointer& _presentationQueue) = 0;

        virtual const Capabilities& getCapabilities() const = 0;
    };

    EGO_POINTER(GraphicDevice);
} // namespace ego
