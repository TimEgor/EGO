#pragma once

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Sampler.h"

#include "D3D12Resource.h"

namespace ego::gpu::d3d12
{
    class D3D12Sampler final : public Sampler, public D3D12Descriptor
    {
    public:
        D3D12Sampler(const SamplerDesc& _desc, D3D12DescriptorIndex _descriptorIndex, D3D12DescriptorAllocatorPointer& _allocator);
        ~D3D12Sampler() override;

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        D3D12DescriptorIndex getDescriptorIndex() const override;
        D3D12_CPU_DESCRIPTOR_HANDLE getCpuDescriptorHandle() const override;
        D3D12_GPU_DESCRIPTOR_HANDLE getGpuDescriptorHandle() const override;
        uint32_t getBindlessIndex() const override;

    private:
        D3D12DescriptorAllocatorWeakPointer m_allocator = nullptr;
        D3D12DescriptorIndex m_descriptorIndex = D3D12InvalidDescriptorIndex;
    };
} // namespace ego::gpu::d3d12
