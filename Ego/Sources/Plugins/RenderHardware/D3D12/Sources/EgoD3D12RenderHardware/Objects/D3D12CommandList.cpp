#include "D3D12CommandList.h"

#include <algorithm>
#include <utility>

#include "D3D12Buffer.h"
#include "D3D12Pipeline.h"
#include "D3D12Texture.h"

#include "../D3D12GraphicDevice.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Memory/Utils.h"

namespace
{
    ego::gpu::d3d12::D3D12Buffer* GetD3D12Buffer(const ego::gpu::BufferReference& _buffer)
    {
        return _buffer ? static_cast<ego::gpu::d3d12::D3D12Buffer*>(_buffer.getObject()) : nullptr;
    }

    ego::gpu::d3d12::D3D12Texture2D* GetD3D12Texture2D(const ego::gpu::TextureReference& _texture)
    {
        return _texture ? static_cast<ego::gpu::d3d12::D3D12Texture2D*>(_texture.getObject()) : nullptr;
    }

    ego::gpu::d3d12::D3D12Resource* GetD3D12Resource(const ego::gpu::GraphicResourceReference& _resource)
    {
        if (!_resource)
        {
            return nullptr;
        }

        const ego::gpu::GraphicResourceType resourceType = _resource->getType();
        if (resourceType == ego::gpu::Buffer::GetGraphicResourceType())
        {
            return static_cast<ego::gpu::d3d12::D3D12Buffer*>(_resource.getObject());
        }

        if (resourceType == ego::gpu::Texture2D::GetGraphicResourceType())
        {
            return static_cast<ego::gpu::d3d12::D3D12Texture2D*>(_resource.getObject());
        }

        EGO_ASSERT_FAIL_MESSAGE("D3D12 resource operation supports only buffers and Texture2D resources");
        return nullptr;
    }

    uint32_t GetSubresourceIndex(
        const ego::gpu::d3d12::D3D12Resource* _texture,
        uint32_t _mipLevel,
        uint32_t _arrayLayer
    )
    {
        const D3D12_RESOURCE_DESC resourceDesc = _texture->getD3D12ResourceDesc();
        return _mipLevel + (_arrayLayer * resourceDesc.MipLevels);
    }

    ego::UInt32Vector3 ResolveTextureExtent(
        const ego::gpu::d3d12::D3D12Resource* _texture,
        uint32_t _mipLevel,
        const ego::UInt32Vector3& _explicitExtent
    )
    {
        if (_explicitExtent.m_x && _explicitExtent.m_y && _explicitExtent.m_z)
        {
            return _explicitExtent;
        }

        const D3D12_RESOURCE_DESC resourceDesc = _texture->getD3D12ResourceDesc();
        const uint32_t width = std::max<uint32_t>(1, static_cast<uint32_t>(resourceDesc.Width) >> _mipLevel);
        const uint32_t height = std::max<uint32_t>(1, resourceDesc.Height >> _mipLevel);
        const uint32_t depth = resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ?
                                   std::max<uint32_t>(1, resourceDesc.DepthOrArraySize >> _mipLevel) :
                                   1;
        return ego::UInt32Vector3(width, height, depth);
    }

    bool ExtractFootprint(
        ID3D12Device* _device,
        const ego::gpu::d3d12::D3D12Resource* _texture,
        uint32_t _subresource,
        uint64_t _bufferOffset,
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT& _outFootprint
    )
    {
        if (!_device || !_texture)
        {
            return false;
        }

        const D3D12_RESOURCE_DESC resourceDesc = _texture->getD3D12ResourceDesc();
        UINT numRows = 0;
        UINT64 rowSizeInBytes = 0;
        UINT64 totalBytes = 0;
        _device->GetCopyableFootprints(
            &resourceDesc,
            _subresource,
            1,
            _bufferOffset,
            &_outFootprint,
            &numRows,
            &rowSizeInBytes,
            &totalBytes
        );
        return true;
    }
}

ego::gpu::d3d12::D3D12CommandListBase::D3D12CommandListBase(
    D3D12GraphicDevice* _device,
    CommandType _commandType,
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>&& _commandList
)
    : m_device(_device),
      m_commandType(_commandType),
      m_allocator(std::move(_allocator)),
      m_commandList(std::move(_commandList))
{}

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
    m_currentPipelineType = PipelineType::Graphic;
    m_boundResourceViews.clear();
    m_boundSamplers.clear();

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
    if (!m_device || !m_commandList)
    {
        return;
    }

    ID3D12DescriptorHeap* heaps[2] = {};
    uint32_t heapCount = 0;

    D3D12DescriptorAllocatorPointer viewAllocator = m_device->getViewDescriptorAllocator();
    if (viewAllocator && viewAllocator->getHeap())
    {
        heaps[heapCount] = viewAllocator->getHeap();
        ++heapCount;
    }

    D3D12DescriptorAllocatorPointer samplerAllocator = m_device->getSamplerDescriptorAllocator();
    if (samplerAllocator && samplerAllocator->getHeap())
    {
        heaps[heapCount] = samplerAllocator->getHeap();
        ++heapCount;
    }

    if (heapCount)
    {
        m_commandList->SetDescriptorHeaps(heapCount, heaps);
    }
}

void ego::gpu::d3d12::D3D12CommandListBase::resourceBarrierInternal(
    const GraphicResourceReference& _resource,
    GraphicResourceState _prevState,
    GraphicResourceState _nextState
)
{
    D3D12Resource* resource = GetD3D12Resource(_resource);
    if (!resource || _prevState == _nextState)
    {
        return;
    }

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource->getD3D12Resource();
    barrier.Transition.StateBefore = ToD3D12ResourceState(_prevState);
    barrier.Transition.StateAfter = ToD3D12ResourceState(_nextState);
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_commandList->ResourceBarrier(1, &barrier);
}

void ego::gpu::d3d12::D3D12CommandListBase::bindResourceViewInternal(
    uint32_t _slot,
    const ResourceViewReference& _resourceView
)
{
    EGO_ASSERT_MESSAGE(
        !_resourceView || _resourceView->getBindlessIndex() != InvalidBindlessIndex,
        "ResourceView must expose a valid bindless descriptor index"
    );

    if (_resourceView && _resourceView->getBindlessIndex() == InvalidBindlessIndex)
    {
        return;
    }

    if (_slot >= m_boundResourceViews.size())
    {
        m_boundResourceViews.resize(static_cast<size_t>(_slot) + 1);
    }

    m_boundResourceViews[_slot] = _resourceView;
    bindBindlessDescriptorHeapsInternal();
}

void ego::gpu::d3d12::D3D12CommandListBase::bindSamplerInternal(
    uint32_t _slot,
    const SamplerReference& _sampler
)
{
    EGO_ASSERT_MESSAGE(
        !_sampler || _sampler->getBindlessIndex() != InvalidBindlessIndex,
        "Sampler must expose a valid bindless descriptor index"
    );

    if (_sampler && _sampler->getBindlessIndex() == InvalidBindlessIndex)
    {
        return;
    }

    if (_slot >= m_boundSamplers.size())
    {
        m_boundSamplers.resize(static_cast<size_t>(_slot) + 1);
    }

    m_boundSamplers[_slot] = _sampler;
    bindBindlessDescriptorHeapsInternal();
}

uint32_t ego::gpu::d3d12::D3D12CommandListBase::getResourceViewBindlessIndexInternal(uint32_t _slot) const
{
    if (_slot >= m_boundResourceViews.size() || !m_boundResourceViews[_slot])
    {
        return InvalidBindlessIndex;
    }

    return m_boundResourceViews[_slot]->getBindlessIndex();
}

uint32_t ego::gpu::d3d12::D3D12CommandListBase::getSamplerBindlessIndexInternal(uint32_t _slot) const
{
    if (_slot >= m_boundSamplers.size() || !m_boundSamplers[_slot])
    {
        return InvalidBindlessIndex;
    }

    return m_boundSamplers[_slot]->getBindlessIndex();
}

void ego::gpu::d3d12::D3D12CommandListBase::pushConstantsInternal(
    ShaderStageFlags _stageFlags,
    uint32_t _offset,
    uint32_t _size,
    const void* _data
)
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
        m_commandList->SetGraphicsRoot32BitConstants(
            pushConstantRange->m_rootParameterIndex,
            valuesCount,
            _data,
            valuesOffset
        );
    }
    else
    {
        m_commandList->SetComputeRoot32BitConstants(
            pushConstantRange->m_rootParameterIndex,
            valuesCount,
            _data,
            valuesOffset
        );
    }
}

void ego::gpu::d3d12::D3D12CommandListBase::copyBufferInternal(
    const BufferReference& _srcBuffer,
    const BufferReference& _dstBuffer,
    const BufferCopyRegionDesc& _region
)
{
    D3D12Buffer* srcBuffer = GetD3D12Buffer(_srcBuffer);
    D3D12Buffer* dstBuffer = GetD3D12Buffer(_dstBuffer);
    if (!srcBuffer || !dstBuffer)
    {
        return;
    }

    m_commandList->CopyBufferRegion(
        dstBuffer->getD3D12Resource(),
        _region.m_dstOffset,
        srcBuffer->getD3D12Resource(),
        _region.m_srcOffset,
        _region.m_size
    );
}

void ego::gpu::d3d12::D3D12CommandListBase::copyTextureInternal(
    const TextureReference& _srcTexture,
    const TextureReference& _dstTexture,
    const TextureCopyRegionDesc& _region
)
{
    D3D12Texture2D* srcTexture = GetD3D12Texture2D(_srcTexture);
    D3D12Texture2D* dstTexture = GetD3D12Texture2D(_dstTexture);
    if (!srcTexture || !dstTexture)
    {
        return;
    }

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = srcTexture->getD3D12Resource();
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = GetSubresourceIndex(srcTexture, _region.m_srcMipLevel, _region.m_srcArrayLayer);

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = dstTexture->getD3D12Resource();
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

    m_commandList->CopyTextureRegion(
        &dstLocation,
        _region.m_dstOffset.m_x,
        _region.m_dstOffset.m_y,
        _region.m_dstOffset.m_z,
        &srcLocation,
        &srcBox
    );
}

void ego::gpu::d3d12::D3D12CommandListBase::copyBufferToTextureInternal(
    const BufferReference& _srcBuffer,
    const TextureReference& _dstTexture,
    const BufferTextureCopyRegionDesc& _region
)
{
    D3D12Buffer* srcBuffer = GetD3D12Buffer(_srcBuffer);
    D3D12Texture2D* dstTexture = GetD3D12Texture2D(_dstTexture);
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
        footprint.Footprint.RowPitch = static_cast<UINT>(ego::Align(
            _region.m_bufferRowPitch,
            static_cast<uint32_t>(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
        ));
    }

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = srcBuffer->getD3D12Resource();
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.PlacedFootprint = footprint;

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = dstTexture->getD3D12Resource();
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

    m_commandList->CopyTextureRegion(
        &dstLocation,
        _region.m_textureOffset.m_x,
        _region.m_textureOffset.m_y,
        _region.m_textureOffset.m_z,
        &srcLocation,
        &srcBox
    );
}

void ego::gpu::d3d12::D3D12CommandListBase::copyTextureToBufferInternal(
    const TextureReference& _srcTexture,
    const BufferReference& _dstBuffer,
    const BufferTextureCopyRegionDesc& _region
)
{
    D3D12Texture2D* srcTexture = GetD3D12Texture2D(_srcTexture);
    D3D12Buffer* dstBuffer = GetD3D12Buffer(_dstBuffer);
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
        footprint.Footprint.RowPitch = static_cast<UINT>(ego::Align(
            _region.m_bufferRowPitch,
            static_cast<uint32_t>(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
        ));
    }

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = srcTexture->getD3D12Resource();
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = subresourceIndex;

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = dstBuffer->getD3D12Resource();
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
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>&& _commandList
)
    : D3D12CommandListBase(_device, CommandType::Copy, std::move(_allocator), std::move(_commandList))
{}

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
}

void ego::gpu::d3d12::D3D12CopyCommandList::reset()
{
    resetInternal();
}

void ego::gpu::d3d12::D3D12CopyCommandList::end()
{
    endInternal();
}

void ego::gpu::d3d12::D3D12CopyCommandList::resourceBarrier(
    const GraphicResourceReference& _resource,
    GraphicResourceState _prevState,
    GraphicResourceState _nextState
)
{
    resourceBarrierInternal(_resource, _prevState, _nextState);
}

void ego::gpu::d3d12::D3D12CopyCommandList::bindResourceView(uint32_t, const ResourceViewReference&)
{
    EGO_ASSERT_FAIL_MESSAGE("Copy command list does not support resource bindings");
}

void ego::gpu::d3d12::D3D12CopyCommandList::bindSampler(uint32_t, const SamplerReference&)
{
    EGO_ASSERT_FAIL_MESSAGE("Copy command list does not support sampler bindings");
}

uint32_t ego::gpu::d3d12::D3D12CopyCommandList::getResourceViewBindlessIndex(uint32_t) const
{
    return InvalidBindlessIndex;
}

uint32_t ego::gpu::d3d12::D3D12CopyCommandList::getSamplerBindlessIndex(uint32_t) const
{
    return InvalidBindlessIndex;
}

void ego::gpu::d3d12::D3D12CopyCommandList::pushConstants(ShaderStageFlags, uint32_t, uint32_t, const void*)
{
    EGO_ASSERT_FAIL_MESSAGE("Copy command list does not support push constants");
}

void ego::gpu::d3d12::D3D12CopyCommandList::copyBuffer(
    const BufferReference& _srcBuffer,
    const BufferReference& _dstBuffer,
    const BufferCopyRegionDesc& _region
)
{
    copyBufferInternal(_srcBuffer, _dstBuffer, _region);
}

void ego::gpu::d3d12::D3D12CopyCommandList::copyTexture(
    const TextureReference& _srcTexture,
    const TextureReference& _dstTexture,
    const TextureCopyRegionDesc& _region
)
{
    copyTextureInternal(_srcTexture, _dstTexture, _region);
}

void ego::gpu::d3d12::D3D12CopyCommandList::copyBufferToTexture(
    const BufferReference& _srcBuffer,
    const TextureReference& _dstTexture,
    const BufferTextureCopyRegionDesc& _region
)
{
    copyBufferToTextureInternal(_srcBuffer, _dstTexture, _region);
}

void ego::gpu::d3d12::D3D12CopyCommandList::copyTextureToBuffer(
    const TextureReference& _srcTexture,
    const BufferReference& _dstBuffer,
    const BufferTextureCopyRegionDesc& _region
)
{
    copyTextureToBufferInternal(_srcTexture, _dstBuffer, _region);
}

ego::gpu::d3d12::D3D12ComputeCommandList::D3D12ComputeCommandList(
    D3D12GraphicDevice* _device,
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>&& _commandList
)
    : D3D12CommandListBase(_device, CommandType::Compute, std::move(_allocator), std::move(_commandList))
{}

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
}

void ego::gpu::d3d12::D3D12ComputeCommandList::reset()
{
    resetInternal();
}

void ego::gpu::d3d12::D3D12ComputeCommandList::end()
{
    endInternal();
}

void ego::gpu::d3d12::D3D12ComputeCommandList::resourceBarrier(
    const GraphicResourceReference& _resource,
    GraphicResourceState _prevState,
    GraphicResourceState _nextState
)
{
    resourceBarrierInternal(_resource, _prevState, _nextState);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::bindResourceView(
    uint32_t _slot,
    const ResourceViewReference& _resourceView
)
{
    bindResourceViewInternal(_slot, _resourceView);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::bindSampler(
    uint32_t _slot,
    const SamplerReference& _sampler
)
{
    bindSamplerInternal(_slot, _sampler);
}

uint32_t ego::gpu::d3d12::D3D12ComputeCommandList::getResourceViewBindlessIndex(uint32_t _slot) const
{
    return getResourceViewBindlessIndexInternal(_slot);
}

uint32_t ego::gpu::d3d12::D3D12ComputeCommandList::getSamplerBindlessIndex(uint32_t _slot) const
{
    return getSamplerBindlessIndexInternal(_slot);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::pushConstants(
    ShaderStageFlags _stageFlags,
    uint32_t _offset,
    uint32_t _size,
    const void* _data
)
{
    pushConstantsInternal(_stageFlags, _offset, _size, _data);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::copyBuffer(
    const BufferReference& _srcBuffer,
    const BufferReference& _dstBuffer,
    const BufferCopyRegionDesc& _region
)
{
    copyBufferInternal(_srcBuffer, _dstBuffer, _region);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::copyTexture(
    const TextureReference& _srcTexture,
    const TextureReference& _dstTexture,
    const TextureCopyRegionDesc& _region
)
{
    copyTextureInternal(_srcTexture, _dstTexture, _region);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::copyBufferToTexture(
    const BufferReference& _srcBuffer,
    const TextureReference& _dstTexture,
    const BufferTextureCopyRegionDesc& _region
)
{
    copyBufferToTextureInternal(_srcBuffer, _dstTexture, _region);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::copyTextureToBuffer(
    const TextureReference& _srcTexture,
    const BufferReference& _dstBuffer,
    const BufferTextureCopyRegionDesc& _region
)
{
    copyTextureToBufferInternal(_srcTexture, _dstBuffer, _region);
}

void ego::gpu::d3d12::D3D12ComputeCommandList::setPipeline(const ComputePipelineReference& _pipeline)
{
    D3D12ComputePipeline* pipeline = _pipeline ? static_cast<D3D12ComputePipeline*>(_pipeline.getObject()) : nullptr;
    EGO_ASSERT_MESSAGE(pipeline, "Compute pipeline must be created by D3D12 device");
    if (!pipeline)
    {
        return;
    }

    getD3D12CommandList()->SetPipelineState(pipeline->getPipelineState());
    getD3D12CommandList()->SetComputeRootSignature(
        pipeline->getBindingLayout() ? pipeline->getBindingLayout()->getRootSignature() : nullptr
    );
    bindBindlessDescriptorHeapsInternal();
    m_currentBindingLayout = pipeline->getBindingLayout();
    m_currentPipelineType = PipelineType::Compute;
}

void ego::gpu::d3d12::D3D12ComputeCommandList::dispatch(
    uint32_t _threadGroupsX,
    uint32_t _threadGroupsY,
    uint32_t _threadGroupsZ
)
{
    getD3D12CommandList()->Dispatch(_threadGroupsX, _threadGroupsY, _threadGroupsZ);
}

ego::gpu::d3d12::D3D12GraphicCommandList::D3D12GraphicCommandList(
    D3D12GraphicDevice* _device,
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& _allocator,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>&& _commandList
)
    : D3D12CommandListBase(_device, CommandType::Graphic, std::move(_allocator), std::move(_commandList))
{}

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
}

void ego::gpu::d3d12::D3D12GraphicCommandList::reset()
{
    resetInternal();
}

void ego::gpu::d3d12::D3D12GraphicCommandList::end()
{
    endInternal();
}

void ego::gpu::d3d12::D3D12GraphicCommandList::resourceBarrier(
    const GraphicResourceReference& _resource,
    GraphicResourceState _prevState,
    GraphicResourceState _nextState
)
{
    resourceBarrierInternal(_resource, _prevState, _nextState);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::bindResourceView(
    uint32_t _slot,
    const ResourceViewReference& _resourceView
)
{
    bindResourceViewInternal(_slot, _resourceView);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::bindSampler(
    uint32_t _slot,
    const SamplerReference& _sampler
)
{
    bindSamplerInternal(_slot, _sampler);
}

uint32_t ego::gpu::d3d12::D3D12GraphicCommandList::getResourceViewBindlessIndex(uint32_t _slot) const
{
    return getResourceViewBindlessIndexInternal(_slot);
}

uint32_t ego::gpu::d3d12::D3D12GraphicCommandList::getSamplerBindlessIndex(uint32_t _slot) const
{
    return getSamplerBindlessIndexInternal(_slot);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::pushConstants(
    ShaderStageFlags _stageFlags,
    uint32_t _offset,
    uint32_t _size,
    const void* _data
)
{
    pushConstantsInternal(_stageFlags, _offset, _size, _data);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::copyBuffer(
    const BufferReference& _srcBuffer,
    const BufferReference& _dstBuffer,
    const BufferCopyRegionDesc& _region
)
{
    copyBufferInternal(_srcBuffer, _dstBuffer, _region);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::copyTexture(
    const TextureReference& _srcTexture,
    const TextureReference& _dstTexture,
    const TextureCopyRegionDesc& _region
)
{
    copyTextureInternal(_srcTexture, _dstTexture, _region);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::copyBufferToTexture(
    const BufferReference& _srcBuffer,
    const TextureReference& _dstTexture,
    const BufferTextureCopyRegionDesc& _region
)
{
    copyBufferToTextureInternal(_srcBuffer, _dstTexture, _region);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::copyTextureToBuffer(
    const TextureReference& _srcTexture,
    const BufferReference& _dstBuffer,
    const BufferTextureCopyRegionDesc& _region
)
{
    copyTextureToBufferInternal(_srcTexture, _dstBuffer, _region);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::setPipeline(const ComputePipelineReference& _pipeline)
{
    D3D12ComputePipeline* pipeline = _pipeline ? static_cast<D3D12ComputePipeline*>(_pipeline.getObject()) : nullptr;
    EGO_ASSERT_MESSAGE(pipeline, "Compute pipeline must be created by D3D12 device");
    if (!pipeline)
    {
        return;
    }

    getD3D12CommandList()->SetPipelineState(pipeline->getPipelineState());
    getD3D12CommandList()->SetComputeRootSignature(
        pipeline->getBindingLayout() ? pipeline->getBindingLayout()->getRootSignature() : nullptr
    );
    bindBindlessDescriptorHeapsInternal();
    m_currentBindingLayout = pipeline->getBindingLayout();
    m_currentPipelineType = PipelineType::Compute;
}

void ego::gpu::d3d12::D3D12GraphicCommandList::dispatch(
    uint32_t _threadGroupsX,
    uint32_t _threadGroupsY,
    uint32_t _threadGroupsZ
)
{
    getD3D12CommandList()->Dispatch(_threadGroupsX, _threadGroupsY, _threadGroupsZ);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::beginRendering(const RenderingDesc& _desc)
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
    rtvHandles.reserve(_desc.m_colorAttachments.size());

    for (const ColorAttachmentDesc& colorAttachment : _desc.m_colorAttachments)
    {
        D3D12TextureView* textureView = colorAttachment.m_view ?
                                            static_cast<D3D12TextureView*>(colorAttachment.m_view.getObject()) :
                                            nullptr;
        EGO_ASSERT_MESSAGE(textureView, "Render target view must be created by D3D12 device");
        if (!textureView)
        {
            continue;
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE handle = textureView->getCpuDescriptorHandle();
        rtvHandles.push_back(handle);

        if (colorAttachment.m_loadOperation == AttachmentLoadOperation::Clear)
        {
            getD3D12CommandList()->ClearRenderTargetView(handle, colorAttachment.m_clearValue.m_values, 0, nullptr);
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};
    D3D12TextureView* depthView = _desc.m_depthStencilAttachment.m_view ?
                                      static_cast<D3D12TextureView*>(_desc.m_depthStencilAttachment.m_view.getObject()) :
                                      nullptr;
    if (depthView)
    {
        dsvHandle = depthView->getCpuDescriptorHandle();

        D3D12_CLEAR_FLAGS clearFlags = static_cast<D3D12_CLEAR_FLAGS>(0);
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
                nullptr
            );
        }
    }

    getD3D12CommandList()->OMSetRenderTargets(
        static_cast<UINT>(rtvHandles.size()),
        rtvHandles.empty() ? nullptr : rtvHandles.data(),
        FALSE,
        depthView ? &dsvHandle : nullptr
    );
}

void ego::gpu::d3d12::D3D12GraphicCommandList::endRendering()
{}

void ego::gpu::d3d12::D3D12GraphicCommandList::setPipeline(const GraphicPipelineReference& _pipeline)
{
    D3D12GraphicPipeline* pipeline = _pipeline ? static_cast<D3D12GraphicPipeline*>(_pipeline.getObject()) : nullptr;
    EGO_ASSERT_MESSAGE(pipeline, "Graphic pipeline must be created by D3D12 device");
    if (!pipeline)
    {
        return;
    }

    getD3D12CommandList()->SetPipelineState(pipeline->getPipelineState());
    getD3D12CommandList()->SetGraphicsRootSignature(
        pipeline->getBindingLayout() ? pipeline->getBindingLayout()->getRootSignature() : nullptr
    );
    bindBindlessDescriptorHeapsInternal();
    getD3D12CommandList()->IASetPrimitiveTopology(pipeline->getPrimitiveTopology());
    m_currentBindingLayout = pipeline->getBindingLayout();
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

void ego::gpu::d3d12::D3D12GraphicCommandList::setVertexBuffer(
    uint32_t _slot,
    const BufferReference& _buffer,
    uint32_t _stride,
    uint32_t _offset
)
{
    D3D12Buffer* buffer = GetD3D12Buffer(_buffer);
    if (!buffer)
    {
        return;
    }

    const D3D12_RESOURCE_DESC bufferDesc = buffer->getD3D12ResourceDesc();
    D3D12_VERTEX_BUFFER_VIEW view = {};
    view.BufferLocation = buffer->getD3D12Resource()->GetGPUVirtualAddress() + _offset;
    view.StrideInBytes = _stride;
    view.SizeInBytes = static_cast<UINT>(bufferDesc.Width - _offset);
    getD3D12CommandList()->IASetVertexBuffers(_slot, 1, &view);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::setIndexBuffer(
    const BufferReference& _buffer,
    GraphicResourceFormat _format,
    uint32_t _offset
)
{
    D3D12Buffer* buffer = GetD3D12Buffer(_buffer);
    if (!buffer)
    {
        return;
    }

    const D3D12_RESOURCE_DESC bufferDesc = buffer->getD3D12ResourceDesc();
    D3D12_INDEX_BUFFER_VIEW view = {};
    view.BufferLocation = buffer->getD3D12Resource()->GetGPUVirtualAddress() + _offset;
    view.Format = ToDXGIFormat(_format);
    view.SizeInBytes = static_cast<UINT>(bufferDesc.Width - _offset);
    getD3D12CommandList()->IASetIndexBuffer(&view);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::draw(
    uint32_t _vertexCount,
    uint32_t _instanceCount,
    uint32_t _firstVertex,
    uint32_t _firstInstance
)
{
    getD3D12CommandList()->DrawInstanced(_vertexCount, _instanceCount, _firstVertex, _firstInstance);
}

void ego::gpu::d3d12::D3D12GraphicCommandList::drawIndexed(
    uint32_t _indexCount,
    uint32_t _instanceCount,
    uint32_t _firstIndex,
    int32_t _vertexOffset,
    uint32_t _firstInstance
)
{
    getD3D12CommandList()->DrawIndexedInstanced(
        _indexCount,
        _instanceCount,
        _firstIndex,
        _vertexOffset,
        _firstInstance
    );
}
