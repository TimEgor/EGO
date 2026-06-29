#pragma once

#include "EgoCore/Reference/Pointer.h"

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

namespace ego
{
    class Window;
}

namespace ego::gpu
{
    using GpuBufferTicket = GpuResourceTicket<BufferReference>;
    using GpuTexture2DTicket = GpuResourceTicket<Texture2DReference>;
    using GpuGeometryAccelerationStructureTicket = GpuResourceTicket<GeometryAccelerationStructureReference>;
    using GpuInstanceAccelerationStructureTicket = GpuResourceTicket<InstanceAccelerationStructureReference>;
} // namespace ego::gpu

namespace ego
{
    class GraphicDevice : public gpu::GraphicObject
    {
    public:
        struct InitParams final
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

        virtual bool init(const InitParams& _params) = 0;
        virtual void release() = 0;

        virtual gpu::CommandQueueReference createCommandQueue(const gpu::CommandQueueDesc& _desc) = 0;
        virtual gpu::GraphicCommandListReference createGraphicCommandList() = 0;
        virtual gpu::ComputeCommandListReference createComputeCommandList() = 0;
        virtual gpu::CopyCommandListReference createCopyCommandList() = 0;

        virtual gpu::BufferReference createBuffer(const gpu::BufferDesc& _desc) = 0;
        gpu::GpuBufferTicket createAndUploadBuffer(
            const gpu::BufferDesc& _desc,
            const gpu::InitialGraphicResourceData& _initialData,
            const gpu::GpuOperationOptions& _options = gpu::GpuOperationOptions());
        virtual gpu::GpuTaskReference uploadBuffer(
            const gpu::BufferReference& _buffer,
            const gpu::InitialGraphicResourceData& _initialData,
            const gpu::GpuOperationOptions& _options = gpu::GpuOperationOptions()) = 0;

        virtual gpu::Texture2DReference createTexture2D(const gpu::Texture2DDesc& _desc) = 0;
        gpu::GpuTexture2DTicket createAndUploadTexture2D(
            const gpu::Texture2DDesc& _desc,
            const gpu::InitialGraphicResourceData& _initialData,
            const gpu::GpuOperationOptions& _options = gpu::GpuOperationOptions());
        virtual gpu::GpuTaskReference uploadTexture2D(
            const gpu::Texture2DReference& _texture,
            const gpu::InitialGraphicResourceData& _initialData,
            const gpu::GpuOperationOptions& _options = gpu::GpuOperationOptions()) = 0;

        virtual gpu::VertexShaderReference createVertexShader(const gpu::ShaderCodeReference& _code) = 0;
        virtual gpu::PixelShaderReference createPixelShader(const gpu::ShaderCodeReference& _code) = 0;
        virtual gpu::ComputeShaderReference createComputeShader(const gpu::ShaderCodeReference& _code) = 0;
        virtual gpu::RayGenerationShaderReference createRayGenerationShader(const gpu::ShaderCodeReference& _code) = 0;
        virtual gpu::MissShaderReference createMissShader(const gpu::ShaderCodeReference& _code) = 0;
        virtual gpu::ClosestHitShaderReference createClosestHitShader(const gpu::ShaderCodeReference& _code) = 0;
        virtual gpu::AnyHitShaderReference createAnyHitShader(const gpu::ShaderCodeReference& _code) = 0;
        virtual gpu::IntersectionShaderReference createIntersectionShader(const gpu::ShaderCodeReference& _code) = 0;
        virtual gpu::BindingLayoutReference createBindingLayout(const gpu::BindingLayoutDesc& _desc) = 0;
        virtual gpu::SamplerReference createSampler(const gpu::SamplerDesc& _desc) = 0;

        virtual gpu::GpuGeometryAccelerationStructureTicket buildGeometryAccelerationStructure(
            const gpu::MeshAccelerationStructureBuildDesc& _desc,
            const gpu::GpuOperationOptions& _options = gpu::GpuOperationOptions()) = 0;
        virtual gpu::GpuInstanceAccelerationStructureTicket buildInstanceAccelerationStructure(
            const gpu::InstanceAccelerationStructureBuildDesc& _desc,
            const gpu::GpuOperationOptions& _options = gpu::GpuOperationOptions()) = 0;
        virtual gpu::AccelerationStructureViewReference createAccelerationStructureView(const gpu::InstanceAccelerationStructureReference& _accelerationStructure) = 0;

        virtual gpu::BufferViewReference createBufferView(const gpu::BufferReference& _buffer, const gpu::BufferViewDesc& _desc) = 0;
        virtual gpu::TextureViewReference createTextureView(const gpu::TextureReference& _texture, const gpu::TextureViewDesc& _desc) = 0;

        virtual gpu::GraphicPipelineReference createGraphicPipeline(const gpu::GraphicPipelineDesc& _desc) = 0;
        virtual gpu::ComputePipelineReference createComputePipeline(const gpu::ComputePipelineDesc& _desc) = 0;
        virtual gpu::RayTracingPipelineReference createRayTracingPipeline(const gpu::RayTracingPipelineDesc& _desc) = 0;

        virtual gpu::FenceReference createFence(gpu::Fence::FenceValue _initialValue = 0) = 0;

        virtual gpu::SwapChainReference createSwapChain(const gpu::SwapChainDesc& _swapChainDesc, const Window& _window, const gpu::CommandQueueReference& _presentationQueue) = 0;

        virtual const Capabilities& getCapabilities() const = 0;
    };

    EGO_POINTER(GraphicDevice);
} // namespace ego
