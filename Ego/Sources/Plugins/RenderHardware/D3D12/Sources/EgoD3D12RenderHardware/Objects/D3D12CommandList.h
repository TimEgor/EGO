#pragma once

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
            const GraphicResourceReference& _resource,
            GraphicResourceState _prevState,
            GraphicResourceState _nextState
        );

        void pushConstantsInternal(ShaderStageFlags _stageFlags, uint32_t _offset, uint32_t _size, const void* _data);

        void copyBufferInternal(
            const BufferReference& _srcBuffer,
            const BufferReference& _dstBuffer,
            const BufferCopyRegionDesc& _region
        );
        void copyTextureInternal(
            const TextureReference& _srcTexture,
            const TextureReference& _dstTexture,
            const TextureCopyRegionDesc& _region
        );
        void copyBufferToTextureInternal(
            const BufferReference& _srcBuffer,
            const TextureReference& _dstTexture,
            const BufferTextureCopyRegionDesc& _region
        );
        void copyTextureToBufferInternal(
            const TextureReference& _srcTexture,
            const BufferReference& _dstBuffer,
            const BufferTextureCopyRegionDesc& _region
        );

        D3D12GraphicDevice* m_device = nullptr;
        CommandType m_commandType = CommandType::Graphic;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_allocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
        const D3D12BindingLayout* m_currentBindingLayout = nullptr;
        PipelineType m_currentPipelineType = PipelineType::Graphic;
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
            const GraphicResourceReference& _resource,
            GraphicResourceState _prevState,
            GraphicResourceState _nextState
        ) override;

        virtual void pushConstants(
            ShaderStageFlags _stageFlags,
            uint32_t _offset,
            uint32_t _size,
            const void* _data
        ) override;

        virtual void copyBuffer(
            const BufferReference& _srcBuffer,
            const BufferReference& _dstBuffer,
            const BufferCopyRegionDesc& _region
        ) override;

        virtual void copyTexture(
            const TextureReference& _srcTexture,
            const TextureReference& _dstTexture,
            const TextureCopyRegionDesc& _region
        ) override;

        virtual void copyBufferToTexture(
            const BufferReference& _srcBuffer,
            const TextureReference& _dstTexture,
            const BufferTextureCopyRegionDesc& _region
        ) override;

        virtual void copyTextureToBuffer(
            const TextureReference& _srcTexture,
            const BufferReference& _dstBuffer,
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
            const GraphicResourceReference& _resource,
            GraphicResourceState _prevState,
            GraphicResourceState _nextState
        ) override;

        virtual void pushConstants(
            ShaderStageFlags _stageFlags,
            uint32_t _offset,
            uint32_t _size,
            const void* _data
        ) override;

        virtual void copyBuffer(
            const BufferReference& _srcBuffer,
            const BufferReference& _dstBuffer,
            const BufferCopyRegionDesc& _region
        ) override;

        virtual void copyTexture(
            const TextureReference& _srcTexture,
            const TextureReference& _dstTexture,
            const TextureCopyRegionDesc& _region
        ) override;

        virtual void copyBufferToTexture(
            const BufferReference& _srcBuffer,
            const TextureReference& _dstTexture,
            const BufferTextureCopyRegionDesc& _region
        ) override;

        virtual void copyTextureToBuffer(
            const TextureReference& _srcTexture,
            const BufferReference& _dstBuffer,
            const BufferTextureCopyRegionDesc& _region
        ) override;

        virtual void setPipeline(const ComputePipelineReference& _pipeline) override;
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
            const GraphicResourceReference& _resource,
            GraphicResourceState _prevState,
            GraphicResourceState _nextState
        ) override;

        virtual void pushConstants(
            ShaderStageFlags _stageFlags,
            uint32_t _offset,
            uint32_t _size,
            const void* _data
        ) override;

        virtual void copyBuffer(
            const BufferReference& _srcBuffer,
            const BufferReference& _dstBuffer,
            const BufferCopyRegionDesc& _region
        ) override;

        virtual void copyTexture(
            const TextureReference& _srcTexture,
            const TextureReference& _dstTexture,
            const TextureCopyRegionDesc& _region
        ) override;

        virtual void copyBufferToTexture(
            const BufferReference& _srcBuffer,
            const TextureReference& _dstTexture,
            const BufferTextureCopyRegionDesc& _region
        ) override;

        virtual void copyTextureToBuffer(
            const TextureReference& _srcTexture,
            const BufferReference& _dstBuffer,
            const BufferTextureCopyRegionDesc& _region
        ) override;

        virtual void setPipeline(const ComputePipelineReference& _pipeline) override;
        virtual void dispatch(uint32_t _threadGroupsX, uint32_t _threadGroupsY, uint32_t _threadGroupsZ) override;

        virtual void beginRendering(const RenderingDesc& _desc) override;
        virtual void endRendering() override;

        virtual void setPipeline(const GraphicPipelineReference& _pipeline) override;
        virtual void setViewport(const ViewportDesc& _viewportDesc) override;
        virtual void setScissorRect(const ScissorRectDesc& _scissorRect) override;
        virtual void setVertexBuffer(
            uint32_t _slot,
            const BufferReference& _buffer,
            uint32_t _stride,
            uint32_t _offset
        ) override;
        virtual void setIndexBuffer(
            const BufferReference& _buffer,
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
