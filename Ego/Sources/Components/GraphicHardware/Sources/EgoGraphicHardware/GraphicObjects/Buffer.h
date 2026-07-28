#pragma once

#include "Format.h"
#include "GraphicResource.h"
#include "ResourceView.h"

namespace ego::gpu
{
    enum BufferUsage
    {
        GpuBufferUsageVertexBuffer = GraphicResourceMaxUsage << 1,
        GpuBufferUsageIndexBuffer = GraphicResourceMaxUsage << 2,
        GpuBufferUsageConstantBuffer = GraphicResourceMaxUsage << 3,
        GpuBufferUsageIndirectBuffer = GraphicResourceMaxUsage << 4
    };

    struct BufferDesc final : GraphicResourceDesc
    {
        uint32_t m_size = 0;
        uint32_t m_stride = 0;
    };

    class Buffer : public GraphicResource
    {
    public:
        Buffer(const BufferDesc& _desc);

        virtual void* map(uint32_t _offset = 0, uint32_t _size = 0) = 0;
        virtual void unmap(uint32_t _offset = 0, uint32_t _size = 0) = 0;

        const BufferDesc& getDesc() const;

        EGO_GRAPHIC_RESOURCE(Buffer, GraphicResource);

    private:
        BufferDesc m_desc;
    };

    EGO_INTRUSIVE_POINTER(Buffer);

    struct BufferViewDesc final
    {
        GraphicResourceViewType m_type = GraphicResourceViewType::ShaderResource;
        GraphicResourceFormat m_format = GraphicResourceFormat::Undefined;
        uint64_t m_offset = 0;
        uint64_t m_size = 0;
        uint32_t m_stride = 0;
    };

    class BufferView : public ResourceView
    {
    public:
        BufferView(const BufferPointer& _buffer, const BufferViewDesc& _desc);

        const BufferViewDesc& getDesc() const;
        GraphicResourceViewType getViewType() const override;

    private:
        BufferViewDesc m_desc;
    };

    EGO_INTRUSIVE_POINTER(BufferView);
} // namespace ego::gpu
