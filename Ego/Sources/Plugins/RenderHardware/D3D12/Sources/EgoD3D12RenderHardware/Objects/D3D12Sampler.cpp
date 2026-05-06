#include "D3D12Sampler.h"

ego::gpu::d3d12::D3D12Sampler::D3D12Sampler(
    const SamplerDesc& _desc,
    D3D12DescriptorIndex _descriptorIndex,
    D3D12DescriptorAllocatorPointer& _allocator
)
    : Sampler(_desc),
      m_allocator(_allocator),
      m_descriptorIndex(_descriptorIndex)
{}

ego::gpu::d3d12::D3D12Sampler::~D3D12Sampler()
{
    D3D12DescriptorAllocatorPointer allocator = m_allocator.lock();

    if (allocator)
    {
        allocator->release(m_descriptorIndex);
    }
}

void* ego::gpu::d3d12::D3D12Sampler::getNativeHandle() const
{
    return reinterpret_cast<void*>(static_cast<uintptr_t>(m_descriptorIndex));
}

void ego::gpu::d3d12::D3D12Sampler::setName(const char*)
{}

ego::gpu::d3d12::D3D12DescriptorIndex ego::gpu::d3d12::D3D12Sampler::getDescriptorIndex() const
{
    return m_descriptorIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE ego::gpu::d3d12::D3D12Sampler::getCpuDescriptorHandle() const
{
    D3D12DescriptorAllocatorPointer allocator = m_allocator.lock();
    return allocator ? allocator->getCpuHandle(m_descriptorIndex) : D3D12_CPU_DESCRIPTOR_HANDLE{};
}

D3D12_GPU_DESCRIPTOR_HANDLE ego::gpu::d3d12::D3D12Sampler::getGpuDescriptorHandle() const
{
    D3D12DescriptorAllocatorPointer allocator = m_allocator.lock();
    return allocator ? allocator->getGpuHandle(m_descriptorIndex) : D3D12_GPU_DESCRIPTOR_HANDLE{};
}

uint32_t ego::gpu::d3d12::D3D12Sampler::getBindlessIndex() const
{
    return m_descriptorIndex;
}
