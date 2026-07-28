#pragma once

#include <cstdint>

#include "EgoGraphicHardware/GraphicObjects/AccelerationStructure.h"
#include "EgoGraphicHardware/GraphicObjects/Buffer.h"
#include "EgoGraphicHardware/GraphicObjects/Sampler.h"
#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "Common/D3D12DescriptorAllocator.h"

namespace ego::gpu::d3d12
{
    class D3D12Buffer;

    class D3D12DescriptorFactory final
    {
    public:
        static constexpr uint32_t BindlessResourceDescriptorCapacity = 1024;
        static constexpr uint32_t BindlessSamplerDescriptorCapacity = 128;

        D3D12DescriptorFactory() = default;

        bool init(ID3D12Device5* _device);
        void release();

        SamplerPointer createSampler(const SamplerDesc& _desc) const;
        BufferViewPointer createBufferView(const BufferPointer& _buffer, const BufferViewDesc& _desc) const;
        TextureViewPointer createTextureView(const TexturePointer& _texture, const TextureViewDesc& _desc) const;
        AccelerationStructureViewPointer createAccelerationStructureView(const InstanceAccelerationStructurePointer& _accelerationStructure) const;

        ID3D12DescriptorHeap* getViewDescriptorHeap() const;
        ID3D12DescriptorHeap* getSamplerDescriptorHeap() const;

    private:
        static bool FillBufferViewDesc(const D3D12Buffer* _buffer, const BufferViewDesc& _desc, D3D12_BUFFER_SRV& _outBufferDesc, DXGI_FORMAT& _outFormat);

        ID3D12Device5* m_device = nullptr;
        D3D12DescriptorAllocatorPointer m_viewDescriptorAllocator;
        D3D12DescriptorAllocatorPointer m_samplerDescriptorAllocator;
        D3D12DescriptorAllocatorPointer m_rtvDescriptorAllocator;
        D3D12DescriptorAllocatorPointer m_dsvDescriptorAllocator;
    };
} // namespace ego::gpu::d3d12
