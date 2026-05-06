#include "Buffer.h"

ego::gpu::Buffer::Buffer(const BufferDesc& _desc)
    : m_desc(_desc)
{
}

const ego::gpu::BufferDesc& ego::gpu::Buffer::getDesc() const
{
    return m_desc;
}

ego::gpu::BufferView::BufferView(const BufferPointer& _buffer, const BufferViewDesc& _desc)
    : ResourceView(_buffer),
    m_desc(_desc)
{}

const ego::gpu::BufferViewDesc& ego::gpu::BufferView::getDesc() const
{
    return m_desc;
}

ego::gpu::GraphicResourceViewType ego::gpu::BufferView::getViewType() const
{
    return m_desc.m_type;
}