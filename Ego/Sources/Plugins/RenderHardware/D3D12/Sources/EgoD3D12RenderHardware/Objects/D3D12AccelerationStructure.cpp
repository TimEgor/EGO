#include "D3D12AccelerationStructure.h"

ego::gpu::d3d12::D3D12GeometryAccelerationStructure::D3D12GeometryAccelerationStructure(const BufferReference& _buffer)
    : m_buffer(_buffer)
{
}

void* ego::gpu::d3d12::D3D12GeometryAccelerationStructure::getNativeHandle() const
{
    return getD3D12Resource();
}

void ego::gpu::d3d12::D3D12GeometryAccelerationStructure::setName(const char* _name)
{
    SetD3D12ObjectName(getD3D12Resource(), _name);
}

const ego::gpu::BufferReference& ego::gpu::d3d12::D3D12GeometryAccelerationStructure::getBuffer() const
{
    return m_buffer;
}

ID3D12Resource* ego::gpu::d3d12::D3D12GeometryAccelerationStructure::getD3D12Resource() const
{
    D3D12Buffer* d3d12Buffer = m_buffer ? static_cast<D3D12Buffer*>(m_buffer.getObject()) : nullptr;
    return d3d12Buffer ? d3d12Buffer->getD3D12Resource() : nullptr;
}

D3D12_GPU_VIRTUAL_ADDRESS ego::gpu::d3d12::D3D12GeometryAccelerationStructure::getGpuVirtualAddress() const
{
    ID3D12Resource* resource = getD3D12Resource();
    return resource ? resource->GetGPUVirtualAddress() : 0;
}

ego::gpu::d3d12::D3D12InstanceAccelerationStructure::D3D12InstanceAccelerationStructure(const BufferReference& _buffer)
    : m_buffer(_buffer)
{
}

void* ego::gpu::d3d12::D3D12InstanceAccelerationStructure::getNativeHandle() const
{
    return getD3D12Resource();
}

void ego::gpu::d3d12::D3D12InstanceAccelerationStructure::setName(const char* _name)
{
    SetD3D12ObjectName(getD3D12Resource(), _name);
}

const ego::gpu::BufferReference& ego::gpu::d3d12::D3D12InstanceAccelerationStructure::getBuffer() const
{
    return m_buffer;
}

ID3D12Resource* ego::gpu::d3d12::D3D12InstanceAccelerationStructure::getD3D12Resource() const
{
    D3D12Buffer* d3d12Buffer = m_buffer ? static_cast<D3D12Buffer*>(m_buffer.getObject()) : nullptr;
    return d3d12Buffer ? d3d12Buffer->getD3D12Resource() : nullptr;
}

D3D12_GPU_VIRTUAL_ADDRESS ego::gpu::d3d12::D3D12InstanceAccelerationStructure::getGpuVirtualAddress() const
{
    ID3D12Resource* resource = getD3D12Resource();
    return resource ? resource->GetGPUVirtualAddress() : 0;
}

ego::gpu::d3d12::D3D12AccelerationStructureView::D3D12AccelerationStructureView(
    const InstanceAccelerationStructureReference& _accelerationStructure,
    D3D12DescriptorIndex _descriptorIndex,
    D3D12DescriptorAllocatorPointer& _allocator)
    : AccelerationStructureView(_accelerationStructure),
      m_allocator(_allocator),
      m_descriptorIndex(_descriptorIndex)
{
}

ego::gpu::d3d12::D3D12AccelerationStructureView::~D3D12AccelerationStructureView()
{
    D3D12DescriptorAllocatorPointer allocator = m_allocator.lock();

    if (allocator)
    {
        allocator->release(m_descriptorIndex);
    }
}

void* ego::gpu::d3d12::D3D12AccelerationStructureView::getNativeHandle() const
{
    return reinterpret_cast<void*>(static_cast<uintptr_t>(m_descriptorIndex));
}

void ego::gpu::d3d12::D3D12AccelerationStructureView::setName(const char*) {}

ego::gpu::d3d12::D3D12DescriptorIndex ego::gpu::d3d12::D3D12AccelerationStructureView::getDescriptorIndex() const
{
    return m_descriptorIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE ego::gpu::d3d12::D3D12AccelerationStructureView::getCpuDescriptorHandle() const
{
    D3D12DescriptorAllocatorPointer allocator = m_allocator.lock();
    return allocator ? allocator->getCpuHandle(m_descriptorIndex) : D3D12_CPU_DESCRIPTOR_HANDLE{};
}

D3D12_GPU_DESCRIPTOR_HANDLE ego::gpu::d3d12::D3D12AccelerationStructureView::getGpuDescriptorHandle() const
{
    D3D12DescriptorAllocatorPointer allocator = m_allocator.lock();
    return allocator ? allocator->getGpuHandle(m_descriptorIndex) : D3D12_GPU_DESCRIPTOR_HANDLE{};
}

uint32_t ego::gpu::d3d12::D3D12AccelerationStructureView::getBindlessIndex() const
{
    return m_descriptorIndex;
}
