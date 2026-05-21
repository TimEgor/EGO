#pragma once

#include <vector>

#include <wrl/client.h>

#include "Common/D3D12DescriptorAllocator.h"

#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"

namespace ego::gpu::d3d12
{
    class D3D12CommandQueue;

    class D3D12GraphicDevice final : public GraphicDevice
    {
    public:
        D3D12GraphicDevice() = default;

        virtual bool init(const GraphicDeviceInitParams& _params) override;
        virtual void release() override;

        virtual void* getNativeHandle() const override;
        virtual void setName(const char* _name) override;

        virtual CommandQueueReference createCommandQueue(const CommandQueueDesc& _desc) override;
        virtual GraphicCommandListReference createGraphicCommandList() override;
        virtual ComputeCommandListReference createComputeCommandList() override;
        virtual CopyCommandListReference createCopyCommandList() override;

        virtual BufferReference createBuffer(
            const BufferDesc& _desc,
            const InitialGraphicResourceData& _initialData = InitialGraphicResourceData()
        ) override;
        virtual Texture2DReference createTexture2D(
            const Texture2DDesc& _desc,
            const InitialGraphicResourceData& _initialData = InitialGraphicResourceData()
        ) override;

        virtual VertexShaderReference createVertexShader(const ShaderCodeReference& _code) override;
        virtual PixelShaderReference createPixelShader(const ShaderCodeReference& _code) override;
        virtual ComputeShaderReference createComputeShader(const ShaderCodeReference& _code) override;
        virtual BindingLayoutReference createBindingLayout(const BindingLayoutDesc& _desc) override;
        virtual SamplerReference createSampler(const SamplerDesc& _desc) override;

        virtual BufferViewReference createBufferView(const BufferReference& _buffer, const BufferViewDesc& _desc) override;
        virtual TextureViewReference createTextureView(
            const TextureReference& _texture,
            const TextureViewDesc& _desc
        ) override;

        virtual GraphicPipelineReference createGraphicPipeline(const GraphicPipelineDesc& _desc) override;
        virtual ComputePipelineReference createComputePipeline(const ComputePipelineDesc& _desc) override;

        virtual FenceReference createFence(Fence::FenceValue _initialValue = 0) override;

        virtual SwapChainReference createSwapChain(const SwapChainDesc& _swapChainDesc, const Window& _window) override;

        virtual const GraphicDeviceCapabilities& getCapabilities() const override;

        virtual void waitIdle() override;

        ID3D12Device* getDevice() const;
        IDXGIAdapter1* getAdapter() const;

        D3D12DescriptorAllocatorPointer getViewDescriptorAllocator();
        D3D12DescriptorAllocatorPointer getSamplerDescriptorAllocator();
        D3D12DescriptorAllocatorPointer getRtvDescriptorAllocator();
        D3D12DescriptorAllocatorPointer getDsvDescriptorAllocator();

        void registerQueue(D3D12CommandQueue* _queue);
        void unregisterQueue(D3D12CommandQueue* _queue);

    private:
        bool initializeFactory(const GraphicDeviceInitParams& _params);
        bool initializeAdapter();
        bool initializeDevice();
        bool initializeDescriptorAllocators();
        void initializeCapabilities();

        Microsoft::WRL::ComPtr<IDXGIFactory6> m_factory;
        Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;
        Microsoft::WRL::ComPtr<ID3D12Device> m_device;

        D3D12DescriptorAllocatorPointer m_viewDescriptorAllocator;
        D3D12DescriptorAllocatorPointer m_samplerDescriptorAllocator;
        D3D12DescriptorAllocatorPointer m_rtvDescriptorAllocator;
        D3D12DescriptorAllocatorPointer m_dsvDescriptorAllocator;

        GraphicDeviceCapabilities m_capabilities;

        std::vector<D3D12CommandQueue*> m_queues;
    };
}
