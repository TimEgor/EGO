#pragma once

#include <vector>

#include <wrl/client.h>

#include "D3D12Utils.h"

namespace ego::gpu::d3d12
{
    class D3D12DescriptorAllocator final
    {
    public:
        D3D12DescriptorAllocator() = default;

        bool init(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, D3D12DescriptorIndex _capacity, bool _shaderVisible);

        D3D12DescriptorIndex allocate();
        void release(D3D12DescriptorIndex _index);

        D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle(D3D12DescriptorIndex _index) const;
        D3D12_GPU_DESCRIPTOR_HANDLE getGpuHandle(D3D12DescriptorIndex _index) const;
        D3D12_CPU_DESCRIPTOR_HANDLE getCpuHeapStart() const;
        D3D12_GPU_DESCRIPTOR_HANDLE getGpuHeapStart() const;

        ID3D12DescriptorHeap* getHeap() const;
        bool isShaderVisible() const;
        uint32_t getDescriptorSize() const;
        uint32_t getCapacity() const;

    private:
        bool isValidIndex(D3D12DescriptorIndex _index) const;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;

        std::vector<D3D12DescriptorIndex> m_freeIndices;
        uint32_t m_descriptorSize = 0;
        D3D12DescriptorIndex m_capacity = 0;
        bool m_shaderVisible = false;
    };

    EGO_POINTER(D3D12DescriptorAllocator);
    EGO_WEAK_POINTER(D3D12DescriptorAllocator);
}
