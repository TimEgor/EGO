#pragma once

#include "EgoGraphicHardware/GraphicObjects/CommandList.h"

#include "EgoD3D12GraphicHardware/Common/D3D12Utils.h"

#include <wrl/client.h>

namespace ego::gpu::d3d12
{
    class D3D12DescriptorFactory;
    class D3D12GraphicDevice;
    class D3D12BindingLayout;
    class D3D12RayTracingPipeline;

    class D3D12CommandListBase
    {
    public:
        D3D12CommandListBase(
            D3D12GraphicDevice* _device,
            const D3D12DescriptorFactory* _descriptorFactory,
            CommandType _commandType,
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>&& _commandList);
        virtual ~D3D12CommandListBase() = default;

        ID3D12GraphicsCommandList* getD3D12CommandList() const;

    protected:
        void beginInternal();
        void resetInternal();
        void endInternal();

        void bindBindlessDescriptorHeapsInternal();
        void resourceBarrierInternal(const GraphicResourceReference& _resource, GraphicResourceState _prevState, GraphicResourceState _nextState);

        void pushConstantsInternal(ShaderStageFlags _stageFlags, uint32_t _offset, uint32_t _size, const void* _data);
        void setRayTracingPipelineInternal(const RayTracingPipelineReference& _pipeline);
        void dispatchRaysInternal(const DispatchRaysDesc& _desc);

        void copyBufferInternal(const BufferReference& _srcBuffer, const BufferReference& _dstBuffer, const BufferCopyRegionDesc& _region);
        void copyTextureInternal(const TextureReference& _srcTexture, const TextureReference& _dstTexture, const TextureCopyRegionDesc& _region);
        void copyBufferToTextureInternal(const BufferReference& _srcBuffer, const TextureReference& _dstTexture, const BufferTextureCopyRegionDesc& _region);
        void copyTextureToBufferInternal(const TextureReference& _srcTexture, const BufferReference& _dstBuffer, const BufferTextureCopyRegionDesc& _region);

        D3D12GraphicDevice* m_device = nullptr;
        const D3D12DescriptorFactory* m_descriptorFactory = nullptr;
        CommandType m_commandType = CommandType::Graphic;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_allocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> m_commandList;
        const D3D12BindingLayout* m_currentBindingLayout = nullptr;
        const D3D12RayTracingPipeline* m_currentRayTracingPipeline = nullptr;
        PipelineType m_currentPipelineType = PipelineType::Graphic;
    };

    class D3D12CopyCommandList final : public CopyCommandList, public D3D12CommandListBase
    {
    public:
        D3D12CopyCommandList(
            D3D12GraphicDevice* _device,
            const D3D12DescriptorFactory* _descriptorFactory,
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>&& _commandList);

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        void begin() override;
        void reset() override;
        void end() override;

        void resourceBarrier(const GraphicResourceReference& _resource, GraphicResourceState _prevState, GraphicResourceState _nextState) override;

        void pushConstants(ShaderStageFlags _stageFlags, uint32_t _offset, uint32_t _size, const void* _data) override;

        void copyBuffer(const BufferReference& _srcBuffer, const BufferReference& _dstBuffer, const BufferCopyRegionDesc& _region) override;

        void copyTexture(const TextureReference& _srcTexture, const TextureReference& _dstTexture, const TextureCopyRegionDesc& _region) override;

        void copyBufferToTexture(const BufferReference& _srcBuffer, const TextureReference& _dstTexture, const BufferTextureCopyRegionDesc& _region) override;

        void copyTextureToBuffer(const TextureReference& _srcTexture, const BufferReference& _dstBuffer, const BufferTextureCopyRegionDesc& _region) override;
    };

    class D3D12ComputeCommandList final : public ComputeCommandList, public D3D12CommandListBase
    {
    public:
        D3D12ComputeCommandList(
            D3D12GraphicDevice* _device,
            const D3D12DescriptorFactory* _descriptorFactory,
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>&& _commandList);

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        void begin() override;
        void reset() override;
        void end() override;

        void resourceBarrier(const GraphicResourceReference& _resource, GraphicResourceState _prevState, GraphicResourceState _nextState) override;

        void pushConstants(ShaderStageFlags _stageFlags, uint32_t _offset, uint32_t _size, const void* _data) override;

        void copyBuffer(const BufferReference& _srcBuffer, const BufferReference& _dstBuffer, const BufferCopyRegionDesc& _region) override;

        void copyTexture(const TextureReference& _srcTexture, const TextureReference& _dstTexture, const TextureCopyRegionDesc& _region) override;

        void copyBufferToTexture(const BufferReference& _srcBuffer, const TextureReference& _dstTexture, const BufferTextureCopyRegionDesc& _region) override;

        void copyTextureToBuffer(const TextureReference& _srcTexture, const BufferReference& _dstBuffer, const BufferTextureCopyRegionDesc& _region) override;

        void setPipeline(const ComputePipelineReference& _pipeline) override;
        void setPipeline(const RayTracingPipelineReference& _pipeline) override;
        void dispatch(uint32_t _threadGroupsX, uint32_t _threadGroupsY, uint32_t _threadGroupsZ) override;
        void dispatchRays(const DispatchRaysDesc& _desc) override;
    };

    class D3D12GraphicCommandList final : public GraphicCommandList, public D3D12CommandListBase
    {
    public:
        D3D12GraphicCommandList(
            D3D12GraphicDevice* _device,
            const D3D12DescriptorFactory* _descriptorFactory,
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>&& _commandList);

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        void begin() override;
        void reset() override;
        void end() override;

        void resourceBarrier(const GraphicResourceReference& _resource, GraphicResourceState _prevState, GraphicResourceState _nextState) override;

        void pushConstants(ShaderStageFlags _stageFlags, uint32_t _offset, uint32_t _size, const void* _data) override;

        void copyBuffer(const BufferReference& _srcBuffer, const BufferReference& _dstBuffer, const BufferCopyRegionDesc& _region) override;

        void copyTexture(const TextureReference& _srcTexture, const TextureReference& _dstTexture, const TextureCopyRegionDesc& _region) override;

        void copyBufferToTexture(const BufferReference& _srcBuffer, const TextureReference& _dstTexture, const BufferTextureCopyRegionDesc& _region) override;

        void copyTextureToBuffer(const TextureReference& _srcTexture, const BufferReference& _dstBuffer, const BufferTextureCopyRegionDesc& _region) override;

        void setPipeline(const ComputePipelineReference& _pipeline) override;
        void setPipeline(const RayTracingPipelineReference& _pipeline) override;
        void dispatch(uint32_t _threadGroupsX, uint32_t _threadGroupsY, uint32_t _threadGroupsZ) override;
        void dispatchRays(const DispatchRaysDesc& _desc) override;

        void beginRendering(const RenderingDesc& _desc) override;
        void endRendering() override;

        void setPipeline(const GraphicPipelineReference& _pipeline) override;
        void setViewport(const ViewportDesc& _viewportDesc) override;
        void setScissorRect(const ScissorRectDesc& _scissorRect) override;
        void setVertexBuffer(uint32_t _slot, const BufferReference& _buffer, uint32_t _stride, uint32_t _offset) override;
        void setIndexBuffer(const BufferReference& _buffer, GraphicResourceFormat _format, uint32_t _offset) override;

        void draw(uint32_t _vertexCount, uint32_t _instanceCount, uint32_t _firstVertex, uint32_t _firstInstance) override;

        void drawIndexed(uint32_t _indexCount, uint32_t _instanceCount, uint32_t _firstIndex, int32_t _vertexOffset, uint32_t _firstInstance) override;
    };
} // namespace ego::gpu::d3d12
