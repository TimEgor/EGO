#include "D3D12DescriptorAllocator.h"

bool ego::gpu::d3d12::D3D12DescriptorAllocator::init(
    ID3D12Device* _device,
    D3D12_DESCRIPTOR_HEAP_TYPE _type,
    D3D12DescriptorIndex _capacity,
    bool _shaderVisible
)
{
    if (!_device || !_capacity)
    {
        return false;
    }

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = _type;
    heapDesc.NumDescriptors = _capacity;
    heapDesc.Flags = _shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    heapDesc.NodeMask = 0;

    if (FAILED(_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap))))
    {
        return false;
    }

    m_descriptorSize = _device->GetDescriptorHandleIncrementSize(_type);
    m_capacity = _capacity;
    m_shaderVisible = _shaderVisible;

    m_freeIndices.clear();
    m_freeIndices.reserve(_capacity);

    for (D3D12DescriptorIndex index = _capacity; index > 0; --index)
    {
        m_freeIndices.push_back(index - 1);
    }

    return true;
}

ego::gpu::d3d12::D3D12DescriptorIndex ego::gpu::d3d12::D3D12DescriptorAllocator::allocate()
{
    if (m_freeIndices.empty())
    {
        return D3D12InvalidDescriptorIndex;
    }

    const D3D12DescriptorIndex index = m_freeIndices.back();
    m_freeIndices.pop_back();

    return index;
}

void ego::gpu::d3d12::D3D12DescriptorAllocator::release(D3D12DescriptorIndex _index)
{
    if (!isValidIndex(_index))
    {
        return;
    }

    m_freeIndices.push_back(_index);
}

D3D12_CPU_DESCRIPTOR_HANDLE ego::gpu::d3d12::D3D12DescriptorAllocator::getCpuHandle(
    D3D12DescriptorIndex _index
) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = {};
    if (!m_heap || !isValidIndex(_index))
    {
        return handle;
    }

    handle = m_heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(_index * m_descriptorSize);
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE ego::gpu::d3d12::D3D12DescriptorAllocator::getGpuHandle(
    D3D12DescriptorIndex _index
) const
{
    D3D12_GPU_DESCRIPTOR_HANDLE handle = {};
    if (!m_heap || !m_shaderVisible || !isValidIndex(_index))
    {
        return handle;
    }

    handle = m_heap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<UINT64>(_index * m_descriptorSize);
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE ego::gpu::d3d12::D3D12DescriptorAllocator::getCpuHeapStart() const
{
    return m_heap ? m_heap->GetCPUDescriptorHandleForHeapStart() : D3D12_CPU_DESCRIPTOR_HANDLE{};
}

D3D12_GPU_DESCRIPTOR_HANDLE ego::gpu::d3d12::D3D12DescriptorAllocator::getGpuHeapStart() const
{
    if (!m_heap || !m_shaderVisible)
    {
        return D3D12_GPU_DESCRIPTOR_HANDLE{};
    }

    return m_heap->GetGPUDescriptorHandleForHeapStart();
}

ID3D12DescriptorHeap* ego::gpu::d3d12::D3D12DescriptorAllocator::getHeap() const
{
    return m_heap.Get();
}

bool ego::gpu::d3d12::D3D12DescriptorAllocator::isShaderVisible() const
{
    return m_shaderVisible;
}

uint32_t ego::gpu::d3d12::D3D12DescriptorAllocator::getDescriptorSize() const
{
    return m_descriptorSize;
}

uint32_t ego::gpu::d3d12::D3D12DescriptorAllocator::getCapacity() const
{
    return m_capacity;
}

bool ego::gpu::d3d12::D3D12DescriptorAllocator::isValidIndex(D3D12DescriptorIndex _index) const
{
    return _index != D3D12InvalidDescriptorIndex && _index < m_capacity;
}
