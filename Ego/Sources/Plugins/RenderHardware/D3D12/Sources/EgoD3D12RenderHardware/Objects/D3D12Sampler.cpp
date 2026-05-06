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
    return reinterpret_cast<void*>(m_descriptorIndex);
}

void ego::gpu::d3d12::D3D12Sampler::setName(const char*)
{}

ego::gpu::d3d12::D3D12DescriptorIndex ego::gpu::d3d12::D3D12Sampler::getDescriptorIndex() const
{
    return m_descriptorIndex;
}
