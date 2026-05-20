#pragma once

#include <vector>

#include <wrl/client.h>

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/CommandList.h"

#include "EgoD3D12RenderHardware/Common/D3D12Utils.h"

namespace ego::gpu::d3d12
{
    class D3D12GraphicDevice;
    class D3D12BindingLayout;

    class D3D12CommandListBase
    {
    public:
        D3D12CommandListBase(
            D3D12GraphicDevice* _device,
            CommandType _commandType,
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>&& _commandList
        );
        virtual ~D3D12CommandListBase() = default;

        ID3D12GraphicsCommandList* getD3D12CommandList() const;

    protected:
        void beginInternal();
        void resetInternal();
        void endInternal();

        void bindBindlessDescriptorHeapsInternal();
        void resourceBarrierInternal(
            const GraphicResourcePointer& _resource,
            GraphicResourceState _prevState,
            GraphicResourceState _nextState
        );

        void bindResourceViewInternal(uint32_t _slot, const ResourceViewPointer& _resourceView);
        void bindSamplerInternal(uint32_t _slot, const SamplerPointer& _sampler);
        uint32_t getResourceViewBindlessIndexInternal(uint32_t _slot) const;
        uint32_t getSamplerBindlessIndexInternal(uint32_t _slot) const;
        void pushConstantsInternal(ShaderStageFlags _stageFlags, uint32_t _offset, uint32_t _size, const void* _data);

        void copyBufferInternal(
            const BufferPointer& _srcBuffer,
            const BufferPointer& _dstBuffer,
            const BufferCopyRegionDesc& _region
        );
        void copyTextureInternal(
            const TexturePointer& _srcTexture,
            const TexturePointer& _dstTexture,
            const TextureCopyRegionDesc& _region
        );
        void copyBufferToTextureInternal(
            const BufferPointer& _srcBuffer,
            const TexturePointer& _dstTexture,
            const BufferTextureCopyRegionDesc& _region
        );
        void copyTextureToBufferInternal(
            const TexturePointer& _srcTexture,
            const BufferPointer& _dstBuffer,
            const BufferTextureCopyRegionDesc& _region
        );

        D3D12GraphicDevice* m_device = nullptr;
        CommandType m_commandType = CommandType::Graphic;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_allocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
        const D3D12BindingLayout* m_currentBindingLayout = nullptr;
        PipelineType m_currentPipelineType = PipelineType::Graphic;
        std::vector<ResourceViewPointer> m_boundResourceViews;
        std::vector<SamplerPointer> m_boundSamplers;
    };

    class D3D12CopyCommandList final : public CopyCommandList,
                                       public D3D12CommandListBase
    {
    public:
        D3D12CopyCommandList(
            D3D12GraphicDevice* _device,
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>&& _commandList
        );

        virtual void* getNativeHandle() const override;
        virtual void setName(const char* _name) override;

        virtual void begin() override;
        virtual void reset() override;
        virtual void end() override;

        virtual void resourceBarrier(
            const GraphicResourcePointer& _resource,
            GraphicResourceState _prevState,
            GraphicResourceState _nextState
        ) override;

        virtual void bindResourceView(uint32_t _slot, const ResourceViewPointer& _resourceView) override;
        virtual void bindSampler(uint32_t _slot, const SamplerPointer& _sampler) override;
        virtual uint32_t getResourceViewBindlessIndex(uint32_t _slot) const override;
        virtual uint32_t getSamplerBindlessIndex(uint32_t _slot) const override;
        virtual void pushConstants(
            ShaderStageFlags _stageFlags,
            uint32_t _offset,
            uint32_t _size,
            const void* _data
        ) override;

        virtual void copyBuffer(
            const BufferPointer& _srcBuffer,
            const BufferPointer& _dstBuffer,
            const BufferCopyRegionDesc& _region
        ) override;

        virtual void copyTexture(
            const TexturePointer& _srcTexture,
            const TexturePointer& _dstTexture,
            const TextureCopyRegionDesc& _region
        ) override;

        virtual void copyBufferToTexture(
            const BufferPointer& _srcBuffer,
            const TexturePointer& _dstTexture,
            const BufferTextureCopyRegionDesc& _region
        ) override;

        virtual void copyTextureToBuffer(
            const TexturePointer& _srcTexture,
            const BufferPointer& _dstBuffer,
            const BufferTextureCopyRegionDesc& _region
        ) override;
    };

    class D3D12ComputeCommandList final : public ComputeCommandList,
                                          public D3D12CommandListBase
    {
    public:
        D3D12ComputeCommandList(
            D3D12GraphicDevice* _device,
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>&& _commandList
        );

        virtual void* getNativeHandle() const override;
        virtual void setName(const char* _name) override;

        virtual void begin() override;
        virtual void reset() override;
        virtual void end() override;

        virtual void resourceBarrier(
            const GraphicResourcePointer& _resource,
            GraphicResourceState _prevState,
            GraphicResourceState _nextState
        ) override;

        virtual void bindResourceView(uint32_t _slot, const ResourceViewPointer& _resourceView) override;
        virtual void bindSampler(uint32_t _slot, const SamplerPointer& _sampler) override;
        virtual uint32_t getResourceViewBindlessIndex(uint32_t _slot) const override;
        virtual uint32_t getSamplerBindlessIndex(uint32_t _slot) const override;
        virtual void pushConstants(
            ShaderStageFlags _stageFlags,
            uint32_t _offset,
            uint32_t _size,
            const void* _data
        ) override;

        virtual void copyBuffer(
            const BufferPointer& _srcBuffer,
            const BufferPointer& _dstBuffer,
            const BufferCopyRegionDesc& _region
        ) override;

        virtual void copyTexture(
            const TexturePointer& _srcTexture,
            const TexturePointer& _dstTexture,
            const TextureCopyRegionDesc& _region
        ) override;

        virtual void copyBufferToTexture(
            const BufferPointer& _srcBuffer,
            const TexturePointer& _dstTexture,
            const BufferTextureCopyRegionDesc& _region
        ) override;

        virtual void copyTextureToBuffer(
            const TexturePointer& _srcTexture,
            const BufferPointer& _dstBuffer,
            const BufferTextureCopyRegionDesc& _region
        ) override;

        virtual void setPipeline(const ComputePipelinePointer& _pipeline) override;
        virtual void dispatch(uint32_t _threadGroupsX, uint32_t _threadGroupsY, uint32_t _threadGroupsZ) override;
    };

    class D3D12GraphicCommandList final : public GraphicCommandList,
                                          public D3D12CommandListBase
    {
    public:
        D3D12GraphicCommandList(
            D3D12GraphicDevice* _device,
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>&& _commandList
        );

        virtual void* getNativeHandle() const override;
        virtual void setName(const char* _name) override;

        virtual void begin() override;
        virtual void reset() override;
        virtual void end() override;

        virtual void resourceBarrier(
            const GraphicResourcePointer& _resource,
            GraphicResourceState _prevState,
            GraphicResourceState _nextState
        ) override;

        virtual void bindResourceView(uint32_t _slot, const ResourceViewPointer& _resourceView) override;
        virtual void bindSampler(uint32_t _slot, const SamplerPointer& _sampler) override;
        virtual uint32_t getResourceViewBindlessIndex(uint32_t _slot) const override;
        virtual uint32_t getSamplerBindlessIndex(uint32_t _slot) const override;
        virtual void pushConstants(
            ShaderStageFlags _stageFlags,
            uint32_t _offset,
            uint32_t _size,
            const void* _data
        ) override;

        virtual void copyBuffer(
            const BufferPointer& _srcBuffer,
            const BufferPointer& _dstBuffer,
            const BufferCopyRegionDesc& _region
        ) override;

        virtual void copyTexture(
            const TexturePointer& _srcTexture,
            const TexturePointer& _dstTexture,
            const TextureCopyRegionDesc& _region
        ) override;

        virtual void copyBufferToTexture(
            const BufferPointer& _srcBuffer,
            const TexturePointer& _dstTexture,
            const BufferTextureCopyRegionDesc& _region
        ) override;

        virtual void copyTextureToBuffer(
            const TexturePointer& _srcTexture,
            const BufferPointer& _dstBuffer,
            const BufferTextureCopyRegionDesc& _region
        ) override;

        virtual void setPipeline(const ComputePipelinePointer& _pipeline) override;
        virtual void dispatch(uint32_t _threadGroupsX, uint32_t _threadGroupsY, uint32_t _threadGroupsZ) override;

        virtual void beginRendering(const RenderingDesc& _desc) override;
        virtual void endRendering() override;

        virtual void setPipeline(const GraphicPipelinePointer& _pipeline) override;
        virtual void setViewport(const ViewportDesc& _viewportDesc) override;
        virtual void setScissorRect(const ScissorRectDesc& _scissorRect) override;
        virtual void setVertexBuffer(
            uint32_t _slot,
            const BufferPointer& _buffer,
            uint32_t _stride,
            uint32_t _offset
        ) override;
        virtual void setIndexBuffer(
            const BufferPointer& _buffer,
            GraphicResourceFormat _format,
            uint32_t _offset
        ) override;

        virtual void draw(
            uint32_t _vertexCount,
            uint32_t _instanceCount,
            uint32_t _firstVertex,
            uint32_t _firstInstance
        ) override;

        virtual void drawIndexed(
            uint32_t _indexCount,
            uint32_t _instanceCount,
            uint32_t _firstIndex,
            int32_t _vertexOffset,
            uint32_t _firstInstance
        ) override;
    };
}
