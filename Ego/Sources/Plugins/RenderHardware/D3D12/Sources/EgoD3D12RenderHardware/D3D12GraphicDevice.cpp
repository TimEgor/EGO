#include "D3D12GraphicDevice.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "EgoCore/Memory/Utils.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Platform/Window/Window.h"
#include "EgoPlugin/PluginController.h"

#include "Objects/D3D12AccelerationStructure.h"
#include "Objects/D3D12Buffer.h"
#include "Objects/D3D12CommandList.h"
#include "Objects/D3D12CommandQueue.h"
#include "Objects/D3D12Fence.h"
#include "Objects/D3D12Pipeline.h"
#include "Objects/D3D12Shader.h"
#include "Objects/D3D12SwapChain.h"
#include "Objects/D3D12Texture.h"

namespace
{
    constexpr auto RayGenerationEntryPointName = L"RayGenerationMain";
    constexpr auto MissEntryPointName = L"MissMain";
    constexpr auto ClosestHitEntryPointName = L"ClosestHitMain";
    constexpr auto AnyHitEntryPointName = L"AnyHitMain";
    constexpr auto IntersectionEntryPointName = L"IntersectionMain";
    constexpr auto HitGroupExportName = L"HitGroup";

    std::wstring MakeIndexedExportName(const wchar_t* _baseName, size_t _index)
    {
        return std::wstring(_baseName) + std::to_wstring(_index);
    }

    D3D12_HIT_GROUP_TYPE GetD3D12HitGroupType(ego::gpu::RayTracingHitGroupType _type)
    {
        return _type == ego::gpu::RayTracingHitGroupType::ProceduralPrimitive ? D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE : D3D12_HIT_GROUP_TYPE_TRIANGLES;
    }
} // namespace

template <typename TCommandListReference, typename TCommandListObject>
TCommandListReference ego::gpu::d3d12::D3D12GraphicDevice::createCommandList(D3D12_COMMAND_LIST_TYPE _type)
{
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> commandList;

    if (!getD3D12Device())
    {
        return TCommandListReference();
    }

    if (FAILED(getD3D12Device()->CreateCommandAllocator(_type, IID_PPV_ARGS(&allocator))))
    {
        return TCommandListReference();
    }

    if (FAILED(getD3D12Device()->CreateCommandList(0, _type, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList))))
    {
        return TCommandListReference();
    }

    commandList->Close();
    return TCommandListReference(new TCommandListObject(this, &m_descriptorFactory, std::move(allocator), std::move(commandList)));
}

bool ego::gpu::d3d12::D3D12GraphicDevice::createUploadBuffer(uint64_t _size, Microsoft::WRL::ComPtr<ID3D12Resource>& _resource) const
{
    if (!getD3D12Device() || !_size)
    {
        return false;
    }

    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = _size;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    return SUCCEEDED(
        getD3D12Device()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_resource)));
}

ego::Reference<ego::gpu::d3d12::D3D12Buffer> ego::gpu::d3d12::D3D12GraphicDevice::createUploadD3D12Buffer(uint64_t _size)
{
    if (!FitsUint32(_size))
    {
        return ego::Reference<D3D12Buffer>();
    }

    BufferDesc uploadDesc;
    uploadDesc.m_usage = GraphicResourceUsageTransferSrc;
    uploadDesc.m_access = static_cast<CommonGraphicResourceAccess>(GraphicResourceAccessCpuWrite | GraphicResourceAccessGpuRead);
    uploadDesc.m_size = static_cast<uint32_t>(_size);
    uploadDesc.m_stride = 1;

    return createD3D12Buffer(uploadDesc);
}

ego::gpu::GpuTaskReference ego::gpu::d3d12::D3D12GraphicDevice::uploadBufferToDefaultHeap(
    ID3D12Resource* _dstResource,
    uint64_t _dstSize,
    const InitialGraphicResourceData& _initialData)
{
    const uint64_t uploadSize = (std::min<uint64_t>)(_initialData.m_dataSize, _dstSize);
    ego::Reference<D3D12Buffer> uploadBuffer = createUploadD3D12Buffer(uploadSize);

    if (!uploadBuffer || !WriteToUploadBuffer(uploadBuffer->getD3D12Resource(), _initialData.m_data, uploadSize, uploadSize))
    {
        return GpuTaskReference();
    }

    return submitImmediateCommands(
        [&](ID3D12GraphicsCommandList* _commandList)
        {
            D3D12_RESOURCE_BARRIER beforeCopyBarrier = {};
            beforeCopyBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            beforeCopyBarrier.Transition.pResource = _dstResource;
            beforeCopyBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
            beforeCopyBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            beforeCopyBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            _commandList->ResourceBarrier(1, &beforeCopyBarrier);

            _commandList->CopyBufferRegion(_dstResource, 0, uploadBuffer->getD3D12Resource(), 0, uploadSize);

            D3D12_RESOURCE_BARRIER afterCopyBarrier = {};
            afterCopyBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            afterCopyBarrier.Transition.pResource = _dstResource;
            afterCopyBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            afterCopyBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
            afterCopyBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            _commandList->ResourceBarrier(1, &afterCopyBarrier);
        },
        std::vector<GraphicObjectReference>{uploadBuffer});
}

ego::gpu::GpuTaskReference ego::gpu::d3d12::D3D12GraphicDevice::uploadTexture2DToDefaultHeap(
    ID3D12Resource* _dstResource,
    const D3D12_RESOURCE_DESC& _dstDesc,
    const InitialGraphicResourceData& _initialData)
{
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
    UINT numRows = 0;
    UINT64 rowSizeInBytes = 0;
    UINT64 totalBytes = 0;
    if (!getD3D12Device())
    {
        return GpuTaskReference();
    }

    getD3D12Device()->GetCopyableFootprints(&_dstDesc, 0, 1, 0, &footprint, &numRows, &rowSizeInBytes, &totalBytes);

    ego::Reference<D3D12Buffer> uploadBuffer = createUploadD3D12Buffer(totalBytes);
    if (!uploadBuffer)
    {
        return GpuTaskReference();
    }

    void* mappedData = nullptr;
    D3D12_RANGE readRange = {};
    if (FAILED(uploadBuffer->getD3D12Resource()->Map(0, &readRange, &mappedData)))
    {
        return GpuTaskReference();
    }

    auto srcBytes = static_cast<const uint8_t*>(_initialData.m_data);
    uint8_t* dstBytes = static_cast<uint8_t*>(mappedData) + footprint.Offset;
    const uint64_t srcRowPitch = _initialData.m_rowPitch ? _initialData.m_rowPitch : rowSizeInBytes;
    const uint64_t rowsToCopy = (std::min<uint64_t>)(numRows, _initialData.m_dataSize / srcRowPitch);
    const uint64_t rowBytesToCopy = (std::min<uint64_t>)(rowSizeInBytes, srcRowPitch);

    for (uint64_t row = 0; row < rowsToCopy; ++row)
    {
        std::memcpy(dstBytes + row * footprint.Footprint.RowPitch, srcBytes + row * srcRowPitch, static_cast<size_t>(rowBytesToCopy));
    }

    D3D12_RANGE writtenRange = {};
    writtenRange.Begin = static_cast<SIZE_T>(footprint.Offset);
    writtenRange.End = static_cast<SIZE_T>(footprint.Offset + totalBytes);
    uploadBuffer->getD3D12Resource()->Unmap(0, &writtenRange);

    return submitImmediateCommands(
        [&](ID3D12GraphicsCommandList* _commandList)
        {
            D3D12_RESOURCE_BARRIER beforeCopyBarrier = {};
            beforeCopyBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            beforeCopyBarrier.Transition.pResource = _dstResource;
            beforeCopyBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
            beforeCopyBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            beforeCopyBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            _commandList->ResourceBarrier(1, &beforeCopyBarrier);

            D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
            srcLocation.pResource = uploadBuffer->getD3D12Resource();
            srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            srcLocation.PlacedFootprint = footprint;

            D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
            dstLocation.pResource = _dstResource;
            dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dstLocation.SubresourceIndex = 0;

            _commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

            D3D12_RESOURCE_BARRIER afterCopyBarrier = {};
            afterCopyBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            afterCopyBarrier.Transition.pResource = _dstResource;
            afterCopyBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            afterCopyBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
            afterCopyBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            _commandList->ResourceBarrier(1, &afterCopyBarrier);
        },
        std::vector<GraphicObjectReference>{uploadBuffer});
}

bool ego::gpu::d3d12::D3D12GraphicDevice::createShaderTable(
    ID3D12StateObject* _stateObject,
    const std::vector<std::wstring>& _hitGroupExportNames,
    Microsoft::WRL::ComPtr<ID3D12Resource>& _shaderTable,
    uint64_t& _shaderRecordSize) const
{
    if (!getD3D12Device() || !_stateObject || _hitGroupExportNames.empty())
    {
        return false;
    }

    Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
    if (FAILED(_stateObject->QueryInterface(IID_PPV_ARGS(&stateObjectProperties))))
    {
        return false;
    }

    const void* rayGenerationIdentifier = stateObjectProperties->GetShaderIdentifier(RayGenerationEntryPointName);
    const void* missIdentifier = stateObjectProperties->GetShaderIdentifier(MissEntryPointName);
    if (!rayGenerationIdentifier || !missIdentifier)
    {
        return false;
    }

    std::vector<const void*> hitGroupIdentifiers;
    hitGroupIdentifiers.reserve(_hitGroupExportNames.size());
    for (const std::wstring& hitGroupExportName : _hitGroupExportNames)
    {
        const void* hitGroupIdentifier = stateObjectProperties->GetShaderIdentifier(hitGroupExportName.c_str());
        if (!hitGroupIdentifier)
        {
            return false;
        }

        hitGroupIdentifiers.push_back(hitGroupIdentifier);
    }

    _shaderRecordSize = ego::Align(static_cast<uint64_t>(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES), static_cast<uint64_t>(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT));
    const uint64_t shaderTableSize = _shaderRecordSize * (2 + hitGroupIdentifiers.size());
    if (!createUploadBuffer(shaderTableSize, _shaderTable))
    {
        return false;
    }

    uint8_t* mappedData = nullptr;
    D3D12_RANGE readRange = {};
    if (FAILED(_shaderTable->Map(0, &readRange, reinterpret_cast<void**>(&mappedData))))
    {
        return false;
    }

    std::memcpy(mappedData, rayGenerationIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    std::memcpy(mappedData + _shaderRecordSize, missIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    for (size_t hitGroupIndex = 0; hitGroupIndex < hitGroupIdentifiers.size(); ++hitGroupIndex)
    {
        std::memcpy(
            mappedData + _shaderRecordSize * (2 + hitGroupIndex),
            hitGroupIdentifiers[hitGroupIndex],
            D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    }

    D3D12_RANGE writtenRange = {};
    writtenRange.Begin = 0;
    writtenRange.End = static_cast<SIZE_T>(shaderTableSize);
    _shaderTable->Unmap(0, &writtenRange);
    return true;
}

template <typename TReference, typename TObject>
ego::gpu::GpuResourceTicket<TReference> ego::gpu::d3d12::D3D12GraphicDevice::buildAccelerationStructure(
    const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& _inputs,
    const GpuOperationOptions& _options,
    const std::vector<GraphicObjectReference>& _keepAliveObjects)
{
    if (!getD3D12Device())
    {
        return GpuResourceTicket<TReference>();
    }

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
    getD3D12Device()->GetRaytracingAccelerationStructurePrebuildInfo(&_inputs, &prebuildInfo);
    if (prebuildInfo.ResultDataMaxSizeInBytes == 0 || prebuildInfo.ScratchDataSizeInBytes == 0 || !FitsUint32(prebuildInfo.ResultDataMaxSizeInBytes) ||
        !FitsUint32(prebuildInfo.ScratchDataSizeInBytes))
    {
        return GpuResourceTicket<TReference>();
    }

    BufferDesc scratchDesc;
    scratchDesc.m_usage = GraphicResourceUsageAllowUnorderedAccess;
    scratchDesc.m_access = static_cast<CommonGraphicResourceAccess>(GraphicResourceAccessGpuRead | GraphicResourceAccessGpuWrite);
    scratchDesc.m_size = static_cast<uint32_t>(prebuildInfo.ScratchDataSizeInBytes);

    ego::Reference<D3D12Buffer> scratchBuffer = createD3D12Buffer(scratchDesc);
    if (!scratchBuffer)
    {
        return GpuResourceTicket<TReference>();
    }

    BufferDesc resultDesc;
    resultDesc.m_usage = static_cast<GraphicResourceUsage>(GraphicResourceUsageAllowUnorderedAccess | GraphicResourceUsageRayTracingAccelerationStructure);
    resultDesc.m_access = static_cast<CommonGraphicResourceAccess>(GraphicResourceAccessGpuRead | GraphicResourceAccessGpuWrite);
    resultDesc.m_size = static_cast<uint32_t>(prebuildInfo.ResultDataMaxSizeInBytes);

    ego::Reference<D3D12Buffer> resultBuffer = createD3D12Buffer(resultDesc);
    if (!resultBuffer)
    {
        return GpuResourceTicket<TReference>();
    }

    std::vector<GraphicObjectReference> keepAliveObjects = _keepAliveObjects;
    keepAliveObjects.push_back(scratchBuffer);

    GpuTaskReference buildTask = submitImmediateCommands(
        [&](ID3D12GraphicsCommandList4* _commandList)
        {
            D3D12_RESOURCE_BARRIER scratchBarrier = {};
            scratchBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            scratchBarrier.Transition.pResource = scratchBuffer->getD3D12Resource();
            scratchBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
            scratchBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            scratchBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            _commandList->ResourceBarrier(1, &scratchBarrier);

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
            buildDesc.Inputs = _inputs;
            buildDesc.ScratchAccelerationStructureData = scratchBuffer->getD3D12Resource()->GetGPUVirtualAddress();
            buildDesc.DestAccelerationStructureData = resultBuffer->getD3D12Resource()->GetGPUVirtualAddress();

            _commandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

            D3D12_RESOURCE_BARRIER resultBarrier = {};
            resultBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            resultBarrier.UAV.pResource = resultBuffer->getD3D12Resource();
            _commandList->ResourceBarrier(1, &resultBarrier);
        },
        keepAliveObjects);
    if (!buildTask)
    {
        return GpuResourceTicket<TReference>();
    }

    resultBuffer->setLastWriteTask(buildTask);

    TReference accelerationStructure(new TObject(resultBuffer));
    accelerationStructure->setLastWriteTask(buildTask);

    if (_options.shouldWait())
    {
        buildTask->wait();
    }

    return GpuResourceTicket<TReference>{accelerationStructure, buildTask};
}

bool ego::gpu::d3d12::D3D12GraphicDevice::init(const GraphicDevice::InitParams& _params)
{
    EGO_CHECK_INITIALIZATION(m_deviceContext.init(_params, D3D12DescriptorFactory::BindlessResourceDescriptorCapacity, D3D12DescriptorFactory::BindlessSamplerDescriptorCapacity));
    EGO_CHECK_INITIALIZATION(m_immediateContext.init(getD3D12Device()));
    EGO_CHECK_INITIALIZATION(m_descriptorFactory.init(getD3D12Device()));
    EGO_CHECK_INITIALIZATION(registerResourceProviders());

    return true;
}

void ego::gpu::d3d12::D3D12GraphicDevice::release()
{
    unregisterResourceProviders();
    m_immediateContext.release();
    m_descriptorFactory.release();
    m_deviceContext.release();
}

void* ego::gpu::d3d12::D3D12GraphicDevice::getNativeHandle() const
{
    return getD3D12Device();
}

void ego::gpu::d3d12::D3D12GraphicDevice::setName(const char* _name)
{
    SetD3D12ObjectName(getD3D12Device(), _name);
}

ego::gpu::CommandQueueReference ego::gpu::d3d12::D3D12GraphicDevice::createCommandQueue(const CommandQueueDesc& _desc)
{
    if (!getD3D12Device())
    {
        return CommandQueueReference();
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = ToD3D12CommandListType(_desc.m_type);
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;
    if (FAILED(getD3D12Device()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue))))
    {
        return CommandQueueReference();
    }

    return CommandQueueReference(new D3D12CommandQueue(_desc, std::move(queue)));
}

ego::gpu::GraphicCommandListReference ego::gpu::d3d12::D3D12GraphicDevice::createGraphicCommandList()
{
    return createCommandList<GraphicCommandListReference, D3D12GraphicCommandList>(D3D12_COMMAND_LIST_TYPE_DIRECT);
}

ego::gpu::ComputeCommandListReference ego::gpu::d3d12::D3D12GraphicDevice::createComputeCommandList()
{
    return createCommandList<ComputeCommandListReference, D3D12ComputeCommandList>(D3D12_COMMAND_LIST_TYPE_COMPUTE);
}

ego::gpu::CopyCommandListReference ego::gpu::d3d12::D3D12GraphicDevice::createCopyCommandList()
{
    return createCommandList<CopyCommandListReference, D3D12CopyCommandList>(D3D12_COMMAND_LIST_TYPE_COPY);
}

ego::gpu::BufferReference ego::gpu::d3d12::D3D12GraphicDevice::createBuffer(const BufferDesc& _desc)
{
    return createD3D12Buffer(_desc);
}

ego::Reference<ego::gpu::d3d12::D3D12Buffer> ego::gpu::d3d12::D3D12GraphicDevice::createD3D12Buffer(const BufferDesc& _desc)
{
    if (!getD3D12Device() || !_desc.m_size)
    {
        return ego::Reference<D3D12Buffer>();
    }

    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = GetHeapType(_desc);
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 0;
    heapProperties.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = _desc.m_size;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = GetBufferResourceFlags(_desc);

    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    if (FAILED(
            getD3D12Device()->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                GetInitialBufferState(_desc, InitialGraphicResourceData()),
                nullptr,
                IID_PPV_ARGS(&resource))))
    {
        return ego::Reference<D3D12Buffer>();
    }

    return ego::Reference<D3D12Buffer>(new D3D12Buffer(_desc, std::move(resource)));
}

ego::gpu::GpuTaskReference ego::gpu::d3d12::D3D12GraphicDevice::uploadBuffer(
    const BufferReference& _buffer,
    const InitialGraphicResourceData& _initialData,
    const GpuOperationOptions& _options)
{
    ID3D12Resource* resource = _buffer ? _buffer->getNativeHandle<ID3D12Resource>() : nullptr;
    if (!resource || !_initialData.isValid())
    {
        return GpuTaskReference();
    }

    const BufferDesc& desc = _buffer->getDesc();
    const D3D12_HEAP_TYPE heapType = GetHeapType(desc);
    if (heapType == D3D12_HEAP_TYPE_UPLOAD)
    {
        if (!WriteToUploadBuffer(resource, _initialData.m_data, _initialData.m_dataSize, desc.m_size))
        {
            return GpuTaskReference();
        }

        GpuTaskReference uploadTask(new GpuTask());
        _buffer->setLastWriteTask(uploadTask);
        return uploadTask;
    }

    if (heapType == D3D12_HEAP_TYPE_DEFAULT)
    {
        GpuTaskReference uploadTask = uploadBufferToDefaultHeap(resource, desc.m_size, _initialData);
        if (!uploadTask)
        {
            return GpuTaskReference();
        }

        _buffer->setLastWriteTask(uploadTask);
        if (_options.shouldWait())
        {
            uploadTask->wait();
        }

        return uploadTask;
    }

    EGO_ASSERT_FAIL_MESSAGE("Initial data upload to readback heap is not supported");
    return GpuTaskReference();
}

ego::gpu::Texture2DReference ego::gpu::d3d12::D3D12GraphicDevice::createTexture2D(const Texture2DDesc& _desc)
{
    if (!getD3D12Device())
    {
        return Texture2DReference();
    }

    EGO_ASSERT_MESSAGE(!(_desc.m_access & (GraphicResourceAccessCpuRead | GraphicResourceAccessCpuWrite)), "D3D12 textures are created in GPU-local memory by this backend");

    const DXGI_FORMAT format = ToDXGIFormat(_desc.m_format);
    if (format == DXGI_FORMAT_UNKNOWN)
    {
        return Texture2DReference();
    }

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = (std::max<uint32_t>)(_desc.m_size.m_x, 1);
    resourceDesc.Height = (std::max<uint32_t>)(_desc.m_size.m_y, 1);
    resourceDesc.DepthOrArraySize = static_cast<UINT16>((std::max<uint32_t>)(_desc.m_arrayLayers, 1));
    resourceDesc.MipLevels = static_cast<UINT16>((std::max<uint32_t>)(_desc.m_mipLevels, 1));
    resourceDesc.Format = format;
    resourceDesc.SampleDesc.Count = (std::max<uint32_t>)(_desc.m_samples.m_count, 1);
    resourceDesc.SampleDesc.Quality = _desc.m_samples.m_quality;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = ToD3D12ResourceFlags(_desc.m_usage);

    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE clearValue = {};
    const D3D12_CLEAR_VALUE* clearValuePtr = nullptr;
    if (_desc.m_usage & TextureUsageRenderTarget)
    {
        clearValue.Format = format;
        clearValue.Color[0] = 0.0f;
        clearValue.Color[1] = 0.0f;
        clearValue.Color[2] = 0.0f;
        clearValue.Color[3] = 1.0f;
        clearValuePtr = &clearValue;
    }
    else if (_desc.m_usage & TextureUsageDepthStencil)
    {
        clearValue.Format = format;
        clearValue.DepthStencil.Depth = 1.0f;
        clearValue.DepthStencil.Stencil = 0;
        clearValuePtr = &clearValue;
    }

    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    if (FAILED(
            getD3D12Device()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, clearValuePtr, IID_PPV_ARGS(&resource))))
    {
        return Texture2DReference();
    }

    return Texture2DReference(new D3D12Texture2D(_desc, std::move(resource)));
}

ego::gpu::GpuTaskReference ego::gpu::d3d12::D3D12GraphicDevice::uploadTexture2D(
    const Texture2DReference& _texture,
    const InitialGraphicResourceData& _initialData,
    const GpuOperationOptions& _options)
{
    D3D12Texture2D* d3d12Texture = _texture ? static_cast<D3D12Texture2D*>(_texture.getObject()) : nullptr;
    if (!d3d12Texture || !_initialData.isValid())
    {
        return GpuTaskReference();
    }

    EGO_ASSERT_MESSAGE(_texture->getDesc().m_samples.m_count <= 1, "Initial data upload for multisampled textures is not supported");
    if (_texture->getDesc().m_samples.m_count > 1)
    {
        return GpuTaskReference();
    }

    GpuTaskReference uploadTask = uploadTexture2DToDefaultHeap(d3d12Texture->getD3D12Resource(), d3d12Texture->getD3D12Resource()->GetDesc(), _initialData);
    if (!uploadTask)
    {
        return GpuTaskReference();
    }

    _texture->setLastWriteTask(uploadTask);
    if (_options.shouldWait())
    {
        uploadTask->wait();
    }

    return uploadTask;
}

ego::gpu::SamplerReference ego::gpu::d3d12::D3D12GraphicDevice::createSampler(const SamplerDesc& _desc)
{
    return m_descriptorFactory.createSampler(_desc);
}

ego::gpu::BufferViewReference ego::gpu::d3d12::D3D12GraphicDevice::createBufferView(const BufferReference& _buffer, const BufferViewDesc& _desc)
{
    return m_descriptorFactory.createBufferView(_buffer, _desc);
}

ego::gpu::TextureViewReference ego::gpu::d3d12::D3D12GraphicDevice::createTextureView(const TextureReference& _texture, const TextureViewDesc& _desc)
{
    return m_descriptorFactory.createTextureView(_texture, _desc);
}

ego::gpu::VertexShaderReference ego::gpu::d3d12::D3D12GraphicDevice::createVertexShader(const ShaderCodeReference& _code)
{
    if (!_code || !_code->getCode() || !_code->getCodeSize())
    {
        return VertexShaderReference();
    }

    return VertexShaderReference(new D3D12VertexShader(_code));
}

ego::gpu::PixelShaderReference ego::gpu::d3d12::D3D12GraphicDevice::createPixelShader(const ShaderCodeReference& _code)
{
    if (!_code || !_code->getCode() || !_code->getCodeSize())
    {
        return PixelShaderReference();
    }

    return PixelShaderReference(new D3D12PixelShader(_code));
}

ego::gpu::ComputeShaderReference ego::gpu::d3d12::D3D12GraphicDevice::createComputeShader(const ShaderCodeReference& _code)
{
    if (!IsValidShaderCode(_code))
    {
        return ComputeShaderReference();
    }

    return ComputeShaderReference(new D3D12ComputeShader(_code));
}

ego::gpu::RayGenerationShaderReference ego::gpu::d3d12::D3D12GraphicDevice::createRayGenerationShader(const ShaderCodeReference& _code)
{
    if (!IsValidShaderCode(_code))
    {
        return RayGenerationShaderReference();
    }

    return RayGenerationShaderReference(new D3D12RayGenerationShader(_code));
}

ego::gpu::MissShaderReference ego::gpu::d3d12::D3D12GraphicDevice::createMissShader(const ShaderCodeReference& _code)
{
    if (!IsValidShaderCode(_code))
    {
        return MissShaderReference();
    }

    return MissShaderReference(new D3D12MissShader(_code));
}

ego::gpu::ClosestHitShaderReference ego::gpu::d3d12::D3D12GraphicDevice::createClosestHitShader(const ShaderCodeReference& _code)
{
    if (!IsValidShaderCode(_code))
    {
        return ClosestHitShaderReference();
    }

    return ClosestHitShaderReference(new D3D12ClosestHitShader(_code));
}

ego::gpu::AnyHitShaderReference ego::gpu::d3d12::D3D12GraphicDevice::createAnyHitShader(const ShaderCodeReference& _code)
{
    if (!IsValidShaderCode(_code))
    {
        return AnyHitShaderReference();
    }

    return AnyHitShaderReference(new D3D12AnyHitShader(_code));
}

ego::gpu::IntersectionShaderReference ego::gpu::d3d12::D3D12GraphicDevice::createIntersectionShader(const ShaderCodeReference& _code)
{
    if (!IsValidShaderCode(_code))
    {
        return IntersectionShaderReference();
    }

    return IntersectionShaderReference(new D3D12IntersectionShader(_code));
}

ego::gpu::GpuGeometryAccelerationStructureTicket ego::gpu::d3d12::D3D12GraphicDevice::buildGeometryAccelerationStructure(
    const MeshAccelerationStructureBuildDesc& _desc,
    const GpuOperationOptions& _options)
{
    if (!getD3D12Device() || _desc.m_geometries.empty())
    {
        return GpuGeometryAccelerationStructureTicket();
    }

    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs;
    geometryDescs.reserve(_desc.m_geometries.size());

    for (const GeometryAccelerationStructureDesc& geometry : _desc.m_geometries)
    {
        ID3D12Resource* vertexBuffer = geometry.m_vertexBuffer ? geometry.m_vertexBuffer->getNativeHandle<ID3D12Resource>() : nullptr;
        ID3D12Resource* indexBuffer = geometry.m_indexBuffer ? geometry.m_indexBuffer->getNativeHandle<ID3D12Resource>() : nullptr;

        if (!vertexBuffer || geometry.m_vertexStride == 0 || geometry.m_vertexCount == 0 || !IsSupportedRayTracingIndexFormat(geometry.m_indexFormat) ||
            (geometry.m_indexCount != 0 && !indexBuffer))
        {
            return GpuGeometryAccelerationStructureTicket();
        }

        if (!geometry.m_vertexBuffer->isGpuReady() || (geometry.m_indexBuffer && !geometry.m_indexBuffer->isGpuReady()))
        {
            return GpuGeometryAccelerationStructureTicket();
        }

        D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
        geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress() + geometry.m_vertexOffset;
        geometryDesc.Triangles.VertexBuffer.StrideInBytes = geometry.m_vertexStride;
        geometryDesc.Triangles.VertexCount = geometry.m_vertexCount;
        geometryDesc.Triangles.VertexFormat = ToDXGIFormat(geometry.m_vertexFormat);
        geometryDesc.Triangles.IndexFormat = ToDXGIFormat(geometry.m_indexFormat);
        geometryDesc.Triangles.IndexCount = geometry.m_indexCount;
        geometryDesc.Triangles.IndexBuffer = indexBuffer ? indexBuffer->GetGPUVirtualAddress() + geometry.m_indexOffset : 0;
        if (geometryDesc.Triangles.VertexFormat == DXGI_FORMAT_UNKNOWN)
        {
            return GpuGeometryAccelerationStructureTicket();
        }

        geometryDescs.push_back(geometryDesc);
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.NumDescs = static_cast<UINT>(geometryDescs.size());
    inputs.pGeometryDescs = geometryDescs.data();

    GpuGeometryAccelerationStructureTicket accelerationStructure =
        buildAccelerationStructure<GeometryAccelerationStructureReference, D3D12GeometryAccelerationStructure>(inputs, _options);

    return accelerationStructure;
}

ego::gpu::GpuInstanceAccelerationStructureTicket ego::gpu::d3d12::D3D12GraphicDevice::buildInstanceAccelerationStructure(
    const InstanceAccelerationStructureBuildDesc& _desc,
    const GpuOperationOptions& _options)
{
    if (!getD3D12Device() || _desc.m_instances.empty())
    {
        return GpuInstanceAccelerationStructureTicket();
    }

    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs;
    instanceDescs.reserve(_desc.m_instances.size());

    for (const InstanceGeometryAccelerationStructureBuildDesc& instance : _desc.m_instances)
    {
        ID3D12Resource* geometry = instance.m_geometry ? instance.m_geometry->getNativeHandle<ID3D12Resource>() : nullptr;
        if (!geometry || !instance.m_geometry->isGpuReady())
        {
            return GpuInstanceAccelerationStructureTicket();
        }

        D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
        const ego::FloatMatrix4x4& transform = instance.m_transform;
        // Ego stores transform columns in FloatMatrix4x4 rows, while DXR expects a row-major 3x4 matrix.
        for (uint32_t row = 0; row < 3; ++row)
        {
            for (uint32_t column = 0; column < 4; ++column)
            {
                instanceDesc.Transform[row][column] = transform.m_values[column][row];
            }
        }

        instanceDesc.InstanceID = instance.m_instanceId;
        instanceDesc.InstanceMask = instance.m_instanceMask & 0xff;
        instanceDesc.InstanceContributionToHitGroupIndex = instance.m_hitGroupIndex;
        instanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
        instanceDesc.AccelerationStructure = geometry->GetGPUVirtualAddress();
        instanceDescs.push_back(instanceDesc);
    }

    const uint64_t instanceBufferSize = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instanceDescs.size();
    if (!FitsUint32(instanceBufferSize))
    {
        return GpuInstanceAccelerationStructureTicket();
    }

    BufferDesc instanceBufferDesc;
    instanceBufferDesc.m_usage = GraphicResourceUsageShaderResource;
    instanceBufferDesc.m_access = static_cast<CommonGraphicResourceAccess>(GraphicResourceAccessCpuWrite | GraphicResourceAccessGpuRead);
    instanceBufferDesc.m_size = static_cast<uint32_t>(instanceBufferSize);
    instanceBufferDesc.m_stride = sizeof(D3D12_RAYTRACING_INSTANCE_DESC);

    const InitialGraphicResourceData instanceData(instanceDescs.data(), static_cast<uint32_t>(instanceBufferSize));
    const GpuOperationOptions uploadOptions{GpuCompletionMode::WaitForCompletion};
    ego::Reference<D3D12Buffer> instanceBuffer = createD3D12Buffer(instanceBufferDesc);
    if (!instanceBuffer)
    {
        return GpuInstanceAccelerationStructureTicket();
    }

    GpuTaskReference instanceUploadTask = uploadBuffer(instanceBuffer, instanceData, uploadOptions);
    if (!instanceUploadTask)
    {
        return GpuInstanceAccelerationStructureTicket();
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.NumDescs = static_cast<UINT>(instanceDescs.size());
    inputs.InstanceDescs = instanceBuffer->getD3D12Resource()->GetGPUVirtualAddress();

    GpuInstanceAccelerationStructureTicket accelerationStructure = buildAccelerationStructure<InstanceAccelerationStructureReference, D3D12InstanceAccelerationStructure>(
        inputs,
        _options,
        std::vector<GraphicObjectReference>{instanceBuffer});

    return accelerationStructure;
}

ego::gpu::AccelerationStructureViewReference ego::gpu::d3d12::D3D12GraphicDevice::createAccelerationStructureView(
    const InstanceAccelerationStructureReference& _accelerationStructure)
{
    return m_descriptorFactory.createAccelerationStructureView(_accelerationStructure);
}

ego::gpu::GraphicPipelineReference ego::gpu::d3d12::D3D12GraphicDevice::createGraphicPipeline(const GraphicPipelineDesc& _desc)
{
    const bool hasValidVertexShader = !_desc.m_vertexShader || _desc.m_vertexShader->getShaderType() == ShaderStage::Vertex;
    const bool hasValidPixelShader = !_desc.m_pixelShader || _desc.m_pixelShader->getShaderType() == ShaderStage::Pixel;

    D3D12BindingLayout* layout = _desc.m_bindingLayout ? static_cast<D3D12BindingLayout*>(_desc.m_bindingLayout.getObject()) : nullptr;
    D3D12VertexShader* vertexShader = _desc.m_vertexShader && hasValidVertexShader ? static_cast<D3D12VertexShader*>(_desc.m_vertexShader.getObject()) : nullptr;
    D3D12PixelShader* pixelShader = _desc.m_pixelShader && hasValidPixelShader ? static_cast<D3D12PixelShader*>(_desc.m_pixelShader.getObject()) : nullptr;

    EGO_ASSERT_MESSAGE(!_desc.m_bindingLayout || layout, "BindingLayout must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(!_desc.m_vertexShader || vertexShader, "Vertex shader must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(!_desc.m_pixelShader || pixelShader, "Pixel shader must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(hasValidVertexShader, "Graphic pipeline vertex shader has invalid shader stage");
    EGO_ASSERT_MESSAGE(hasValidPixelShader, "Graphic pipeline pixel shader has invalid shader stage");

    if ((_desc.m_bindingLayout && !layout) || (_desc.m_vertexShader && !vertexShader) || (_desc.m_pixelShader && !pixelShader) || !hasValidVertexShader || !hasValidPixelShader)
    {
        return GraphicPipelineReference();
    }

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
    inputElements.reserve(_desc.m_inputLayoutDesc.m_elements.size());

    for (const InputLayoutElementDesc& element : _desc.m_inputLayoutDesc.m_elements)
    {
        D3D12_INPUT_ELEMENT_DESC inputElement = {};
        inputElement.SemanticName = element.m_semanticName ? element.m_semanticName : "TEXCOORD";
        inputElement.SemanticIndex = element.m_index;
        inputElement.Format = ToDXGIFormat(element.m_type, element.m_componentsCount);
        inputElement.InputSlot = element.m_slot;
        inputElement.AlignedByteOffset = element.m_offset;
        inputElement.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        inputElement.InstanceDataStepRate = 0;

        for (const InputLayoutBindingDesc& binding : _desc.m_inputLayoutDesc.m_bindings)
        {
            if (binding.m_slot == element.m_slot)
            {
                inputElement.InputSlotClass =
                    binding.m_type == InputLayoutBindingType::InstanceBinding ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                inputElement.InstanceDataStepRate = binding.m_type == InputLayoutBindingType::InstanceBinding ? binding.m_instanceStepRate : 0;
                break;
            }
        }

        inputElements.push_back(inputElement);
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = layout ? layout->getRootSignature() : nullptr;
    psoDesc.VS = vertexShader ? vertexShader->getD3D12ByteCode() : D3D12_SHADER_BYTECODE{};
    psoDesc.PS = pixelShader ? pixelShader->getD3D12ByteCode() : D3D12_SHADER_BYTECODE{};
    psoDesc.BlendState.AlphaToCoverageEnable = _desc.m_blendStateDesc.m_alphaToCoverageEnable;
    psoDesc.BlendState.IndependentBlendEnable = _desc.m_blendStateDesc.m_renderTargets.size() > 1;

    for (D3D12_RENDER_TARGET_BLEND_DESC& renderTargetBlend : psoDesc.BlendState.RenderTarget)
    {
        renderTargetBlend.BlendEnable = FALSE;
        renderTargetBlend.LogicOpEnable = FALSE;
        renderTargetBlend.SrcBlend = D3D12_BLEND_ONE;
        renderTargetBlend.DestBlend = D3D12_BLEND_ZERO;
        renderTargetBlend.BlendOp = D3D12_BLEND_OP_ADD;
        renderTargetBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
        renderTargetBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
        renderTargetBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        renderTargetBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
        renderTargetBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }

    const size_t blendTargetCount = (std::min<size_t>)(_desc.m_blendStateDesc.m_renderTargets.size(), D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);
    for (size_t renderTargetIndex = 0; renderTargetIndex < blendTargetCount; ++renderTargetIndex)
    {
        const RenderTargetBlendDesc& blendDesc = _desc.m_blendStateDesc.m_renderTargets[renderTargetIndex];
        D3D12_RENDER_TARGET_BLEND_DESC& targetDesc = psoDesc.BlendState.RenderTarget[renderTargetIndex];
        targetDesc.BlendEnable = blendDesc.m_blendEnable;
        targetDesc.SrcBlend = ToD3D12Blend(blendDesc.m_srcColorFactor);
        targetDesc.DestBlend = ToD3D12Blend(blendDesc.m_dstColorFactor);
        targetDesc.BlendOp = ToD3D12BlendOperation(blendDesc.m_colorOperation);
        targetDesc.SrcBlendAlpha = ToD3D12Blend(blendDesc.m_srcAlphaFactor);
        targetDesc.DestBlendAlpha = ToD3D12Blend(blendDesc.m_dstAlphaFactor);
        targetDesc.BlendOpAlpha = ToD3D12BlendOperation(blendDesc.m_alphaOperation);
        targetDesc.RenderTargetWriteMask = blendDesc.m_colorWriteMask;
    }

    const UINT sampleCount = static_cast<UINT>((std::max)(_desc.m_multisampleCount, 1));

    psoDesc.SampleMask = 0xffffffffu;
    psoDesc.RasterizerState.FillMode = ToD3D12FillMode(_desc.m_rasterizationStateDesc.m_fillMode);
    psoDesc.RasterizerState.CullMode = ToD3D12CullMode(_desc.m_rasterizationStateDesc.m_cullMode);
    psoDesc.RasterizerState.FrontCounterClockwise = _desc.m_rasterizationStateDesc.m_frontCounterClockwise;
    psoDesc.RasterizerState.DepthBias = _desc.m_rasterizationStateDesc.m_depthBiasEnable ? _desc.m_rasterizationStateDesc.m_depthBias : 0;
    psoDesc.RasterizerState.DepthBiasClamp = _desc.m_rasterizationStateDesc.m_depthBiasClamp;
    psoDesc.RasterizerState.SlopeScaledDepthBias = _desc.m_rasterizationStateDesc.m_depthBiasSlopeScale;
    psoDesc.RasterizerState.DepthClipEnable = _desc.m_rasterizationStateDesc.m_depthClip;
    psoDesc.RasterizerState.MultisampleEnable = sampleCount > 1;
    psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
    psoDesc.RasterizerState.ForcedSampleCount = 0;
    psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    const bool hasDepth = _desc.m_depthFormat != GraphicResourceFormat::Undefined;
    psoDesc.DepthStencilState.DepthEnable = hasDepth && _desc.m_depthStencilStateDesc.m_depthTestEnable;
    psoDesc.DepthStencilState.DepthWriteMask = hasDepth && _desc.m_depthStencilStateDesc.m_depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    psoDesc.DepthStencilState.DepthFunc = ToD3D12ComparisonFunc(_desc.m_depthStencilStateDesc.m_depthCompareOperation);
    psoDesc.DepthStencilState.StencilEnable = hasDepth && _desc.m_depthStencilStateDesc.m_stencilEnable;
    psoDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    psoDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    psoDesc.DepthStencilState.BackFace = psoDesc.DepthStencilState.FrontFace;

    psoDesc.InputLayout.NumElements = static_cast<UINT>(inputElements.size());
    psoDesc.InputLayout.pInputElementDescs = inputElements.empty() ? nullptr : inputElements.data();
    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    psoDesc.PrimitiveTopologyType = ToD3D12PrimitiveTopologyType(_desc.m_topology);

    const size_t colorFormatCount = (std::min<size_t>)(_desc.m_colorFormats.size(), D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);
    psoDesc.NumRenderTargets = static_cast<UINT>(colorFormatCount);
    for (size_t i = 0; i < colorFormatCount; ++i)
    {
        psoDesc.RTVFormats[i] = ToDXGIFormat(_desc.m_colorFormats[i]);
    }

    psoDesc.DSVFormat = ToDXGIFormat(_desc.m_depthFormat);
    psoDesc.SampleDesc.Count = sampleCount;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.NodeMask = 0;
    psoDesc.CachedPSO = {};
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
    if (FAILED(getD3D12Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState))))
    {
        return GraphicPipelineReference();
    }

    return GraphicPipelineReference(new D3D12GraphicPipeline(_desc, std::move(pipelineState), layout));
}

ego::gpu::ComputePipelineReference ego::gpu::d3d12::D3D12GraphicDevice::createComputePipeline(const ComputePipelineDesc& _desc)
{
    const bool hasValidComputeShader = !_desc.m_computeShader || _desc.m_computeShader->getShaderType() == ShaderStage::Compute;

    D3D12BindingLayout* layout = _desc.m_bindingLayout ? static_cast<D3D12BindingLayout*>(_desc.m_bindingLayout.getObject()) : nullptr;
    D3D12ComputeShader* computeShader = _desc.m_computeShader && hasValidComputeShader ? static_cast<D3D12ComputeShader*>(_desc.m_computeShader.getObject()) : nullptr;

    EGO_ASSERT_MESSAGE(!_desc.m_bindingLayout || layout, "BindingLayout must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(!_desc.m_computeShader || computeShader, "Compute shader must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(hasValidComputeShader, "Compute pipeline shader has invalid shader stage");

    if ((_desc.m_bindingLayout && !layout) || (_desc.m_computeShader && !computeShader) || !hasValidComputeShader)
    {
        return ComputePipelineReference();
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = layout ? layout->getRootSignature() : nullptr;
    psoDesc.CS = computeShader ? computeShader->getD3D12ByteCode() : D3D12_SHADER_BYTECODE{};
    psoDesc.NodeMask = 0;
    psoDesc.CachedPSO = {};
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
    if (FAILED(getD3D12Device()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState))))
    {
        return ComputePipelineReference();
    }

    return ComputePipelineReference(new D3D12ComputePipeline(_desc, std::move(pipelineState), layout));
}

ego::gpu::RayTracingPipelineReference ego::gpu::d3d12::D3D12GraphicDevice::createRayTracingPipeline(const RayTracingPipelineDesc& _desc)
{
    if (!getD3D12Device() || !m_deviceContext.getCapabilities().m_supportsRayTracing)
    {
        return RayTracingPipelineReference();
    }

    D3D12BindingLayout* layout = _desc.m_bindingLayout ? static_cast<D3D12BindingLayout*>(_desc.m_bindingLayout.getObject()) : nullptr;
    D3D12RayGenerationShader* rayGenerationShader = _desc.m_rayGenerationShader ? static_cast<D3D12RayGenerationShader*>(_desc.m_rayGenerationShader.getObject()) : nullptr;
    D3D12MissShader* missShader = _desc.m_missShader ? static_cast<D3D12MissShader*>(_desc.m_missShader.getObject()) : nullptr;

    struct HitGroupShaders final
    {
        RayTracingHitGroupType m_type = RayTracingHitGroupType::Triangles;
        D3D12ClosestHitShader* m_closestHitShader = nullptr;
        D3D12AnyHitShader* m_anyHitShader = nullptr;
        D3D12IntersectionShader* m_intersectionShader = nullptr;
    };

    std::vector<HitGroupShaders> hitGroupShaders;
    hitGroupShaders.reserve(_desc.m_hitGroups.size());
    for (const RayTracingHitGroupDesc& hitGroupDesc : _desc.m_hitGroups)
    {
        HitGroupShaders shaders;
        shaders.m_type = hitGroupDesc.m_type;
        shaders.m_closestHitShader = hitGroupDesc.m_closestHitShader ? static_cast<D3D12ClosestHitShader*>(hitGroupDesc.m_closestHitShader.getObject()) : nullptr;
        shaders.m_anyHitShader = hitGroupDesc.m_anyHitShader ? static_cast<D3D12AnyHitShader*>(hitGroupDesc.m_anyHitShader.getObject()) : nullptr;
        shaders.m_intersectionShader = hitGroupDesc.m_intersectionShader ? static_cast<D3D12IntersectionShader*>(hitGroupDesc.m_intersectionShader.getObject()) : nullptr;

        if (!shaders.m_closestHitShader && !shaders.m_anyHitShader && !shaders.m_intersectionShader)
        {
            return RayTracingPipelineReference();
        }

        if (shaders.m_type == RayTracingHitGroupType::Triangles && shaders.m_intersectionShader)
        {
            return RayTracingPipelineReference();
        }

        if (shaders.m_type == RayTracingHitGroupType::ProceduralPrimitive && !shaders.m_intersectionShader)
        {
            return RayTracingPipelineReference();
        }

        hitGroupShaders.push_back(shaders);
    }

    EGO_ASSERT_MESSAGE(layout, "Ray tracing pipeline requires a D3D12 binding layout");
    EGO_ASSERT_MESSAGE(rayGenerationShader, "Ray tracing pipeline has invalid ray generation shader");
    EGO_ASSERT_MESSAGE(missShader, "Ray tracing pipeline has invalid miss shader");
    EGO_ASSERT_MESSAGE(!hitGroupShaders.empty(), "Ray tracing pipeline has no hit groups");

    if (!layout || !rayGenerationShader || !missShader || hitGroupShaders.empty())
    {
        return RayTracingPipelineReference();
    }

    D3D12_EXPORT_DESC rayGenerationExport = {};
    rayGenerationExport.Name = RayGenerationEntryPointName;
    rayGenerationExport.ExportToRename = nullptr;
    rayGenerationExport.Flags = D3D12_EXPORT_FLAG_NONE;

    D3D12_EXPORT_DESC missExport = {};
    missExport.Name = MissEntryPointName;
    missExport.ExportToRename = nullptr;
    missExport.Flags = D3D12_EXPORT_FLAG_NONE;

    D3D12_DXIL_LIBRARY_DESC rayGenerationLibraryDesc = {};
    rayGenerationLibraryDesc.DXILLibrary = rayGenerationShader->getD3D12ByteCode();
    rayGenerationLibraryDesc.NumExports = 1;
    rayGenerationLibraryDesc.pExports = &rayGenerationExport;

    D3D12_DXIL_LIBRARY_DESC missLibraryDesc = {};
    missLibraryDesc.DXILLibrary = missShader->getD3D12ByteCode();
    missLibraryDesc.NumExports = 1;
    missLibraryDesc.pExports = &missExport;

    std::vector<std::wstring> closestHitExportNames;
    std::vector<std::wstring> anyHitExportNames;
    std::vector<std::wstring> intersectionExportNames;
    std::vector<std::wstring> hitGroupExportNames;

    closestHitExportNames.resize(hitGroupShaders.size());
    anyHitExportNames.resize(hitGroupShaders.size());
    intersectionExportNames.resize(hitGroupShaders.size());
    hitGroupExportNames.reserve(hitGroupShaders.size());

    size_t hitGroupShaderCount = 0;
    for (size_t shaderIndex = 0; shaderIndex < hitGroupShaders.size(); ++shaderIndex)
    {
        if (hitGroupShaders[shaderIndex].m_closestHitShader)
        {
            closestHitExportNames[shaderIndex] = MakeIndexedExportName(ClosestHitEntryPointName, shaderIndex);
            ++hitGroupShaderCount;
        }

        if (hitGroupShaders[shaderIndex].m_anyHitShader)
        {
            anyHitExportNames[shaderIndex] = MakeIndexedExportName(AnyHitEntryPointName, shaderIndex);
            ++hitGroupShaderCount;
        }

        if (hitGroupShaders[shaderIndex].m_intersectionShader)
        {
            intersectionExportNames[shaderIndex] = MakeIndexedExportName(IntersectionEntryPointName, shaderIndex);
            ++hitGroupShaderCount;
        }

        hitGroupExportNames.push_back(MakeIndexedExportName(HitGroupExportName, shaderIndex));
    }

    std::vector<D3D12_EXPORT_DESC> hitGroupShaderExports;
    std::vector<D3D12_DXIL_LIBRARY_DESC> hitGroupShaderLibraryDescs;
    std::vector<D3D12_HIT_GROUP_DESC> hitGroupDescs;

    hitGroupShaderExports.reserve(hitGroupShaderCount);
    hitGroupShaderLibraryDescs.reserve(hitGroupShaderCount);
    hitGroupDescs.reserve(hitGroupShaders.size());

    for (size_t shaderIndex = 0; shaderIndex < hitGroupShaders.size(); ++shaderIndex)
    {
        if (hitGroupShaders[shaderIndex].m_closestHitShader)
        {
            D3D12_EXPORT_DESC closestHitExport = {};
            closestHitExport.Name = closestHitExportNames[shaderIndex].c_str();
            closestHitExport.ExportToRename = ClosestHitEntryPointName;
            closestHitExport.Flags = D3D12_EXPORT_FLAG_NONE;
            hitGroupShaderExports.push_back(closestHitExport);

            D3D12_DXIL_LIBRARY_DESC closestHitLibraryDesc = {};
            closestHitLibraryDesc.DXILLibrary = hitGroupShaders[shaderIndex].m_closestHitShader->getD3D12ByteCode();
            closestHitLibraryDesc.NumExports = 1;
            closestHitLibraryDesc.pExports = &hitGroupShaderExports.back();
            hitGroupShaderLibraryDescs.push_back(closestHitLibraryDesc);
        }

        if (hitGroupShaders[shaderIndex].m_anyHitShader)
        {
            D3D12_EXPORT_DESC anyHitExport = {};
            anyHitExport.Name = anyHitExportNames[shaderIndex].c_str();
            anyHitExport.ExportToRename = AnyHitEntryPointName;
            anyHitExport.Flags = D3D12_EXPORT_FLAG_NONE;
            hitGroupShaderExports.push_back(anyHitExport);

            D3D12_DXIL_LIBRARY_DESC anyHitLibraryDesc = {};
            anyHitLibraryDesc.DXILLibrary = hitGroupShaders[shaderIndex].m_anyHitShader->getD3D12ByteCode();
            anyHitLibraryDesc.NumExports = 1;
            anyHitLibraryDesc.pExports = &hitGroupShaderExports.back();
            hitGroupShaderLibraryDescs.push_back(anyHitLibraryDesc);
        }

        if (hitGroupShaders[shaderIndex].m_intersectionShader)
        {
            D3D12_EXPORT_DESC intersectionExport = {};
            intersectionExport.Name = intersectionExportNames[shaderIndex].c_str();
            intersectionExport.ExportToRename = IntersectionEntryPointName;
            intersectionExport.Flags = D3D12_EXPORT_FLAG_NONE;
            hitGroupShaderExports.push_back(intersectionExport);

            D3D12_DXIL_LIBRARY_DESC intersectionLibraryDesc = {};
            intersectionLibraryDesc.DXILLibrary = hitGroupShaders[shaderIndex].m_intersectionShader->getD3D12ByteCode();
            intersectionLibraryDesc.NumExports = 1;
            intersectionLibraryDesc.pExports = &hitGroupShaderExports.back();
            hitGroupShaderLibraryDescs.push_back(intersectionLibraryDesc);
        }

        D3D12_HIT_GROUP_DESC hitGroupDesc = {};
        hitGroupDesc.HitGroupExport = hitGroupExportNames[shaderIndex].c_str();
        hitGroupDesc.Type = GetD3D12HitGroupType(hitGroupShaders[shaderIndex].m_type);
        hitGroupDesc.ClosestHitShaderImport = hitGroupShaders[shaderIndex].m_closestHitShader ? closestHitExportNames[shaderIndex].c_str() : nullptr;
        hitGroupDesc.AnyHitShaderImport = hitGroupShaders[shaderIndex].m_anyHitShader ? anyHitExportNames[shaderIndex].c_str() : nullptr;
        hitGroupDesc.IntersectionShaderImport = hitGroupShaders[shaderIndex].m_intersectionShader ? intersectionExportNames[shaderIndex].c_str() : nullptr;
        hitGroupDescs.push_back(hitGroupDesc);
    }

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
    shaderConfig.MaxPayloadSizeInBytes = _desc.m_maxPayloadSize;
    shaderConfig.MaxAttributeSizeInBytes = _desc.m_maxAttributeSize;

    std::vector<const wchar_t*> shaderConfigExports;
    shaderConfigExports.reserve(2 + hitGroupExportNames.size());
    shaderConfigExports.push_back(RayGenerationEntryPointName);
    shaderConfigExports.push_back(MissEntryPointName);

    for (const std::wstring& hitGroupExportName : hitGroupExportNames)
    {
        shaderConfigExports.push_back(hitGroupExportName.c_str());
    }

    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderConfigAssociation = {};
    shaderConfigAssociation.NumExports = static_cast<UINT>(shaderConfigExports.size());
    shaderConfigAssociation.pExports = shaderConfigExports.data();

    D3D12_GLOBAL_ROOT_SIGNATURE globalRootSignature = {};
    globalRootSignature.pGlobalRootSignature = layout->getRootSignature();

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
    pipelineConfig.MaxTraceRecursionDepth = (std::max<uint32_t>)(_desc.m_maxRecursionDepth, 1);

    std::vector<D3D12_STATE_SUBOBJECT> subobjects;
    subobjects.reserve(6 + hitGroupShaderLibraryDescs.size() + hitGroupDescs.size());

    D3D12_STATE_SUBOBJECT rayGenerationLibrarySubobject = {};
    rayGenerationLibrarySubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    rayGenerationLibrarySubobject.pDesc = &rayGenerationLibraryDesc;
    subobjects.push_back(rayGenerationLibrarySubobject);

    D3D12_STATE_SUBOBJECT missLibrarySubobject = {};
    missLibrarySubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    missLibrarySubobject.pDesc = &missLibraryDesc;
    subobjects.push_back(missLibrarySubobject);

    for (D3D12_DXIL_LIBRARY_DESC& hitGroupShaderLibraryDesc : hitGroupShaderLibraryDescs)
    {
        D3D12_STATE_SUBOBJECT hitGroupShaderLibrarySubobject = {};
        hitGroupShaderLibrarySubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
        hitGroupShaderLibrarySubobject.pDesc = &hitGroupShaderLibraryDesc;
        subobjects.push_back(hitGroupShaderLibrarySubobject);
    }

    for (D3D12_HIT_GROUP_DESC& hitGroupDesc : hitGroupDescs)
    {
        D3D12_STATE_SUBOBJECT hitGroupSubobject = {};
        hitGroupSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
        hitGroupSubobject.pDesc = &hitGroupDesc;
        subobjects.push_back(hitGroupSubobject);
    }

    D3D12_STATE_SUBOBJECT shaderConfigSubobject = {};
    shaderConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    shaderConfigSubobject.pDesc = &shaderConfig;
    subobjects.push_back(shaderConfigSubobject);
    shaderConfigAssociation.pSubobjectToAssociate = &subobjects.back();

    D3D12_STATE_SUBOBJECT shaderConfigAssociationSubobject = {};
    shaderConfigAssociationSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    shaderConfigAssociationSubobject.pDesc = &shaderConfigAssociation;
    subobjects.push_back(shaderConfigAssociationSubobject);

    D3D12_STATE_SUBOBJECT globalRootSignatureSubobject = {};
    globalRootSignatureSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    globalRootSignatureSubobject.pDesc = &globalRootSignature;
    subobjects.push_back(globalRootSignatureSubobject);

    D3D12_STATE_SUBOBJECT pipelineConfigSubobject = {};
    pipelineConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    pipelineConfigSubobject.pDesc = &pipelineConfig;
    subobjects.push_back(pipelineConfigSubobject);

    D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
    stateObjectDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    stateObjectDesc.NumSubobjects = static_cast<UINT>(subobjects.size());
    stateObjectDesc.pSubobjects = subobjects.data();

    Microsoft::WRL::ComPtr<ID3D12StateObject> stateObject;
    if (FAILED(getD3D12Device()->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&stateObject))))
    {
        return RayTracingPipelineReference();
    }

    Microsoft::WRL::ComPtr<ID3D12Resource> shaderTable;
    uint64_t shaderRecordSize = 0;
    if (!createShaderTable(stateObject.Get(), hitGroupExportNames, shaderTable, shaderRecordSize))
    {
        return RayTracingPipelineReference();
    }

    return RayTracingPipelineReference(new D3D12RayTracingPipeline(
        _desc,
        std::move(stateObject),
        std::move(shaderTable),
        shaderRecordSize,
        static_cast<uint32_t>(hitGroupExportNames.size()),
        layout));
}

ego::gpu::BindingLayoutReference ego::gpu::d3d12::D3D12GraphicDevice::createBindingLayout(const BindingLayoutDesc& _desc)
{
    std::vector<D3D12_ROOT_PARAMETER1> rootParameters;
    std::vector<D3D12BindingLayout::PushConstantInfo> pushConstants;
    pushConstants.reserve(_desc.m_pushConstants.size());

    for (const PushConstantRangeDesc& pushConstantRange : _desc.m_pushConstants)
    {
        EGO_ASSERT_MESSAGE((pushConstantRange.m_size % sizeof(uint32_t)) == 0, "D3D12 push constants must be 32-bit aligned");

        if (!pushConstantRange.m_size || (pushConstantRange.m_size % sizeof(uint32_t)) != 0)
        {
            continue;
        }

        D3D12_ROOT_PARAMETER1 rootParameter = {};
        rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rootParameter.ShaderVisibility = ToD3D12ShaderVisibility(pushConstantRange.m_stageFlag);
        rootParameter.Constants.ShaderRegister = pushConstantRange.m_offset / sizeof(uint32_t);
        rootParameter.Constants.RegisterSpace = 0;
        rootParameter.Constants.Num32BitValues = pushConstantRange.m_size / sizeof(uint32_t);

        D3D12BindingLayout::PushConstantInfo pushConstantInfo;
        pushConstantInfo.m_rootParameterIndex = static_cast<uint32_t>(rootParameters.size());
        pushConstantInfo.m_offset = pushConstantRange.m_offset;
        pushConstantInfo.m_size = pushConstantRange.m_size;
        pushConstantInfo.m_stageFlags = pushConstantRange.m_stageFlag;

        rootParameters.push_back(rootParameter);
        pushConstants.push_back(pushConstantInfo);
    }

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    rootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(rootParameters.size());
    rootSignatureDesc.Desc_1_1.pParameters = rootParameters.empty() ? nullptr : rootParameters.data();
    rootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
    rootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
    rootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    if (m_deviceContext.getCapabilities().m_supportsBindlessResources)
    {
        rootSignatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;
    }

    Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSignature;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    if (FAILED(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &serializedRootSignature, &errorBlob)))
    {
        return BindingLayoutReference();
    }

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    if (FAILED(getD3D12Device()->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(), serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(&rootSignature))))
    {
        return BindingLayoutReference();
    }

    return BindingLayoutReference(new D3D12BindingLayout(_desc, std::move(rootSignature), std::move(pushConstants)));
}

ego::gpu::FenceReference ego::gpu::d3d12::D3D12GraphicDevice::createFence(Fence::FenceValue _initialValue)
{
    if (!getD3D12Device())
    {
        return FenceReference();
    }

    Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    if (FAILED(getD3D12Device()->CreateFence(_initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
    {
        return FenceReference();
    }

    return FenceReference(new D3D12Fence(_initialValue, std::move(fence)));
}

ego::gpu::SwapChainReference ego::gpu::d3d12::D3D12GraphicDevice::createSwapChain(
    const SwapChainDesc& _swapChainDesc,
    const Window& _window,
    const CommandQueueReference& _presentationQueue)
{
    if (!m_deviceContext.getFactory() || !getD3D12Device() || !_window.getNativeHandle())
    {
        return SwapChainReference();
    }

    if (_swapChainDesc.m_bufferCount == 0)
    {
        return SwapChainReference();
    }

    const DXGI_FORMAT format = ToDXGIFormat(_swapChainDesc.m_format);
    if (format == DXGI_FORMAT_UNKNOWN)
    {
        return SwapChainReference();
    }

    if (!_presentationQueue || _presentationQueue->getCommandType() != CommandType::Graphic || !_presentationQueue->getDesc().m_supportsPresentation)
    {
        return SwapChainReference();
    }

    ID3D12CommandQueue* presentationQueue = _presentationQueue->getNativeHandle<ID3D12CommandQueue>();
    if (!presentationQueue)
    {
        return SwapChainReference();
    }

    const WindowSize& clientAreaSize = _window.getClientAreaSize();
    const uint32_t width = clientAreaSize.m_x;
    const uint32_t height = clientAreaSize.m_y;

    if (width == 0 || height == 0)
    {
        return SwapChainReference();
    }

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = format;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = _swapChainDesc.m_bufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    auto windowHandle = reinterpret_cast<HWND>(_window.getNativeHandle());
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
    if (FAILED(m_deviceContext.getFactory()->CreateSwapChainForHwnd(presentationQueue, windowHandle, &swapChainDesc, nullptr, nullptr, &swapChain)))
    {
        return SwapChainReference();
    }

    m_deviceContext.getFactory()->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER);

    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain3;
    if (FAILED(swapChain.As(&swapChain3)))
    {
        return SwapChainReference();
    }

    std::vector<Texture2DReference> targetTextures;
    targetTextures.reserve(_swapChainDesc.m_bufferCount);

    Texture2DDesc textureDesc;
    textureDesc.m_usage = static_cast<GraphicResourceUsage>(TextureUsageRenderTarget);
    textureDesc.m_size = Texture2DSize(width, height);
    textureDesc.m_arrayLayers = 1;
    textureDesc.m_mipLevels = 1;
    textureDesc.m_samples.m_count = 1;
    textureDesc.m_samples.m_quality = 0;
    textureDesc.m_format = _swapChainDesc.m_format;

    for (uint32_t bufferIndex = 0; bufferIndex < _swapChainDesc.m_bufferCount; ++bufferIndex)
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        if (FAILED(swapChain3->GetBuffer(bufferIndex, IID_PPV_ARGS(&resource))))
        {
            return SwapChainReference();
        }

        targetTextures.push_back(Texture2DReference(new D3D12Texture2D(textureDesc, std::move(resource))));
    }

    return SwapChainReference(new D3D12SwapChain(_swapChainDesc, std::move(swapChain3), std::move(targetTextures), _presentationQueue));
}

const ego::GraphicDevice::Capabilities& ego::gpu::d3d12::D3D12GraphicDevice::getCapabilities() const
{
    return m_deviceContext.getCapabilities();
}

ID3D12Device* ego::gpu::d3d12::D3D12GraphicDevice::getDevice() const
{
    return getD3D12Device();
}

IDXGIAdapter1* ego::gpu::d3d12::D3D12GraphicDevice::getAdapter() const
{
    return m_deviceContext.getAdapter();
}

ID3D12Device5* ego::gpu::d3d12::D3D12GraphicDevice::getD3D12Device() const
{
    return m_deviceContext.getDevice();
}

ego::gpu::GpuTaskReference ego::gpu::d3d12::D3D12GraphicDevice::submitImmediateCommands(
    const std::function<void(ID3D12GraphicsCommandList4*)>& _recordCommands,
    const std::vector<GraphicObjectReference>& _keepAliveObjects)
{
    return m_immediateContext.submit(_recordCommands, _keepAliveObjects);
}

bool ego::gpu::d3d12::D3D12GraphicDevice::executeImmediateCommands(const std::function<void(ID3D12GraphicsCommandList4*)>& _recordCommands)
{
    return m_immediateContext.execute(_recordCommands);
}

bool ego::gpu::d3d12::D3D12GraphicDevice::registerResourceProviders()
{
    if (m_resourceProviderPlugin)
    {
        return true;
    }

    const PluginControllerPointer pluginController = PluginControllerCore::GetInstance().getPluginController();
    EGO_CHECK_RETURN_FALSE(pluginController);

    const FileName dxcModuleName = engine::GetEngine().getPluginCatalog().getModulePath(ResourceProviderPlugin::GetPluginType(), "DXC");
    EGO_CHECK_RETURN_FALSE(dxcModuleName);

    m_resourceProviderPlugin = pluginController->loadPlugin<ResourceProviderPlugin>(dxcModuleName);
    EGO_CHECK_RETURN_FALSE(m_resourceProviderPlugin);

    m_resourceProviderPlugin->registerResourceProviders();

    return true;
}

void ego::gpu::d3d12::D3D12GraphicDevice::unregisterResourceProviders()
{
    if (m_resourceProviderPlugin)
    {
        m_resourceProviderPlugin->unregisterResourceProviders();
    }

    m_resourceProviderPlugin = nullptr;
}
