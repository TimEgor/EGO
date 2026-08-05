#include "D3D12CommandList.h"

#include <algorithm>
#include <array>
#include <utility>
#include <vector>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Memory/Utils.h"

#include "../D3D12DescriptorFactory.h"
#include "../D3D12GraphicDevice.h"
#include "D3D12Pipeline.h"
#include "D3D12Texture.h"

namespace
{
    uint32_t GetSubresourceIndex(ID3D12Resource* _texture, uint32_t _mipLevel, uint32_t _arrayLayer)
    {
        const D3D12_RESOURCE_DESC resourceDesc = _texture->GetDesc();
        return _mipLevel + (_arrayLayer * resourceDesc.MipLevels);
    }

    ego::UInt32Vector3 ResolveTextureExtent(ID3D12Resource* _texture, uint32_t _mipLevel, const ego::UInt32Vector3& _explicitExtent)
    {
        if (_explicitExtent.m_x && _explicitExtent.m_y && _explicitExtent.m_z)
        {
            return _explicitExtent;
        }

        const D3D12_RESOURCE_DESC resourceDesc = _texture->GetDesc();
        const uint32_t width = std::max<uint32_t>(1, static_cast<uint32_t>(resourceDesc.Width) >> _mipLevel);
        const uint32_t height = std::max<uint32_t>(1, resourceDesc.Height >> _mipLevel);
        const uint32_t depth =
            resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? std::max<uint32_t>(1, resourceDesc.DepthOrArraySize >> _mipLevel) : 1;
        return ego::UInt32Vector3(width, height, depth);
    }

    bool ExtractFootprint(
        ID3D12Device* _device,
        ID3D12Resource* _texture,
        uint32_t _subresource,
        uint64_t _bufferOffset,
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT& _outFootprint)
    {
        if (!_device || !_texture)
        {
            return false;
        }

        const D3D12_RESOURCE_DESC resourceDesc = _texture->GetDesc();
        UINT numRows = 0;
        UINT64 rowSizeInBytes = 0;
        UINT64 totalBytes = 0;
        _device->GetCopyableFootprints(&resourceDesc, _subresource, 1, _bufferOffset, &_outFootprint, &numRows, &rowSizeInBytes, &totalBytes);
        return true;
    }
} // namespace

ego::gpu::d3d12::D3D12CommandListBase::D3D12CommandListBase(
    D3D12GraphicDevice* _device,
    const D3D12DescriptorFactory* _descriptorFactory,
    CommandType _commandType,
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>&& _commandList)
    : m_device(_device),
      m_descriptorFactory(_descriptorFactory),
      m_commandType(_commandType),
      m_allocator(std::move(_allocator)),
      m_commandList(std::move(_commandList))
{
}

ID3D12GraphicsCommandList* ego::gpu::d3d12::D3D12CommandListBase::getD3D12CommandList() const
{
    return m_commandList.Get();
}

void ego::gpu::d3d12::D3D12CommandListBase::beginInternal()
{
    resetInternal();
}

void ego::gpu::d3d12::D3D12CommandListBase::resetInternal()
{
    m_currentBindingLayout = nullptr;
    m_currentRayTracingPipeline = nullptr;
    m_currentPipelineType = PipelineType::Graphic;

    if (m_allocator)
    {
        m_allocator->Reset();
    }

    if (m_commandList)
    {
        m_commandList->Reset(m_allocator.Get(), nullptr);
    }
}

void ego::gpu::d3d12::D3D12CommandListBase::endInternal()
{
    if (m_commandList)
    {
        m_commandList->Close();
    }
}

void ego::gpu::d3d12::D3D12CommandListBase::bindBindlessDescriptorHeapsInternal()
{
    if (!m_descriptorFactory || !m_commandList)
    {
        return;
    }

    ID3D12DescriptorHeap* heaps[2] = {};
    uint32_t heapCount = 0;

    ID3D12DescriptorHeap* viewDescriptorHeap = m_descriptorFactory->getViewDescriptorHeap();
    if (viewDescriptorHeap)
    {
        heaps[heapCount] = viewDescriptorHeap;
        ++heapCount;
    }

    ID3D12DescriptorHeap* samplerDescriptorHeap = m_descriptorFactory->getSamplerDescriptorHeap();
    if (samplerDescriptorHeap)
    {
        heaps[heapCount] = samplerDescriptorHeap;
        ++heapCount;
    }

    if (heapCount)
    {
        m_commandList->SetDescriptorHeaps(heapCount, heaps);
    }
}

void ego::gpu::d3d12::D3D12CommandListBase::resourceBarrierInternal(const GraphicResourcePointer& _resource, GraphicResourceState _nextState)
{
    ID3D12Resource* resource = _resource ? _resource->getNativeHandle<ID3D12Resource>() : nullptr;
    const GraphicResourceState prevState = _resource ? _resource->getState() : GraphicResourceState::Undefined;
    if (!resource || prevState == _nextState)
    {
        return;
    }

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = ToD3D12ResourceState(prevState);
    barrier.Transition.StateAfter = ToD3D12ResourceState(_nextState);
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_commandList->ResourceBarrier(1, &barrier);
    _resource->setState(_nextState);
}

void ego::gpu::d3d12::D3D12CommandListBase::pushConstantsInternal(ShaderStageFlags _stageFlags, uint32_t _offset, uint32_t _size, const void* _data)
{
    EGO_ASSERT_MESSAGE((_size % sizeof(uint32_t)) == 0, "D3D12 root constants require 32-bit aligned size");
    EGO_ASSERT_MESSAGE(m_currentBindingLayout, "D3D12 push constants require a bound pipeline");

    const D3D12BindingLayout::PushConstantInfo* pushConstantRange =
        m_currentBindingLayout ? m_currentBindingLayout->findPushConstantRange(_stageFlags, _offset, _size) : nullptr;

    EGO_ASSERT_MESSAGE(pushConstantRange, "Push constant range is not present in layout");

    if (!pushConstantRange)
    {
        return;
    }

    const uint32_t valuesOffset = (_offset - pushConstantRange->m_offset) / sizeof(uint32_t);
    const uint32_t valuesCount = _size / sizeof(uint32_t);

    if (m_currentPipelineType == PipelineType::Graphic)
    {
        m_commandList->SetGraphicsRoot32BitConstants(pushConstantRange->m_rootParameterIndex, valuesCount, _data, valuesOffset);
    }
    else
    {
        m_commandList->SetComputeRoot32BitConstants(pushConstantRange->m_rootParameterIndex, valuesCount, _data, valuesOffset);
    }
}

void ego::gpu::d3d12::D3D12CommandListBase::setRayTracingPipelineInternal(const RayTracingPipelinePointer& _pipeline)
{
    D3D12RayTracingPipeline* pipeline = _pipeline ? static_cast<D3D12RayTracingPipeline*>(_pipeline.getObject()) : nullptr;
    EGO_ASSERT_MESSAGE(pipeline, "Ray tracing pipeline must be created by D3D12 device");

    if (!pipeline || !m_commandList)
    {
        return;
    }

    bindBindlessDescriptorHeapsInternal();
    m_commandList->SetPipelineState1(pipeline->getStateObject());
    m_commandList->SetComputeRootSignature(pipeline->getBindingLayout() ? pipeline->getBindingLayout()->getRootSignature() : nullptr);
    m_currentBindingLayout = pipeline->getBindingLayout();
    m_currentRayTracingPipeline = pipeline;
    m_currentPipelineType = PipelineType::RayTracing;
}

void ego::gpu::d3d12::D3D12CommandListBase::dispatchRaysInternal(const DispatchRaysDesc& _desc)
{
    EGO_ASSERT_MESSAGE(m_currentRayTracingPipeline, "D3D12 DispatchRays requires a bound ray tracing pipeline");

    if (!m_currentRayTracingPipeline || !m_commandList)
    {
        return;
    }

    if (_desc.m_width == 0 || _desc.m_height == 0 || _desc.m_depth == 0)
    {
        return;
    }

    D3D12_DISPATCH_RAYS_DESC dispatchDesc = m_currentRayTracingPipeline->getDispatchRaysDesc(_desc.m_width, _desc.m_height, _desc.m_depth);
    if (dispatchDesc.RayGenerationShaderRecord.StartAddress == 0)
    {
        return;
    }

    m_commandList->DispatchRays(&dispatchDesc);
}

void ego::gpu::d3d12::D3D12CommandListBase::copyBufferInternal(
    const BufferPointer& _srcBuffer,
    const BufferPointer& _dstBuffer,
    const BufferCopyRegionDesc& _region)
{
    ID3D12Resource* srcBuffer = _srcBuffer ? _srcBuffer->getNativeHandle<ID3D12Resource>() : nullptr;
    ID3D12Resource* dstBuffer = _dstBuffer ? _dstBuffer->getNativeHandle<ID3D12Resource>() : nullptr;
    if (!srcBuffer || !dstBuffer)
    {
        return;
    }

    m_commandList->CopyBufferRegion(dstBuffer, _region.m_dstOffset, srcBuffer, _region.m_srcOffset, _region.m_size);
}

void ego::gpu::d3d12::D3D12CommandListBase::copyTextureInternal(
    const TexturePointer& _srcTexture,
    const TexturePointer& _dstTexture,
    const TextureCopyRegionDesc& _region)
{
    ID3D12Resource* srcTexture = _srcTexture ? _srcTexture->getNativeHandle<ID3D12Resource>() : nullptr;
    ID3D12Resource* dstTexture = _dstTexture ? _dstTexture->getNativeHandle<ID3D12Resource>() : nullptr;
    if (!srcTexture || !dstTexture)
    {
        return;
    }

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = srcTexture;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = GetSubresourceIndex(srcTexture, _region.m_srcMipLevel, _region.m_srcArrayLayer);

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = dstTexture;
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = GetSubresourceIndex(dstTexture, _region.m_dstMipLevel, _region.m_dstArrayLayer);

    D3D12_BOX srcBox = {};
    ego::UInt32Vector3 extent = ResolveTextureExtent(srcTexture, _region.m_srcMipLevel, _region.m_extent);
    srcBox.left = _region.m_srcOffset.m_x;
    srcBox.top = _region.m_srcOffset.m_y;
    srcBox.front = _region.m_srcOffset.m_z;
    srcBox.right = _region.m_srcOffset.m_x + extent.m_x;
    srcBox.bottom = _region.m_srcOffset.m_y + extent.m_y;
    srcBox.back = _region.m_srcOffset.m_z + extent.m_z;

    m_commandList->CopyTextureRegion(&dstLocation, _region.m_dstOffset.m_x, _region.m_dstOffset.m_y, _region.m_dstOffset.m_z, &srcLocation, &srcBox);
}

void ego::gpu::d3d12::D3D12CommandListBase::copyBufferToTextureInternal(
    const BufferPointer& _srcBuffer,
    const TexturePointer& _dstTexture,
    const BufferTextureCopyRegionDesc& _region)
{
    ID3D12Resource* srcBuffer = _srcBuffer ? _srcBuffer->getNativeHandle<ID3D12Resource>() : nullptr;
    ID3D12Resource* dstTexture = _dstTexture ? _dstTexture->getNativeHandle<ID3D12Resource>() : nullptr;
    if (!srcBuffer || !dstTexture)
    {
        return;
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
    const uint32_t subresourceIndex = GetSubresourceIndex(dstTexture, _region.m_mipLevel, _region.m_arrayLayer);
    if (!ExtractFootprint(m_device->getDevice(), dstTexture, subresourceIndex, _region.m_bufferOffset, footprint))
    {
        return;
    }

    if (_region.m_bufferRowPitch)
    {
        footprint.Footprint.RowPitch = static_cast<UINT>(ego::Align(_region.m_bufferRowPitch, static_cast<uint32_t>(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)));
    }

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = srcBuffer;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.PlacedFootprint = footprint;

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = dstTexture;
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = subresourceIndex;

    D3D12_BOX srcBox = {};
    ego::UInt32Vector3 extent = ResolveTextureExtent(dstTexture, _region.m_mipLevel, _region.m_textureExtent);
    srcBox.left = 0;
    srcBox.top = 0;
    srcBox.front = 0;
    srcBox.right = extent.m_x;
    srcBox.bottom = extent.m_y;
    srcBox.back = extent.m_z;

    m_commandList
        ->CopyTextureRegion(&dstLocation, _region.m_textureOffset.m_x, _region.m_textureOffset.m_y, _region.m_textureOffset.m_z, &srcLocation, &srcBox);
}

void ego::gpu::d3d12::D3D12CommandListBase::copyTextureToBufferInternal(
    const TexturePointer& _srcTexture,
    const BufferPointer& _dstBuffer,
    const BufferTextureCopyRegionDesc& _region)
{
    ID3D12Resource* srcTexture = _srcTexture ? _srcTexture->getNativeHandle<ID3D12Resource>() : nullptr;
    ID3D12Resource* dstBuffer = _dstBuffer ? _dstBuffer->getNativeHandle<ID3D12Resource>() : nullptr;
    if (!srcTexture || !dstBuffer)
    {
        return;
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
    const uint32_t subresourceIndex = GetSubresourceIndex(srcTexture, _region.m_mipLevel, _region.m_arrayLayer);
    if (!ExtractFootprint(m_device->getDevice(), srcTexture, subresourceIndex, _region.m_bufferOffset, footprint))
    {
        return;
    }

    if (_region.m_bufferRowPitch)
    {
        footprint.Footprint.RowPitch = static_cast<UINT>(ego::Align(_region.m_bufferRowPitch, static_cast<uint32_t>(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)));
    }

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = srcTexture;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = subresourceIndex;

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = dstBuffer;
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dstLocation.PlacedFootprint = footprint;

    D3D12_BOX srcBox = {};
    ego::UInt32Vector3 extent = ResolveTextureExtent(srcTexture, _region.m_mipLevel, _region.m_textureExtent);
    srcBox.left = _region.m_textureOffset.m_x;
    srcBox.top = _region.m_textureOffset.m_y;
    srcBox.front = _region.m_textureOffset.m_z;
    srcBox.right = _region.m_textureOffset.m_x + extent.m_x;
    srcBox.bottom = _region.m_textureOffset.m_y + extent.m_y;
    srcBox.back = _region.m_textureOffset.m_z + extent.m_z;

    m_commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &srcBox);
}

ego::gpu::d3d12::D3D12CopyCommandList::D3D12CopyCommandList(
    D3D12GraphicDevice* _device,
    const D3D12DescriptorFactory* _descriptorFactory,
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>&& _commandList)
    : D3D12CommandListBase(_device, _descriptorFactory, CommandType::Copy, std::move(_allocator), std::move(_commandList))
{
}

void* ego::gpu::d3d12::D3D12CopyCommandList::getNativeHandle() const
{
    return m_commandList.Get();
}

void ego::gpu::d3d12::D3D12CopyCommandList::setName(const char* _name)
{
    SetD3D12ObjectName(m_commandList.Get(), _name);
}

void ego::gpu::d3d12::D3D12CopyCommandList::begin()
{
    beginInternal();
    clearGpuWaits();
}

void ego::gpu::d3d12::D3D12CopyCommandList::reset()
{
    resetInternal();
    clearGpuWaits();
}

void ego::gpu::d3d12::D3D12CopyCommandList::end()
{
    endInternal();
}

void ego::gpu::d3d12::D3D12CopyCommandList::resourceBarrier(const GraphicResourcePointer& _resource, GraphicResourceState _nextState)
{
    if (_resource && _resource->getState() != _nextState)
    {
        addResourceGpuWait(_resource);
        resourceBarrierInternal(_resource, _nextState);
    }
}

void ego::gpu::d3d12::D3D12CopyCommandList::pushConstants(ShaderStageFlags, uint32_t, uint32_t, const void*)
{
    EGO_ASSERT_FAIL_MESSAGE("Copy command list does not support push constants");
}

void ego::gpu::d3d12::D3D12CopyCommandList::copyBuffer(const BufferPointer& _srcBuffer, const BufferPointer& _dstBuffer, const BufferCopyRegionDesc& _region)
{
    addResourceGpuWait(_srcBuffer);
    addResourceGpuWait(_dstBuffer);
    copyBufferInternal(_srcBuffer, _dstBuffer, _region);
}

void ego::gpu::d3d12::D3D12CopyCommandList::copyTexture(
    const TexturePointer& _srcTexture,
    const TexturePointer& _dstTexture,
    const TextureCopyRegionDesc& _region)
{
    addResourceGpuWait(_srcTexture);
    addResourceGpuWait(_dstTexture);
    copyTextureInternal(_srcTexture, _dstTexture, _region);
}

void ego::gpu::d3d12::D3D12CopyCommandList::copyBufferToTexture(
    const BufferPointer& _srcBuffer,
    const TexturePointer& _dstTexture,
    const BufferTextureCopyRegionDesc& _region)
{
    addResourceGpuWait(_srcBuffer);
    addResourceGpuWait(_dstTexture);
    copyBufferToTextureInternal(_srcBuffer, _dstTexture, _region);
}

void ego::gpu::d3d12::D3D12CopyCommandList::copyTextureToBuffer(
    const TexturePointer& _srcTexture,
    const BufferPointer& _dstBuffer,
    const BufferTextureCopyRegionDesc& _region)
{
    addResourceGpuWait(_srcTexture);
    addResourceGpuWait(_dstBuffer);
    copyTextureToBufferInternal(_srcTexture, _dstBuffer, _region);
}

ego::gpu::d3d12::D3D12ComputeCommandList::D3D12ComputeCommandList(
    D3D12GraphicDevice* _device,
    const D3D12DescriptorFactory* _descriptorFactory,
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>&& _commandList)
    : D3D12CommandListBase(_device, _descriptorFactory, CommandType::Compute, std::move(_allocator), std::move(_commandList))
{
}

void* ego::gpu::d3d12::D3D12ComputeCommandList::getNativeHandle() const
{
    return m_commandList.Get();
}

void ego::gpu::d3d12::D3D12ComputeCommandList::setName(const char* _name)
{
    SetD3D12ObjectName(m_commandList.Get(), _name);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::begin()
{
    beginInternal();
    clearGpuWaits();
}

void ego::gpu::d3d12::D3D12ComputeCommandList::reset()
{
    resetInternal();
    clearGpuWaits();
}

void ego::gpu::d3d12::D3D12ComputeCommandList::end()
{
    endInternal();
}

void ego::gpu::d3d12::D3D12ComputeCommandList::resourceBarrier(const GraphicResourcePointer& _resource, GraphicResourceState _nextState)
{
    if (_resource && _resource->getState() != _nextState)
    {
        addResourceGpuWait(_resource);
        resourceBarrierInternal(_resource, _nextState);
    }
}

void ego::gpu::d3d12::D3D12ComputeCommandList::pushConstants(ShaderStageFlags _stageFlags, uint32_t _offset, uint32_t _size, const void* _data)
{
    pushConstantsInternal(_stageFlags, _offset, _size, _data);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::copyBuffer(const BufferPointer& _srcBuffer, const BufferPointer& _dstBuffer, const BufferCopyRegionDesc& _region)
{
    addResourceGpuWait(_srcBuffer);
    addResourceGpuWait(_dstBuffer);
    copyBufferInternal(_srcBuffer, _dstBuffer, _region);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::copyTexture(
    const TexturePointer& _srcTexture,
    const TexturePointer& _dstTexture,
    const TextureCopyRegionDesc& _region)
{
    addResourceGpuWait(_srcTexture);
    addResourceGpuWait(_dstTexture);
    copyTextureInternal(_srcTexture, _dstTexture, _region);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::copyBufferToTexture(
    const BufferPointer& _srcBuffer,
    const TexturePointer& _dstTexture,
    const BufferTextureCopyRegionDesc& _region)
{
    addResourceGpuWait(_srcBuffer);
    addResourceGpuWait(_dstTexture);
    copyBufferToTextureInternal(_srcBuffer, _dstTexture, _region);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::copyTextureToBuffer(
    const TexturePointer& _srcTexture,
    const BufferPointer& _dstBuffer,
    const BufferTextureCopyRegionDesc& _region)
{
    addResourceGpuWait(_srcTexture);
    addResourceGpuWait(_dstBuffer);
    copyTextureToBufferInternal(_srcTexture, _dstBuffer, _region);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::setPipeline(const ComputePipelinePointer& _pipeline)
{
    D3D12ComputePipeline* pipeline = _pipeline ? static_cast<D3D12ComputePipeline*>(_pipeline.getObject()) : nullptr;
    EGO_ASSERT_MESSAGE(pipeline, "Compute pipeline must be created by D3D12 device");
    if (!pipeline)
    {
        return;
    }

    bindBindlessDescriptorHeapsInternal();
    getD3D12CommandList()->SetPipelineState(pipeline->getPipelineState());
    getD3D12CommandList()->SetComputeRootSignature(pipeline->getBindingLayout() ? pipeline->getBindingLayout()->getRootSignature() : nullptr);
    m_currentBindingLayout = pipeline->getBindingLayout();
    m_currentRayTracingPipeline = nullptr;
    m_currentPipelineType = PipelineType::Compute;
}

void ego::gpu::d3d12::D3D12ComputeCommandList::setPipeline(const RayTracingPipelinePointer& _pipeline)
{
    setRayTracingPipelineInternal(_pipeline);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::dispatch(uint32_t _threadGroupsX, uint32_t _threadGroupsY, uint32_t _threadGroupsZ)
{
    getD3D12CommandList()->Dispatch(_threadGroupsX, _threadGroupsY, _threadGroupsZ);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::dispatchRays(const DispatchRaysDesc& _desc)
{
    dispatchRaysInternal(_desc);
}

ego::gpu::d3d12::D3D12GraphicCommandList::D3D12GraphicCommandList(
    D3D12GraphicDevice* _device,
    const D3D12DescriptorFactory* _descriptorFactory,
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>&& _commandList)
    : D3D12CommandListBase(_device, _descriptorFactory, CommandType::Graphic, std::move(_allocator), std::move(_commandList))
{
}

void* ego::gpu::d3d12::D3D12GraphicCommandList::getNativeHandle() const
{
    return m_commandList.Get();
}

void ego::gpu::d3d12::D3D12GraphicCommandList::setName(const char* _name)
{
    SetD3D12ObjectName(m_commandList.Get(), _name);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::begin()
{
    beginInternal();
    clearGpuWaits();
}

void ego::gpu::d3d12::D3D12GraphicCommandList::reset()
{
    resetInternal();
    clearGpuWaits();
}

void ego::gpu::d3d12::D3D12GraphicCommandList::end()
{
    endInternal();
}

void ego::gpu::d3d12::D3D12GraphicCommandList::resourceBarrier(const GraphicResourcePointer& _resource, GraphicResourceState _nextState)
{
    if (_resource && _resource->getState() != _nextState)
    {
        addResourceGpuWait(_resource);
        resourceBarrierInternal(_resource, _nextState);
    }
}

void ego::gpu::d3d12::D3D12GraphicCommandList::pushConstants(ShaderStageFlags _stageFlags, uint32_t _offset, uint32_t _size, const void* _data)
{
    pushConstantsInternal(_stageFlags, _offset, _size, _data);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::copyBuffer(const BufferPointer& _srcBuffer, const BufferPointer& _dstBuffer, const BufferCopyRegionDesc& _region)
{
    addResourceGpuWait(_srcBuffer);
    addResourceGpuWait(_dstBuffer);
    copyBufferInternal(_srcBuffer, _dstBuffer, _region);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::copyTexture(
    const TexturePointer& _srcTexture,
    const TexturePointer& _dstTexture,
    const TextureCopyRegionDesc& _region)
{
    addResourceGpuWait(_srcTexture);
    addResourceGpuWait(_dstTexture);
    copyTextureInternal(_srcTexture, _dstTexture, _region);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::copyBufferToTexture(
    const BufferPointer& _srcBuffer,
    const TexturePointer& _dstTexture,
    const BufferTextureCopyRegionDesc& _region)
{
    addResourceGpuWait(_srcBuffer);
    addResourceGpuWait(_dstTexture);
    copyBufferToTextureInternal(_srcBuffer, _dstTexture, _region);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::copyTextureToBuffer(
    const TexturePointer& _srcTexture,
    const BufferPointer& _dstBuffer,
    const BufferTextureCopyRegionDesc& _region)
{
    addResourceGpuWait(_srcTexture);
    addResourceGpuWait(_dstBuffer);
    copyTextureToBufferInternal(_srcTexture, _dstBuffer, _region);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::setPipeline(const ComputePipelinePointer& _pipeline)
{
    D3D12ComputePipeline* pipeline = _pipeline ? static_cast<D3D12ComputePipeline*>(_pipeline.getObject()) : nullptr;
    EGO_ASSERT_MESSAGE(pipeline, "Compute pipeline must be created by D3D12 device");
    if (!pipeline)
    {
        return;
    }

    bindBindlessDescriptorHeapsInternal();
    getD3D12CommandList()->SetPipelineState(pipeline->getPipelineState());
    getD3D12CommandList()->SetComputeRootSignature(pipeline->getBindingLayout() ? pipeline->getBindingLayout()->getRootSignature() : nullptr);
    m_currentBindingLayout = pipeline->getBindingLayout();
    m_currentRayTracingPipeline = nullptr;
    m_currentPipelineType = PipelineType::Compute;
}

void ego::gpu::d3d12::D3D12GraphicCommandList::setPipeline(const RayTracingPipelinePointer& _pipeline)
{
    setRayTracingPipelineInternal(_pipeline);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::dispatch(uint32_t _threadGroupsX, uint32_t _threadGroupsY, uint32_t _threadGroupsZ)
{
    getD3D12CommandList()->Dispatch(_threadGroupsX, _threadGroupsY, _threadGroupsZ);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::dispatchRays(const DispatchRaysDesc& _desc)
{
    dispatchRaysInternal(_desc);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::beginRendering(const RenderingDesc& _desc)
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
    rtvHandles.reserve(_desc.m_colorAttachments.size());

    for (const ColorAttachmentDesc& colorAttachment : _desc.m_colorAttachments)
    {
        D3D12TextureView* textureView = colorAttachment.m_view ? static_cast<D3D12TextureView*>(colorAttachment.m_view.getObject()) : nullptr;
        EGO_ASSERT_MESSAGE(textureView, "Render target view must be created by D3D12 device");
        if (!textureView)
        {
            continue;
        }

        addResourceGpuWait(textureView->getResource());

        const D3D12_CPU_DESCRIPTOR_HANDLE handle = textureView->getCpuDescriptorHandle();
        rtvHandles.push_back(handle);

        if (colorAttachment.m_loadOperation == AttachmentLoadOperation::Clear)
        {
            const std::array<float, 4> clearValue = {colorAttachment.m_clearValue.getX(),
                colorAttachment.m_clearValue.getY(),
                colorAttachment.m_clearValue.getZ(),
                colorAttachment.m_clearValue.getW()};
            getD3D12CommandList()->ClearRenderTargetView(handle, clearValue.data(), 0, nullptr);
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};
    D3D12TextureView* depthView =
        _desc.m_depthStencilAttachment.m_view ? static_cast<D3D12TextureView*>(_desc.m_depthStencilAttachment.m_view.getObject()) : nullptr;
    if (depthView)
    {
        addResourceGpuWait(depthView->getResource());

        dsvHandle = depthView->getCpuDescriptorHandle();

        auto clearFlags = static_cast<D3D12_CLEAR_FLAGS>(0);
        if (_desc.m_depthStencilAttachment.m_depthLoadOperation == AttachmentLoadOperation::Clear)
        {
            clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
        }

        if (_desc.m_depthStencilAttachment.m_stencilLoadOperation == AttachmentLoadOperation::Clear)
        {
            clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
        }

        if (clearFlags != 0)
        {
            getD3D12CommandList()->ClearDepthStencilView(
                dsvHandle,
                clearFlags,
                _desc.m_depthStencilAttachment.m_clearDepth,
                static_cast<UINT8>(_desc.m_depthStencilAttachment.m_clearStencil),
                0,
                nullptr);
        }
    }

    getD3D12CommandList()
        ->OMSetRenderTargets(static_cast<UINT>(rtvHandles.size()), rtvHandles.empty() ? nullptr : rtvHandles.data(), FALSE, depthView ? &dsvHandle : nullptr);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::endRendering() {}

void ego::gpu::d3d12::D3D12GraphicCommandList::setPipeline(const GraphicPipelinePointer& _pipeline)
{
    D3D12GraphicPipeline* pipeline = _pipeline ? static_cast<D3D12GraphicPipeline*>(_pipeline.getObject()) : nullptr;
    EGO_ASSERT_MESSAGE(pipeline, "Graphic pipeline must be created by D3D12 device");
    if (!pipeline)
    {
        return;
    }

    bindBindlessDescriptorHeapsInternal();
    getD3D12CommandList()->SetPipelineState(pipeline->getPipelineState());
    getD3D12CommandList()->SetGraphicsRootSignature(pipeline->getBindingLayout() ? pipeline->getBindingLayout()->getRootSignature() : nullptr);
    getD3D12CommandList()->IASetPrimitiveTopology(pipeline->getPrimitiveTopology());
    m_currentBindingLayout = pipeline->getBindingLayout();
    m_currentRayTracingPipeline = nullptr;
    m_currentPipelineType = PipelineType::Graphic;
}

void ego::gpu::d3d12::D3D12GraphicCommandList::setViewport(const ViewportDesc& _viewportDesc)
{
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = _viewportDesc.m_x;
    viewport.TopLeftY = _viewportDesc.m_y;
    viewport.Width = _viewportDesc.m_width;
    viewport.Height = _viewportDesc.m_height;
    viewport.MinDepth = _viewportDesc.m_minDepth;
    viewport.MaxDepth = _viewportDesc.m_maxDepth;
    getD3D12CommandList()->RSSetViewports(1, &viewport);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::setScissorRect(const ScissorRectDesc& _scissorRect)
{
    D3D12_RECT rect = {};
    rect.left = _scissorRect.m_left;
    rect.top = _scissorRect.m_top;
    rect.right = _scissorRect.m_right;
    rect.bottom = _scissorRect.m_bottom;
    getD3D12CommandList()->RSSetScissorRects(1, &rect);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::setVertexBuffer(uint32_t _slot, const BufferPointer& _buffer, uint32_t _stride, uint32_t _offset)
{
    ID3D12Resource* buffer = _buffer ? _buffer->getNativeHandle<ID3D12Resource>() : nullptr;
    if (!buffer)
    {
        return;
    }

    addResourceGpuWait(_buffer);

    const D3D12_RESOURCE_DESC bufferDesc = buffer->GetDesc();
    D3D12_VERTEX_BUFFER_VIEW view = {};
    view.BufferLocation = buffer->GetGPUVirtualAddress() + _offset;
    view.StrideInBytes = _stride;
    view.SizeInBytes = static_cast<UINT>(bufferDesc.Width - _offset);
    getD3D12CommandList()->IASetVertexBuffers(_slot, 1, &view);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::setIndexBuffer(const BufferPointer& _buffer, GraphicResourceFormat _format, uint32_t _offset)
{
    ID3D12Resource* buffer = _buffer ? _buffer->getNativeHandle<ID3D12Resource>() : nullptr;
    if (!buffer)
    {
        return;
    }

    addResourceGpuWait(_buffer);

    const D3D12_RESOURCE_DESC bufferDesc = buffer->GetDesc();
    D3D12_INDEX_BUFFER_VIEW view = {};
    view.BufferLocation = buffer->GetGPUVirtualAddress() + _offset;
    view.Format = ToDXGIFormat(_format);
    view.SizeInBytes = static_cast<UINT>(bufferDesc.Width - _offset);
    getD3D12CommandList()->IASetIndexBuffer(&view);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::draw(uint32_t _vertexCount, uint32_t _instanceCount, uint32_t _firstVertex, uint32_t _firstInstance)
{
    getD3D12CommandList()->DrawInstanced(_vertexCount, _instanceCount, _firstVertex, _firstInstance);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::drawIndexed(
    uint32_t _indexCount,
    uint32_t _instanceCount,
    uint32_t _firstIndex,
    int32_t _vertexOffset,
    uint32_t _firstInstance)
{
    getD3D12CommandList()->DrawIndexedInstanced(_indexCount, _instanceCount, _firstIndex, _vertexOffset, _firstInstance);
}
