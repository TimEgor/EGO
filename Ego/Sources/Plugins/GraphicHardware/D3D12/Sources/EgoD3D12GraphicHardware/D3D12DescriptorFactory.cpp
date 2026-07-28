#include "D3D12DescriptorFactory.h"

#include <algorithm>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Memory/Utils.h"

#include "Objects/D3D12AccelerationStructure.h"
#include "Objects/D3D12Buffer.h"
#include "Objects/D3D12Sampler.h"
#include "Objects/D3D12Texture.h"

namespace
{
    constexpr ego::gpu::d3d12::D3D12DescriptorIndex RenderTargetDescriptorCapacity = 256;
    constexpr ego::gpu::d3d12::D3D12DescriptorIndex DepthStencilDescriptorCapacity = 256;
} // namespace

bool ego::gpu::d3d12::D3D12DescriptorFactory::init(ID3D12Device5* _device)
{
    release();
    m_device = _device;

    EGO_CHECK_INITIALIZATION(m_device);

    m_viewDescriptorAllocator = MakePointer<D3D12DescriptorAllocator>();
    EGO_CHECK_INITIALIZATION(
        m_viewDescriptorAllocator && m_viewDescriptorAllocator->init(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, BindlessResourceDescriptorCapacity, true));

    m_samplerDescriptorAllocator = MakePointer<D3D12DescriptorAllocator>();
    EGO_CHECK_INITIALIZATION(
        m_samplerDescriptorAllocator && m_samplerDescriptorAllocator->init(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, BindlessSamplerDescriptorCapacity, true));

    m_rtvDescriptorAllocator = MakePointer<D3D12DescriptorAllocator>();
    EGO_CHECK_INITIALIZATION(m_rtvDescriptorAllocator && m_rtvDescriptorAllocator->init(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RenderTargetDescriptorCapacity, false));

    m_dsvDescriptorAllocator = MakePointer<D3D12DescriptorAllocator>();
    EGO_CHECK_INITIALIZATION(m_dsvDescriptorAllocator && m_dsvDescriptorAllocator->init(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, DepthStencilDescriptorCapacity, false));

    return true;
}

void ego::gpu::d3d12::D3D12DescriptorFactory::release()
{
    m_device = nullptr;
    m_viewDescriptorAllocator.reset();
    m_samplerDescriptorAllocator.reset();
    m_rtvDescriptorAllocator.reset();
    m_dsvDescriptorAllocator.reset();
}

ego::gpu::SamplerPointer ego::gpu::d3d12::D3D12DescriptorFactory::createSampler(const SamplerDesc& _desc) const
{
    D3D12DescriptorAllocatorPointer allocator = m_samplerDescriptorAllocator;
    if (!m_device || !allocator)
    {
        return SamplerPointer();
    }

    const D3D12DescriptorIndex descriptorIndex = allocator->allocate();
    if (descriptorIndex == D3D12InvalidDescriptorIndex)
    {
        return SamplerPointer();
    }

    D3D12_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = ToD3D12Filter(_desc);
    samplerDesc.AddressU = ToD3D12AddressMode(_desc.m_addressU);
    samplerDesc.AddressV = ToD3D12AddressMode(_desc.m_addressV);
    samplerDesc.AddressW = ToD3D12AddressMode(_desc.m_addressW);
    samplerDesc.MipLODBias = _desc.m_mipLodBias;
    samplerDesc.MaxAnisotropy = (std::max<uint32_t>)(_desc.m_maxAnisotropy, 1);
    samplerDesc.ComparisonFunc = _desc.m_enableComparison ? ToD3D12ComparisonFunc(_desc.m_comparisonOperation) : D3D12_COMPARISON_FUNC_ALWAYS;
    samplerDesc.MinLOD = _desc.m_minLod;
    samplerDesc.MaxLOD = _desc.m_maxLod;

    const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = allocator->getCpuHandle(descriptorIndex);
    m_device->CreateSampler(&samplerDesc, cpuHandle);

    return MakeIntrusive<D3D12Sampler>(_desc, descriptorIndex, allocator);
}

ego::gpu::BufferViewPointer ego::gpu::d3d12::D3D12DescriptorFactory::createBufferView(const BufferPointer& _buffer, const BufferViewDesc& _desc) const
{
    auto buffer = static_cast<D3D12Buffer*>(_buffer.getObject());
    EGO_ASSERT_MESSAGE(buffer, "Buffer must be created by D3D12 device");
    if (!m_device || !buffer || !m_viewDescriptorAllocator)
    {
        return BufferViewPointer();
    }

    if (_desc.m_type == GraphicResourceViewType::RenderTarget || _desc.m_type == GraphicResourceViewType::DepthStencil)
    {
        EGO_ASSERT_FAIL_MESSAGE("Buffers cannot be used for render-target or depth-stencil views");
        return BufferViewPointer();
    }

    D3D12DescriptorAllocatorPointer allocator = m_viewDescriptorAllocator;
    const D3D12DescriptorIndex descriptorIndex = allocator->allocate();
    if (descriptorIndex == D3D12InvalidDescriptorIndex)
    {
        return BufferViewPointer();
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = allocator->getCpuHandle(descriptorIndex);

    if (_desc.m_type == GraphicResourceViewType::ConstantBuffer)
    {
        const uint64_t size = ResolveViewSize(buffer->getDesc().m_size, _desc.m_offset, _desc.m_size);
        if (!size)
        {
            allocator->release(descriptorIndex);
            return BufferViewPointer();
        }

        D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
        viewDesc.BufferLocation = buffer->getD3D12Resource()->GetGPUVirtualAddress() + _desc.m_offset;
        viewDesc.SizeInBytes = static_cast<UINT>(ego::Align(size, static_cast<uint64_t>(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)));
        m_device->CreateConstantBufferView(&viewDesc, cpuHandle);
    }
    else if (_desc.m_type == GraphicResourceViewType::ShaderResource)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
        viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (!FillBufferViewDesc(buffer, _desc, viewDesc.Buffer, viewDesc.Format))
        {
            allocator->release(descriptorIndex);
            return BufferViewPointer();
        }

        m_device->CreateShaderResourceView(buffer->getD3D12Resource(), &viewDesc, cpuHandle);
    }
    else if (_desc.m_type == GraphicResourceViewType::UnorderedAccess)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
        viewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

        D3D12_BUFFER_SRV srvBufferDesc = {};
        if (!FillBufferViewDesc(buffer, _desc, srvBufferDesc, viewDesc.Format))
        {
            allocator->release(descriptorIndex);
            return BufferViewPointer();
        }

        viewDesc.Buffer.FirstElement = srvBufferDesc.FirstElement;
        viewDesc.Buffer.NumElements = srvBufferDesc.NumElements;
        viewDesc.Buffer.StructureByteStride = srvBufferDesc.StructureByteStride;
        viewDesc.Buffer.CounterOffsetInBytes = 0;
        viewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

        m_device->CreateUnorderedAccessView(buffer->getD3D12Resource(), nullptr, &viewDesc, cpuHandle);
    }

    return MakeIntrusive<D3D12BufferView>(_buffer, _desc, descriptorIndex, allocator);
}

ego::gpu::TextureViewPointer ego::gpu::d3d12::D3D12DescriptorFactory::createTextureView(const TexturePointer& _texture, const TextureViewDesc& _desc) const
{
    D3D12Resource* texture = static_cast<D3D12Texture2D*>(_texture.getObject());
    EGO_ASSERT_MESSAGE(texture, "Texture must be created by D3D12 device");
    if (!m_device || !texture)
    {
        return TextureViewPointer();
    }

    D3D12DescriptorAllocatorPointer allocator = nullptr;

    switch (_desc.m_type)
    {
    case GraphicResourceViewType::RenderTarget:
        allocator = m_rtvDescriptorAllocator;
        break;
    case GraphicResourceViewType::DepthStencil:
        allocator = m_dsvDescriptorAllocator;
        break;
    case GraphicResourceViewType::ShaderResource:
    case GraphicResourceViewType::UnorderedAccess:
        allocator = m_viewDescriptorAllocator;
        break;
    default:
        break;
    }

    if (!allocator)
    {
        return TextureViewPointer();
    }

    const D3D12DescriptorIndex descriptorIndex = allocator->allocate();
    if (descriptorIndex == D3D12InvalidDescriptorIndex)
    {
        return TextureViewPointer();
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = allocator->getCpuHandle(descriptorIndex);
    const D3D12_RESOURCE_DESC& resourceDesc = texture->getD3D12Resource()->GetDesc();
    const DXGI_FORMAT format = _desc.m_format == GraphicResourceFormat::Undefined ? resourceDesc.Format : ToDXGIFormat(_desc.m_format);
    const UINT mipLevels = resourceDesc.MipLevels ? resourceDesc.MipLevels : 1;
    const UINT arrayLayers = resourceDesc.DepthOrArraySize ? resourceDesc.DepthOrArraySize : 1;

    if (_desc.m_type == GraphicResourceViewType::ShaderResource)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
        viewDesc.Format = format;
        viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (arrayLayers > 1)
        {
            viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            viewDesc.Texture2DArray.MostDetailedMip = 0;
            viewDesc.Texture2DArray.MipLevels = mipLevels;
            viewDesc.Texture2DArray.FirstArraySlice = 0;
            viewDesc.Texture2DArray.ArraySize = arrayLayers;
        }
        else
        {
            viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MostDetailedMip = 0;
            viewDesc.Texture2D.MipLevels = mipLevels;
        }

        m_device->CreateShaderResourceView(texture->getD3D12Resource(), &viewDesc, cpuHandle);
    }
    else if (_desc.m_type == GraphicResourceViewType::UnorderedAccess)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
        viewDesc.Format = format;

        if (arrayLayers > 1)
        {
            viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            viewDesc.Texture2DArray.MipSlice = 0;
            viewDesc.Texture2DArray.FirstArraySlice = 0;
            viewDesc.Texture2DArray.ArraySize = arrayLayers;
        }
        else
        {
            viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MipSlice = 0;
        }

        m_device->CreateUnorderedAccessView(texture->getD3D12Resource(), nullptr, &viewDesc, cpuHandle);
    }
    else if (_desc.m_type == GraphicResourceViewType::RenderTarget)
    {
        D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
        viewDesc.Format = format;

        if (arrayLayers > 1)
        {
            viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            viewDesc.Texture2DArray.MipSlice = 0;
            viewDesc.Texture2DArray.FirstArraySlice = 0;
            viewDesc.Texture2DArray.ArraySize = arrayLayers;
        }
        else
        {
            viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MipSlice = 0;
        }

        m_device->CreateRenderTargetView(texture->getD3D12Resource(), &viewDesc, cpuHandle);
    }
    else if (_desc.m_type == GraphicResourceViewType::DepthStencil)
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
        viewDesc.Format = format;

        if (arrayLayers > 1)
        {
            viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            viewDesc.Texture2DArray.MipSlice = 0;
            viewDesc.Texture2DArray.FirstArraySlice = 0;
            viewDesc.Texture2DArray.ArraySize = arrayLayers;
        }
        else
        {
            viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MipSlice = 0;
        }

        m_device->CreateDepthStencilView(texture->getD3D12Resource(), &viewDesc, cpuHandle);
    }

    return MakeIntrusive<D3D12TextureView>(_texture, _desc, descriptorIndex, allocator);
}

ego::gpu::AccelerationStructureViewPointer ego::gpu::d3d12::D3D12DescriptorFactory::createAccelerationStructureView(
    const InstanceAccelerationStructurePointer& _accelerationStructure) const
{
    D3D12InstanceAccelerationStructure* accelerationStructure =
        _accelerationStructure ? static_cast<D3D12InstanceAccelerationStructure*>(_accelerationStructure.getObject()) : nullptr;
    if (!m_device || !accelerationStructure || !m_viewDescriptorAllocator)
    {
        return AccelerationStructureViewPointer();
    }

    D3D12DescriptorAllocatorPointer allocator = m_viewDescriptorAllocator;
    const D3D12DescriptorIndex descriptorIndex = allocator->allocate();
    if (descriptorIndex == D3D12InvalidDescriptorIndex)
    {
        return AccelerationStructureViewPointer();
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
    viewDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    viewDesc.RaytracingAccelerationStructure.Location = accelerationStructure->getGpuVirtualAddress();

    const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = allocator->getCpuHandle(descriptorIndex);
    m_device->CreateShaderResourceView(nullptr, &viewDesc, cpuHandle);

    return MakeIntrusive<D3D12AccelerationStructureView>(
        _accelerationStructure,
        descriptorIndex,
        allocator);
}

ID3D12DescriptorHeap* ego::gpu::d3d12::D3D12DescriptorFactory::getViewDescriptorHeap() const
{
    return m_viewDescriptorAllocator ? m_viewDescriptorAllocator->getHeap() : nullptr;
}

ID3D12DescriptorHeap* ego::gpu::d3d12::D3D12DescriptorFactory::getSamplerDescriptorHeap() const
{
    return m_samplerDescriptorAllocator ? m_samplerDescriptorAllocator->getHeap() : nullptr;
}

bool ego::gpu::d3d12::D3D12DescriptorFactory::FillBufferViewDesc(const D3D12Buffer* _buffer, const BufferViewDesc& _desc, D3D12_BUFFER_SRV& _outBufferDesc, DXGI_FORMAT& _outFormat)
{
    const uint64_t size = ResolveViewSize(_buffer->getDesc().m_size, _desc.m_offset, _desc.m_size);
    const uint32_t stride = _desc.m_format == GraphicResourceFormat::Undefined ? (_desc.m_stride ? _desc.m_stride : _buffer->getDesc().m_stride) : GetFormatStride(_desc.m_format);
    if (!stride || !size)
    {
        return false;
    }

    _outFormat = _desc.m_format == GraphicResourceFormat::Undefined ? DXGI_FORMAT_UNKNOWN : ToDXGIFormat(_desc.m_format);
    _outBufferDesc.FirstElement = static_cast<UINT64>(_desc.m_offset / stride);
    _outBufferDesc.NumElements = static_cast<UINT>(size / stride);
    _outBufferDesc.StructureByteStride = _outFormat == DXGI_FORMAT_UNKNOWN ? stride : 0;
    _outBufferDesc.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    return true;
}
