#pragma once

#include "EgoEngine/RenderHardware/GraphicObjects/Sampler.h"

#include "D3D12Resource.h"

namespace ego::gpu::d3d12
{
    class D3D12Sampler final : public Sampler, public D3D12Descriptor
    {
    public:
        D3D12Sampler(
            const SamplerDesc& _desc,
            D3D12DescriptorIndex _descriptorIndex,
            D3D12DescriptorAllocatorPointer& _allocator
        );
        ~D3D12Sampler() override;

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        D3D12DescriptorIndex getDescriptorIndex() const override;

    private:
        D3D12DescriptorAllocatorWeakPointer m_allocator = nullptr;
        D3D12DescriptorIndex m_descriptorIndex = D3D12InvalidDescriptorIndex;
    };
}
