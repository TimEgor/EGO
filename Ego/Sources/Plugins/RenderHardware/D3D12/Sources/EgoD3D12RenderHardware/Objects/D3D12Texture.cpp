#include "D3D12Texture.h"

ego::gpu::d3d12::D3D12Texture2D::D3D12Texture2D(
    const Texture2DDesc& _desc,
    Microsoft::WRL::ComPtr<ID3D12Resource>&& _resource
)
    : Texture2D(_desc),
      m_resource(std::move(_resource))
{}

void* ego::gpu::d3d12::D3D12Texture2D::getNativeHandle() const
{
    return m_resource.Get();
}

void ego::gpu::d3d12::D3D12Texture2D::setName(const char* _name)
{
    SetD3D12ObjectName(m_resource.Get(), _name);
}

ID3D12Resource* ego::gpu::d3d12::D3D12Texture2D::getD3D12Resource() const
{
    return m_resource.Get();
}

ego::gpu::d3d12::D3D12TextureView::D3D12TextureView(
    const TexturePointer& _texture,
    const TextureViewDesc& _desc,
    D3D12DescriptorIndex _descriptorIndex,
    D3D12DescriptorAllocatorPointer& _allocator
)
    : TextureView(_texture, _desc),
      m_descriptorIndex(_descriptorIndex),
      m_allocator(_allocator)
{}

ego::gpu::d3d12::D3D12TextureView::~D3D12TextureView()
{
    D3D12DescriptorAllocatorPointer allocator = m_allocator.lock();

    if (allocator)
    {
        allocator->release(m_descriptorIndex);
    }
}

void* ego::gpu::d3d12::D3D12TextureView::getNativeHandle() const
{
    return reinterpret_cast<void*>(static_cast<uintptr_t>(m_descriptorIndex));
}

void ego::gpu::d3d12::D3D12TextureView::setName(const char*)
{}

ego::gpu::d3d12::D3D12DescriptorIndex ego::gpu::d3d12::D3D12TextureView::getDescriptorIndex() const
{
    return m_descriptorIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE ego::gpu::d3d12::D3D12TextureView::getCpuDescriptorHandle() const
{
    D3D12DescriptorAllocatorPointer allocator = m_allocator.lock();
    return allocator ? allocator->getCpuHandle(m_descriptorIndex) : D3D12_CPU_DESCRIPTOR_HANDLE{};
}

D3D12_GPU_DESCRIPTOR_HANDLE ego::gpu::d3d12::D3D12TextureView::getGpuDescriptorHandle() const
{
    D3D12DescriptorAllocatorPointer allocator = m_allocator.lock();
    return allocator ? allocator->getGpuHandle(m_descriptorIndex) : D3D12_GPU_DESCRIPTOR_HANDLE{};
}

uint32_t ego::gpu::d3d12::D3D12TextureView::getBindlessIndex() const
{
    return m_descriptorIndex;
}
