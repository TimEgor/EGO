#pragma once

#include "EgoD3D12RenderHardware/Common/D3D12DescriptorAllocator.h"

namespace ego::gpu::d3d12
{
    class D3D12Resource
    {
    public:
        virtual ID3D12Resource* getD3D12Resource() const = 0;

        D3D12_RESOURCE_DESC getD3D12ResourceDesc() const
        {
            ID3D12Resource* resource = getD3D12Resource();
            return resource ? resource->GetDesc() : D3D12_RESOURCE_DESC{};
        }

    protected:
        ~D3D12Resource() = default;
    };

    class D3D12Descriptor
    {
    public:
        virtual D3D12DescriptorIndex getDescriptorIndex() const = 0;
        virtual D3D12_CPU_DESCRIPTOR_HANDLE getCpuDescriptorHandle() const = 0;
        virtual D3D12_GPU_DESCRIPTOR_HANDLE getGpuDescriptorHandle() const = 0;

    protected:
        ~D3D12Descriptor() = default;
    };
}
