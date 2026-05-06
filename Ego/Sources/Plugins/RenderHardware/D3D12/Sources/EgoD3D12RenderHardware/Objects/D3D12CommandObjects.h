#pragma once

#include <wrl/client.h>

#include "EgoEngine/RenderHardware/GraphicObjects/CommandList.h"
#include "EgoEngine/RenderHardware/GraphicObjects/CommandQueue.h"
#include "EgoEngine/RenderHardware/GraphicObjects/Fence.h"

#include "EgoD3D12RenderHardware/Common/D3D12Utils.h"

namespace ego::gpu::d3d12
{
    class D3D12GraphicDevice;
    class D3D12BindingLayout;
    class D3D12BindingSet;
    class D3D12GraphicPipeline;
    class D3D12ComputePipeline;

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
        void bindBindingSetInternal(uint32_t _slot, const BindingSetPointer& _bindingSet);
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
    };

    class D3D12Fence final : public Fence
    {
    public:
        D3D12Fence(
            D3D12GraphicDevice* _device,
            FenceValue _initialValue,
            Microsoft::WRL::ComPtr<ID3D12Fence>&& _fence
        );
        ~D3D12Fence() override;

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        FenceValue getCompletedValue() const override;
        void waitValue(FenceValue _value) override;

        ID3D12Fence* getFence() const;

    private:
        D3D12GraphicDevice* m_device = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
        void* m_eventHandle = nullptr;
    };

    class D3D12CommandQueue final : public CommandQueue
    {
    public:
        D3D12CommandQueue(
            D3D12GraphicDevice* _device,
            const CommandQueueDesc& _desc,
            Microsoft::WRL::ComPtr<ID3D12CommandQueue>&& _queue
        );
        ~D3D12CommandQueue() override;

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        void execute(const CommandListPointer& _commandList) override;
        void execute(const std::vector<CommandListPointer>& _commandLists) override;

        void signal(const FencePointer& _fence, Fence::FenceValue _value) override;
        void wait(const FencePointer& _fence, Fence::FenceValue _value) override;
        void waitIdle() override;

        ID3D12CommandQueue* getQueue() const;

    private:
        D3D12GraphicDevice* m_device = nullptr;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_queue;
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

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        void begin() override;
        void reset() override;
        void end() override;

        void resourceBarrier(
            const GraphicResourcePointer& _resource,
            GraphicResourceState _prevState,
            GraphicResourceState _nextState
        ) override;
        void bindBindingSet(uint32_t _slot, const BindingSetPointer& _bindingSet) override;
        void pushConstants(ShaderStageFlags _stageFlags, uint32_t _offset, uint32_t _size, const void* _data) override;

        void copyBuffer(
            const BufferPointer& _srcBuffer,
            const BufferPointer& _dstBuffer,
            const BufferCopyRegionDesc& _region
        ) override;
        void copyTexture(
            const TexturePointer& _srcTexture,
            const TexturePointer& _dstTexture,
            const TextureCopyRegionDesc& _region
        ) override;
        void copyBufferToTexture(
            const BufferPointer& _srcBuffer,
            const TexturePointer& _dstTexture,
            const BufferTextureCopyRegionDesc& _region
        ) override;
        void copyTextureToBuffer(
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

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        void begin() override;
        void reset() override;
        void end() override;

        void resourceBarrier(
            const GraphicResourcePointer& _resource,
            GraphicResourceState _prevState,
            GraphicResourceState _nextState
        ) override;
        void bindBindingSet(uint32_t _slot, const BindingSetPointer& _bindingSet) override;
        void pushConstants(ShaderStageFlags _stageFlags, uint32_t _offset, uint32_t _size, const void* _data) override;

        void copyBuffer(
            const BufferPointer& _srcBuffer,
            const BufferPointer& _dstBuffer,
            const BufferCopyRegionDesc& _region
        ) override;
        void copyTexture(
            const TexturePointer& _srcTexture,
            const TexturePointer& _dstTexture,
            const TextureCopyRegionDesc& _region
        ) override;
        void copyBufferToTexture(
            const BufferPointer& _srcBuffer,
            const TexturePointer& _dstTexture,
            const BufferTextureCopyRegionDesc& _region
        ) override;
        void copyTextureToBuffer(
            const TexturePointer& _srcTexture,
            const BufferPointer& _dstBuffer,
            const BufferTextureCopyRegionDesc& _region
        ) override;

        void setPipeline(const ComputePipelinePointer& _pipeline) override;
        void dispatch(uint32_t _threadGroupsX, uint32_t _threadGroupsY, uint32_t _threadGroupsZ) override;
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

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        void begin() override;
        void reset() override;
        void end() override;

        void resourceBarrier(
            const GraphicResourcePointer& _resource,
            GraphicResourceState _prevState,
            GraphicResourceState _nextState
        ) override;
        void bindBindingSet(uint32_t _slot, const BindingSetPointer& _bindingSet) override;
        void pushConstants(ShaderStageFlags _stageFlags, uint32_t _offset, uint32_t _size, const void* _data) override;

        void copyBuffer(
            const BufferPointer& _srcBuffer,
            const BufferPointer& _dstBuffer,
            const BufferCopyRegionDesc& _region
        ) override;
        void copyTexture(
            const TexturePointer& _srcTexture,
            const TexturePointer& _dstTexture,
            const TextureCopyRegionDesc& _region
        ) override;
        void copyBufferToTexture(
            const BufferPointer& _srcBuffer,
            const TexturePointer& _dstTexture,
            const BufferTextureCopyRegionDesc& _region
        ) override;
        void copyTextureToBuffer(
            const TexturePointer& _srcTexture,
            const BufferPointer& _dstBuffer,
            const BufferTextureCopyRegionDesc& _region
        ) override;

        void setPipeline(const ComputePipelinePointer& _pipeline) override;
        void dispatch(uint32_t _threadGroupsX, uint32_t _threadGroupsY, uint32_t _threadGroupsZ) override;

        void beginRendering(const RenderingDesc& _desc) override;
        void endRendering() override;

        void setPipeline(const GraphicPipelinePointer& _pipeline) override;
        void setViewport(const ViewportDesc& _viewportDesc) override;
        void setScissorRect(const ScissorRectDesc& _scissorRect) override;
        void setVertexBuffer(uint32_t _slot, const BufferPointer& _buffer, uint32_t _stride, uint32_t _offset) override;
        void setIndexBuffer(const BufferPointer& _buffer, GraphicResourceFormat _format, uint32_t _offset) override;

        void draw(
            uint32_t _vertexCount,
            uint32_t _instanceCount,
            uint32_t _firstVertex,
            uint32_t _firstInstance
        ) override;
        void drawIndexed(
            uint32_t _indexCount,
            uint32_t _instanceCount,
            uint32_t _firstIndex,
            int32_t _vertexOffset,
            uint32_t _firstInstance
        ) override;
    };
}
