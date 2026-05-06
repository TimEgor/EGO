#pragma once

#include "EgoEngine/RenderHardware/GraphicObjects/Texture.h"

#include "D3D12Resource.h"

namespace ego::gpu::d3d12
{
    class D3D12Texture2D final : public Texture2D,
                                 public D3D12Resource
    {
    public:
        D3D12Texture2D(
            const Texture2DDesc& _desc,
            Microsoft::WRL::ComPtr<ID3D12Resource>&& _resource
        );

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        ID3D12Resource* getD3D12Resource() const override;

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
    };

    class D3D12TextureView final : public TextureView,
                                   public D3D12Descriptor
    {
    public:
        D3D12TextureView(
            const TexturePointer& _texture,
            const TextureViewDesc& _desc,
            D3D12DescriptorIndex _descriptorIndex,
            D3D12DescriptorAllocatorPointer& _allocator
        );
        ~D3D12TextureView() override;

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        D3D12DescriptorIndex getDescriptorIndex() const override;

    private:
        D3D12DescriptorAllocatorWeakPointer m_allocator;
        D3D12DescriptorIndex m_descriptorIndex = D3D12InvalidDescriptorIndex;
    };
}
