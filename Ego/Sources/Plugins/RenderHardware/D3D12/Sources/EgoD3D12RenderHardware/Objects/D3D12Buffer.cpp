#include "D3D12Buffer.h"

#include <utility>

ego::gpu::d3d12::D3D12Buffer::D3D12Buffer(
    const BufferDesc& _desc,
    Microsoft::WRL::ComPtr<ID3D12Resource>&& _resource
)
    : Buffer(_desc),
      m_resource(std::move(_resource))
{}

void* ego::gpu::d3d12::D3D12Buffer::getNativeHandle() const
{
    return m_resource.Get();
}

void ego::gpu::d3d12::D3D12Buffer::setName(const char* _name)
{
    SetD3D12ObjectName(m_resource.Get(), _name);
}

void* ego::gpu::d3d12::D3D12Buffer::map(uint32_t _offset, uint32_t _size)
{
    if (!m_resource)
    {
        return nullptr;
    }

    void* mappedData = nullptr;
    const BufferDesc& desc = getDesc();

    D3D12_RANGE readRange = {};
    if (desc.m_access & GraphicResourceAccessCpuRead)
    {
        readRange.Begin = static_cast<SIZE_T>(_offset);
        readRange.End = static_cast<SIZE_T>(_size ? _offset + _size : desc.m_size);
    }

    if (FAILED(m_resource->Map(0, (desc.m_access & GraphicResourceAccessCpuRead) ? &readRange : nullptr, &mappedData)))
    {
        return nullptr;
    }

    return static_cast<uint8_t*>(mappedData) + _offset;
}

void ego::gpu::d3d12::D3D12Buffer::unmap(uint32_t _offset, uint32_t _size)
{
    if (!m_resource)
    {
        return;
    }

    const BufferDesc& desc = getDesc();
    D3D12_RANGE writtenRange = {};

    if (desc.m_access & GraphicResourceAccessCpuWrite)
    {
        writtenRange.Begin = static_cast<SIZE_T>(_offset);
        writtenRange.End = static_cast<SIZE_T>(_size ? _offset + _size : desc.m_size);
        m_resource->Unmap(0, &writtenRange);
        return;
    }

    m_resource->Unmap(0, nullptr);
}

ID3D12Resource* ego::gpu::d3d12::D3D12Buffer::getD3D12Resource() const
{
    return m_resource.Get();
}

ego::gpu::d3d12::D3D12BufferView::D3D12BufferView(
    const BufferPointer& _buffer,
    const BufferViewDesc& _desc,
    D3D12DescriptorIndex _descriptorIndex,
    D3D12DescriptorAllocatorPointer& _allocator
)
    : BufferView(_buffer, _desc),
      m_allocator(_allocator),
      m_descriptorIndex(_descriptorIndex)
{}

ego::gpu::d3d12::D3D12BufferView::~D3D12BufferView()
{
    D3D12DescriptorAllocatorPointer allocator = m_allocator.lock();

    if (allocator)
    {
        allocator->release(m_descriptorIndex);
    }
}

void* ego::gpu::d3d12::D3D12BufferView::getNativeHandle() const
{
    return reinterpret_cast<void*>(m_descriptorIndex);
}

void ego::gpu::d3d12::D3D12BufferView::setName(const char*)
{
    
}

ego::gpu::d3d12::D3D12DescriptorIndex ego::gpu::d3d12::D3D12BufferView::getDescriptorIndex() const
{
    return m_descriptorIndex;
}
