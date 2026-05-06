#pragma once

#include "EgoD3D12RenderHardware/Common/D3D12DescriptorAllocator.h"

namespace ego::gpu::d3d12
{
    class D3D12Resource
    {
    public:
        virtual ID3D12Resource* getD3D12Resource() const = 0;

    protected:
        ~D3D12Resource() = default;
    };

    class D3D12Descriptor
    {
    public:
        virtual D3D12DescriptorIndex getDescriptorIndex() const = 0;

    protected:
        ~D3D12Descriptor() = default;
    };
}
