#pragma once

#include <functional>
#include <string>
#include <vector>

#include "EgoGraphicHardware/GraphicDevice.h"

#include "D3D12DescriptorFactory.h"
#include "D3D12DeviceContext.h"
#include "D3D12ImmediateContext.h"

#include <wrl/client.h>

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
        ~D3D12GraphicDevice() override;

        using GraphicDevice::createBuffer;
        using GraphicDevice::createTexture2D;

        bool init(const GraphicDevice::InitData& _initData) override;

        std::string getResourceProviderName() const override;

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        CommandQueuePointer createCommandQueue(const CommandQueueDesc& _desc) override;
        GraphicCommandListPointer createGraphicCommandList() override;
        ComputeCommandListPointer createComputeCommandList() override;
        CopyCommandListPointer createCopyCommandList() override;

        BufferPointer createBuffer(const BufferDesc& _desc) override;
        GpuTaskPointer uploadBuffer(
            const BufferPointer& _buffer,
            const InitialGraphicResourceData& _initialData,
            const GpuOperationOptions& _options = GpuOperationOptions()) override;
        Texture2DPointer createTexture2D(const Texture2DDesc& _desc) override;
        GpuTaskPointer uploadTexture2D(
            const Texture2DPointer& _texture,
            const InitialGraphicResourceData& _initialData,
            const GpuOperationOptions& _options = GpuOperationOptions()) override;

        VertexShaderPointer createVertexShader(const ShaderCodePointer& _code) override;
        PixelShaderPointer createPixelShader(const ShaderCodePointer& _code) override;
        ComputeShaderPointer createComputeShader(const ShaderCodePointer& _code) override;
        RayGenerationShaderPointer createRayGenerationShader(const ShaderCodePointer& _code) override;
        MissShaderPointer createMissShader(const ShaderCodePointer& _code) override;
        ClosestHitShaderPointer createClosestHitShader(const ShaderCodePointer& _code) override;
        AnyHitShaderPointer createAnyHitShader(const ShaderCodePointer& _code) override;
        IntersectionShaderPointer createIntersectionShader(const ShaderCodePointer& _code) override;
        BindingLayoutPointer createBindingLayout(const BindingLayoutDesc& _desc) override;
        SamplerPointer createSampler(const SamplerDesc& _desc) override;

        GpuGeometryAccelerationStructureTicket buildGeometryAccelerationStructure(
            const MeshAccelerationStructureBuildDesc& _desc,
            const GpuOperationOptions& _options = GpuOperationOptions()) override;
        GpuInstanceAccelerationStructureTicket buildInstanceAccelerationStructure(
            const InstanceAccelerationStructureBuildDesc& _desc,
            const GpuOperationOptions& _options = GpuOperationOptions()) override;
        AccelerationStructureViewPointer createAccelerationStructureView(const InstanceAccelerationStructurePointer& _accelerationStructure) override;

        BufferViewPointer createBufferView(const BufferPointer& _buffer, const BufferViewDesc& _desc) override;
        TextureViewPointer createTextureView(const TexturePointer& _texture, const TextureViewDesc& _desc) override;

        GraphicPipelinePointer createGraphicPipeline(const GraphicPipelineDesc& _desc) override;
        ComputePipelinePointer createComputePipeline(const ComputePipelineDesc& _desc) override;
        RayTracingPipelinePointer createRayTracingPipeline(const RayTracingPipelineDesc& _desc) override;

        FencePointer createFence(Fence::FenceValue _initialValue = 0) override;

        SwapChainPointer createSwapChain(
            const SwapChainDesc& _swapChainDesc,
            const PlatformSurface& _surface,
            const CommandQueuePointer& _presentationQueue) override;

        const GraphicDevice::Capabilities& getCapabilities() const override;

        ID3D12Device* getDevice() const;
        IDXGIAdapter1* getAdapter() const;

        GpuTaskPointer submitImmediateCommands(
            const std::function<void(ID3D12GraphicsCommandList4*)>& _recordCommands,
            const std::vector<GraphicObjectPointer>& _keepAliveObjects = std::vector<GraphicObjectPointer>());
        bool executeImmediateCommands(const std::function<void(ID3D12GraphicsCommandList4*)>& _recordCommands);

    private:
        void release();

        ID3D12Device5* getD3D12Device() const;
        template <typename TCommandListPointer, typename TCommandListObject>
        TCommandListPointer createCommandList(D3D12_COMMAND_LIST_TYPE _type);
        ego::SharedPointer<D3D12Buffer> createD3D12Buffer(const BufferDesc& _desc);
        bool createUploadBuffer(uint64_t _size, Microsoft::WRL::ComPtr<ID3D12Resource>& _resource) const;
        ego::SharedPointer<D3D12Buffer> createUploadD3D12Buffer(uint64_t _size);
        GpuTaskPointer uploadBufferToDefaultHeap(ID3D12Resource* _dstResource, uint64_t _dstSize, const InitialGraphicResourceData& _initialData);
        GpuTaskPointer uploadTexture2DToDefaultHeap(
            ID3D12Resource* _dstResource,
            const D3D12_RESOURCE_DESC& _dstDesc,
            const InitialGraphicResourceData& _initialData);
        bool createShaderTable(
            ID3D12StateObject* _stateObject,
            const std::vector<std::wstring>& _hitGroupExportNames,
            Microsoft::WRL::ComPtr<ID3D12Resource>& _shaderTable,
            uint64_t& _shaderRecordSize) const;
        template <typename TPointer, typename TObject>
        GpuResourceTicket<TPointer> buildAccelerationStructure(
            const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& _inputs,
            const GpuOperationOptions& _options,
            const std::vector<GraphicObjectPointer>& _keepAliveObjects = std::vector<GraphicObjectPointer>());
        D3D12DeviceContext m_deviceContext;
        D3D12ImmediateContext m_immediateContext;
        D3D12DescriptorFactory m_descriptorFactory;
    };
} // namespace ego::gpu::d3d12
