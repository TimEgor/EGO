#pragma once

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Buffer.h"

#include "D3D12Resource.h"

namespace ego::gpu::d3d12
{
    class D3D12GraphicDevice;

    class D3D12Buffer final : public Buffer,
                              public D3D12Resource
    {
    public:
        D3D12Buffer(
            const BufferDesc& _desc,
            Microsoft::WRL::ComPtr<ID3D12Resource>&& _resource
        );

        virtual void* getNativeHandle() const override;
        virtual void setName(const char* _name) override;

        virtual void* map(uint32_t _offset = 0, uint32_t _size = 0) override;
        virtual void unmap(uint32_t _offset = 0, uint32_t _size = 0) override;

        virtual ID3D12Resource* getD3D12Resource() const override;

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
    };

    class D3D12BufferView final : public BufferView,
                                  public D3D12Descriptor
    {
    public:
        D3D12BufferView(
            const BufferPointer& _buffer,
            const BufferViewDesc& _desc,
            D3D12DescriptorIndex _descriptorIndex,
            D3D12DescriptorAllocatorPointer& _allocator
        );

        ~D3D12BufferView() override;

        virtual void* getNativeHandle() const override;
        virtual void setName(const char* _name) override;

        virtual D3D12DescriptorIndex getDescriptorIndex() const override;
        virtual D3D12_CPU_DESCRIPTOR_HANDLE getCpuDescriptorHandle() const override;
        virtual D3D12_GPU_DESCRIPTOR_HANDLE getGpuDescriptorHandle() const override;
        virtual uint32_t getBindlessIndex() const override;

    private:
        D3D12DescriptorAllocatorWeakPointer m_allocator;
        D3D12DescriptorIndex m_descriptorIndex = D3D12InvalidDescriptorIndex;
    };
}
