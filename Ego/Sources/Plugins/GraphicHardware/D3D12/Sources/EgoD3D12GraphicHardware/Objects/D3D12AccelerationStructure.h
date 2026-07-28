#pragma once

#include "EgoGraphicHardware/GraphicObjects/AccelerationStructure.h"

#include "D3D12Buffer.h"
#include "D3D12Resource.h"

namespace ego::gpu::d3d12
{
    class D3D12GeometryAccelerationStructure final : public GeometryAccelerationStructure, public D3D12Resource
    {
    public:
        explicit D3D12GeometryAccelerationStructure(const BufferPointer& _buffer);

        void* getNativeHandle() const override;
        void setName(const char* _name) override;
        const BufferPointer& getBuffer() const override;

        ID3D12Resource* getD3D12Resource() const override;
        D3D12_GPU_VIRTUAL_ADDRESS getGpuVirtualAddress() const;

    private:
        BufferPointer m_buffer = nullptr;
    };

    class D3D12InstanceAccelerationStructure final : public InstanceAccelerationStructure, public D3D12Resource
    {
    public:
        explicit D3D12InstanceAccelerationStructure(const BufferPointer& _buffer);

        void* getNativeHandle() const override;
        void setName(const char* _name) override;
        const BufferPointer& getBuffer() const override;

        ID3D12Resource* getD3D12Resource() const override;
        D3D12_GPU_VIRTUAL_ADDRESS getGpuVirtualAddress() const;

    private:
        BufferPointer m_buffer = nullptr;
    };

    class D3D12AccelerationStructureView final : public AccelerationStructureView, public D3D12Descriptor
    {
    public:
        D3D12AccelerationStructureView(
            const InstanceAccelerationStructurePointer& _accelerationStructure,
            D3D12DescriptorIndex _descriptorIndex,
            D3D12DescriptorAllocatorPointer& _allocator);

        ~D3D12AccelerationStructureView() override;

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        D3D12DescriptorIndex getDescriptorIndex() const override;
        D3D12_CPU_DESCRIPTOR_HANDLE getCpuDescriptorHandle() const override;
        D3D12_GPU_DESCRIPTOR_HANDLE getGpuDescriptorHandle() const override;
        uint32_t getBindlessIndex() const override;

    private:
        D3D12DescriptorAllocatorWeakPointer m_allocator;
        D3D12DescriptorIndex m_descriptorIndex = D3D12InvalidDescriptorIndex;
    };
} // namespace ego::gpu::d3d12
