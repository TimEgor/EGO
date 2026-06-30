#pragma once

#include <functional>
#include <string>
#include <vector>

#include <wrl/client.h>

#include "D3D12DescriptorFactory.h"
#include "D3D12DeviceContext.h"
#include "D3D12ImmediateContext.h"

#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"
#include "EgoEngine/Resources/Resource/ResourceProviderPlugin.h"

namespace ego::gpu::d3d12
{
    class D3D12Buffer;
    class D3D12CommandQueue;
    class D3D12GeometryAccelerationStructure;
    class D3D12InstanceAccelerationStructure;

    class D3D12GraphicDevice final : public GraphicDevice
    {
    public:
        D3D12GraphicDevice() = default;

        using GraphicDevice::createBuffer;
        using GraphicDevice::createTexture2D;

        bool init(const GraphicDevice::InitParams& _params) override;
        void release() override;

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        CommandQueueReference createCommandQueue(const CommandQueueDesc& _desc) override;
        GraphicCommandListReference createGraphicCommandList() override;
        ComputeCommandListReference createComputeCommandList() override;
        CopyCommandListReference createCopyCommandList() override;

        BufferReference createBuffer(const BufferDesc& _desc) override;
        GpuTaskReference uploadBuffer(
            const BufferReference& _buffer,
            const InitialGraphicResourceData& _initialData,
            const GpuOperationOptions& _options = GpuOperationOptions()) override;
        Texture2DReference createTexture2D(const Texture2DDesc& _desc) override;
        GpuTaskReference uploadTexture2D(
            const Texture2DReference& _texture,
            const InitialGraphicResourceData& _initialData,
            const GpuOperationOptions& _options = GpuOperationOptions()) override;

        VertexShaderReference createVertexShader(const ShaderCodeReference& _code) override;
        PixelShaderReference createPixelShader(const ShaderCodeReference& _code) override;
        ComputeShaderReference createComputeShader(const ShaderCodeReference& _code) override;
        RayGenerationShaderReference createRayGenerationShader(const ShaderCodeReference& _code) override;
        MissShaderReference createMissShader(const ShaderCodeReference& _code) override;
        ClosestHitShaderReference createClosestHitShader(const ShaderCodeReference& _code) override;
        AnyHitShaderReference createAnyHitShader(const ShaderCodeReference& _code) override;
        IntersectionShaderReference createIntersectionShader(const ShaderCodeReference& _code) override;
        BindingLayoutReference createBindingLayout(const BindingLayoutDesc& _desc) override;
        SamplerReference createSampler(const SamplerDesc& _desc) override;

        GpuGeometryAccelerationStructureTicket buildGeometryAccelerationStructure(
            const MeshAccelerationStructureBuildDesc& _desc,
            const GpuOperationOptions& _options = GpuOperationOptions()) override;
        GpuInstanceAccelerationStructureTicket buildInstanceAccelerationStructure(
            const InstanceAccelerationStructureBuildDesc& _desc,
            const GpuOperationOptions& _options = GpuOperationOptions()) override;
        AccelerationStructureViewReference createAccelerationStructureView(const InstanceAccelerationStructureReference& _accelerationStructure) override;

        BufferViewReference createBufferView(const BufferReference& _buffer, const BufferViewDesc& _desc) override;
        TextureViewReference createTextureView(const TextureReference& _texture, const TextureViewDesc& _desc) override;

        GraphicPipelineReference createGraphicPipeline(const GraphicPipelineDesc& _desc) override;
        ComputePipelineReference createComputePipeline(const ComputePipelineDesc& _desc) override;
        RayTracingPipelineReference createRayTracingPipeline(const RayTracingPipelineDesc& _desc) override;

        FenceReference createFence(Fence::FenceValue _initialValue = 0) override;

        SwapChainReference createSwapChain(const SwapChainDesc& _swapChainDesc, const PresentationSurface& _surface, const CommandQueueReference& _presentationQueue) override;

        const GraphicDevice::Capabilities& getCapabilities() const override;

        ID3D12Device* getDevice() const;
        IDXGIAdapter1* getAdapter() const;

        GpuTaskReference submitImmediateCommands(
            const std::function<void(ID3D12GraphicsCommandList4*)>& _recordCommands,
            const std::vector<GraphicObjectReference>& _keepAliveObjects = std::vector<GraphicObjectReference>());
        bool executeImmediateCommands(const std::function<void(ID3D12GraphicsCommandList4*)>& _recordCommands);

    private:
        ID3D12Device5* getD3D12Device() const;
        template <typename TCommandListReference, typename TCommandListObject>
        TCommandListReference createCommandList(D3D12_COMMAND_LIST_TYPE _type);
        ego::Reference<D3D12Buffer> createD3D12Buffer(const BufferDesc& _desc);
        bool createUploadBuffer(uint64_t _size, Microsoft::WRL::ComPtr<ID3D12Resource>& _resource) const;
        ego::Reference<D3D12Buffer> createUploadD3D12Buffer(uint64_t _size);
        GpuTaskReference uploadBufferToDefaultHeap(ID3D12Resource* _dstResource, uint64_t _dstSize, const InitialGraphicResourceData& _initialData);
        GpuTaskReference uploadTexture2DToDefaultHeap(ID3D12Resource* _dstResource, const D3D12_RESOURCE_DESC& _dstDesc, const InitialGraphicResourceData& _initialData);
        bool createShaderTable(
            ID3D12StateObject* _stateObject,
            const std::vector<std::wstring>& _hitGroupExportNames,
            Microsoft::WRL::ComPtr<ID3D12Resource>& _shaderTable,
            uint64_t& _shaderRecordSize) const;
        template <typename TReference, typename TObject>
        GpuResourceTicket<TReference> buildAccelerationStructure(
            const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& _inputs,
            const GpuOperationOptions& _options,
            const std::vector<GraphicObjectReference>& _keepAliveObjects = std::vector<GraphicObjectReference>());
        bool registerResourceProviders();
        void unregisterResourceProviders();

        D3D12DeviceContext m_deviceContext;
        D3D12ImmediateContext m_immediateContext;
        D3D12DescriptorFactory m_descriptorFactory;

        ResourceProviderPluginPointer m_resourceProviderPlugin = nullptr;
    };
} // namespace ego::gpu::d3d12
