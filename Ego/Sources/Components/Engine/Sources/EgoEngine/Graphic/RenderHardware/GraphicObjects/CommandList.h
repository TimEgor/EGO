#pragma once

#include <cstdint>
#include <vector>

#include "EgoMath/Vector.h"

#include "BindingLayout.h"
#include "Buffer.h"
#include "Texture.h"
#include "Pipeline.h"

namespace ego::gpu
{
    struct ViewportDesc final
    {
        float m_x = 0.0f;
        float m_y = 0.0f;
        float m_width = 0.0f;
        float m_height = 0.0f;
        float m_minDepth = 0.0f;
        float m_maxDepth = 1.0f;
    };

    struct ScissorRectDesc final
    {
        int32_t m_left = 0;
        int32_t m_top = 0;
        int32_t m_right = 0;
        int32_t m_bottom = 0;
    };

    enum class AttachmentLoadOperation
    {
        Load,
        Clear,
        DontCare
    };

    enum class AttachmentStoreOperation
    {
        Store,
        DontCare
    };

    struct BufferCopyRegionDesc final
    {
        uint64_t m_srcOffset = 0;
        uint64_t m_dstOffset = 0;
        uint64_t m_size = 0;
    };

    struct TextureCopyRegionDesc final
    {
        UInt32Vector3 m_extent = UInt32Vector3Zero;
        UInt32Vector3 m_srcOffset = UInt32Vector3Zero;
        UInt32Vector3 m_dstOffset = UInt32Vector3Zero;
        uint32_t m_srcMipLevel = 0;
        uint32_t m_dstMipLevel = 0;
        uint32_t m_srcArrayLayer = 0;
        uint32_t m_dstArrayLayer = 0;
    };

    struct BufferTextureCopyRegionDesc final
    {
        uint64_t m_bufferOffset = 0;
        uint32_t m_bufferRowPitch = 0;
        uint32_t m_bufferSlicePitch = 0;
        UInt32Vector3 m_textureOffset = UInt32Vector3Zero;
        UInt32Vector3 m_textureExtent = UInt32Vector3Zero;
        uint32_t m_mipLevel = 0;
        uint32_t m_arrayLayer = 0;
    };

    struct ColorAttachmentDesc final
    {
        TextureViewReference m_view = nullptr;
        AttachmentLoadOperation m_loadOperation = AttachmentLoadOperation::Load;
        AttachmentStoreOperation m_storeOperation = AttachmentStoreOperation::Store;
        FloatVector4 m_clearValue = FloatVector4Zero;
    };

    struct DepthStencilAttachmentDesc final
    {
        TextureViewReference m_view = nullptr;
        AttachmentLoadOperation m_depthLoadOperation = AttachmentLoadOperation::Load;
        AttachmentStoreOperation m_depthStoreOperation = AttachmentStoreOperation::Store;
        AttachmentLoadOperation m_stencilLoadOperation = AttachmentLoadOperation::Load;
        AttachmentStoreOperation m_stencilStoreOperation = AttachmentStoreOperation::Store;
        float m_clearDepth = 1.0f;
        uint32_t m_clearStencil = 0;
    };

    struct RenderingDesc final
    {
        std::vector<ColorAttachmentDesc> m_colorAttachments;
        DepthStencilAttachmentDesc m_depthStencilAttachment;
        Texture2DSize m_renderArea = UInt32Vector2Zero;
    };

    enum class CommandType
    {
        Graphic,
        Compute,
        Copy
    };

    class CommandList : public GraphicObject
    {
    public:
        CommandList() = default;

        virtual void begin() = 0;
        virtual void reset() = 0;
        virtual void end() = 0;

        virtual CommandType getCommandType() const = 0;

        virtual void resourceBarrier(
            const GraphicResourceReference& _resource,
            GraphicResourceState _prevState,
            GraphicResourceState _nextState
        ) = 0;

        virtual void pushConstants(
            ShaderStageFlags _stageFlags,
            uint32_t _offset,
            uint32_t _size,
            const void* _data
        ) = 0;

        virtual void copyBuffer(
            const BufferReference& _srcBuffer,
            const BufferReference& _dstBuffer,
            const BufferCopyRegionDesc& _region
        ) = 0;
        virtual void copyTexture(
            const TextureReference& _srcTexture,
            const TextureReference& _dstTexture,
            const TextureCopyRegionDesc& _region
        ) = 0;
        virtual void copyBufferToTexture(
            const BufferReference& _srcBuffer,
            const TextureReference& _dstTexture,
            const BufferTextureCopyRegionDesc& _region
        ) = 0;
        virtual void copyTextureToBuffer(
            const TextureReference& _srcTexture,
            const BufferReference& _dstBuffer,
            const BufferTextureCopyRegionDesc& _region
        ) = 0;
    };

    EGO_REFERENCE(CommandList);

    class CopyCommandList : public CommandList
    {
    public:
        CopyCommandList() = default;

        CommandType getCommandType() const override;
    };

    EGO_REFERENCE(CopyCommandList);

    class ComputeCommandList : public CopyCommandList
    {
    public:
        ComputeCommandList() = default;

        virtual void setPipeline(const ComputePipelineReference& _pipeline) = 0;
        virtual void dispatch(uint32_t _threadGroupsX, uint32_t _threadGroupsY, uint32_t _threadGroupsZ) = 0;

        CommandType getCommandType() const override;
    };

    EGO_REFERENCE(ComputeCommandList);

    class GraphicCommandList : public ComputeCommandList
    {
    public:
        GraphicCommandList() = default;

        virtual void beginRendering(const RenderingDesc& _desc) = 0;
        virtual void endRendering() = 0;

        virtual void setPipeline(const GraphicPipelineReference& _pipeline) = 0;
        virtual void setViewport(const ViewportDesc& _viewportDesc) = 0;
        virtual void setScissorRect(const ScissorRectDesc& _scissorRect) = 0;
        virtual void setVertexBuffer(
            uint32_t _slot,
            const BufferReference& _buffer,
            uint32_t _stride,
            uint32_t _offset
        ) = 0;
        virtual void setIndexBuffer(const BufferReference& _buffer, GraphicResourceFormat _format, uint32_t _offset) = 0;

        virtual void draw(
            uint32_t _vertexCount,
            uint32_t _instanceCount = 1,
            uint32_t _firstVertex = 0,
            uint32_t _firstInstance = 0
        ) = 0;

        virtual void drawIndexed(
            uint32_t _indexCount,
            uint32_t _instanceCount = 1,
            uint32_t _firstIndex = 0,
            int32_t _vertexOffset = 0,
            uint32_t _firstInstance = 0
        ) = 0;

        CommandType getCommandType() const override;
    };

    EGO_REFERENCE(GraphicCommandList);
}
