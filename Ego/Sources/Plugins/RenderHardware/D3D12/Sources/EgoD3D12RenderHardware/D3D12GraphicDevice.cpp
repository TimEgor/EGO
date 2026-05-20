#include "D3D12GraphicDevice.h"

#include <algorithm>

#include <d3dcompiler.h>

#include "EgoEngine/Platform/Window/Window.h"

#include "Objects/D3D12Buffer.h"
#include "Objects/D3D12CommandList.h"
#include "Objects/D3D12CommandQueue.h"
#include "Objects/D3D12Fence.h"
#include "Objects/D3D12Pipeline.h"
#include "Objects/D3D12Sampler.h"
#include "Objects/D3D12Shader.h"
#include "Objects/D3D12SwapChain.h"
#include "Objects/D3D12Texture.h"

namespace
{
    constexpr uint32_t BindlessResourceDescriptorCapacity = 1024;
    constexpr uint32_t BindlessSamplerDescriptorCapacity = 128;

    ego::gpu::ShaderCodePointer CompileShaderCode(
        const ego::gpu::ShaderCodePointer& _code,
        const char* _entryPoint,
        const char* _target
    )
    {
        if (!_code || !_code->getCode() || !_code->getCodeSize())
        {
            return nullptr;
        }

        Microsoft::WRL::ComPtr<ID3DBlob> compiledShader;
        Microsoft::WRL::ComPtr<ID3DBlob> errors;

        const HRESULT result = D3DCompile(
            _code->getCode(),
            _code->getCodeSize(),
            nullptr,
            nullptr,
            nullptr,
            _entryPoint,
            _target,
            D3DCOMPILE_ENABLE_STRICTNESS,
            0,
            &compiledShader,
            &errors
        );

        if (FAILED(result) || !compiledShader)
        {
            return _code;
        }

        return ego::gpu::ShaderCodePointer(
            new ego::gpu::ShaderCode(
                compiledShader->GetBufferPointer(),
                static_cast<uint32_t>(compiledShader->GetBufferSize())
            )
        );
    }

    template <typename TCommandListPointer, typename TCommandListObject>
    TCommandListPointer CreateCommandList(
        ego::gpu::d3d12::D3D12GraphicDevice* _device,
        D3D12_COMMAND_LIST_TYPE _type
    )
    {
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;

        ID3D12Device* device = _device ? _device->getDevice() : nullptr;
        if (!device)
        {
            return TCommandListPointer();
        }

        if (FAILED(device->CreateCommandAllocator(_type, IID_PPV_ARGS(&allocator))))
        {
            return TCommandListPointer();
        }

        if (FAILED(device->CreateCommandList(0, _type, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList))))
        {
            return TCommandListPointer();
        }

        commandList->Close();
        return TCommandListPointer(new TCommandListObject(_device, std::move(allocator), std::move(commandList)));
    }

    D3D12_HEAP_TYPE GetHeapType(const ego::gpu::GraphicResourceDesc& _desc)
    {
        if (_desc.m_access & ego::gpu::GraphicResourceAccessCpuWrite)
        {
            return D3D12_HEAP_TYPE_UPLOAD;
        }

        if (_desc.m_access & ego::gpu::GraphicResourceAccessCpuRead)
        {
            return D3D12_HEAP_TYPE_READBACK;
        }

        return D3D12_HEAP_TYPE_DEFAULT;
    }

    D3D12_RESOURCE_STATES GetInitialBufferState(
        const ego::gpu::BufferDesc& _desc,
        const ego::gpu::InitialGraphicResourceData& _initialData
    )
    {
        const D3D12_HEAP_TYPE heapType = GetHeapType(_desc);
        if (heapType == D3D12_HEAP_TYPE_UPLOAD)
        {
            return D3D12_RESOURCE_STATE_GENERIC_READ;
        }

        if (heapType == D3D12_HEAP_TYPE_READBACK)
        {
            return D3D12_RESOURCE_STATE_COPY_DEST;
        }

        if (_initialData.isValid())
        {
            return D3D12_RESOURCE_STATE_COPY_DEST;
        }

        return D3D12_RESOURCE_STATE_COMMON;
    }

    bool CreateUploadBuffer(
        ID3D12Device* _device,
        uint64_t _size,
        Microsoft::WRL::ComPtr<ID3D12Resource>& _resource
    )
    {
        if (!_device || !_size)
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
            _device->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&_resource)
            )
        );
    }

    bool WriteToUploadBuffer(
        ID3D12Resource* _resource,
        const void* _data,
        uint64_t _dataSize,
        uint64_t _resourceSize
    )
    {
        if (!_resource || !_data || !_dataSize)
        {
            return false;
        }

        void* mappedData = nullptr;
        D3D12_RANGE readRange = {};
        if (FAILED(_resource->Map(0, &readRange, &mappedData)))
        {
            return false;
        }

        const uint64_t bytesToCopy = (std::min)(_dataSize, _resourceSize);
        std::memcpy(mappedData, _data, static_cast<size_t>(bytesToCopy));

        D3D12_RANGE writtenRange = {};
        writtenRange.Begin = 0;
        writtenRange.End = static_cast<SIZE_T>(bytesToCopy);
        _resource->Unmap(0, &writtenRange);
        return true;
    }

    template <typename TRecordCommands>
    bool ExecuteImmediateCommands(ID3D12Device* _device, TRecordCommands _recordCommands)
    {
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        if (FAILED(_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue))) ||
            FAILED(_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))) ||
            FAILED(
                _device->CreateCommandList(
                    0,
                    D3D12_COMMAND_LIST_TYPE_DIRECT,
                    allocator.Get(),
                    nullptr,
                    IID_PPV_ARGS(&commandList)
                )
            ) ||
            FAILED(_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
        {
            return false;
        }

        _recordCommands(commandList.Get());

        if (FAILED(commandList->Close()))
        {
            return false;
        }

        ID3D12CommandList* commandLists[] = {commandList.Get()};
        queue->ExecuteCommandLists(1, commandLists);

        constexpr uint64_t waitValue = 1;
        if (FAILED(queue->Signal(fence.Get(), waitValue)))
        {
            return false;
        }

        if (fence->GetCompletedValue() >= waitValue)
        {
            return true;
        }

        HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!eventHandle)
        {
            return false;
        }

        const HRESULT setEventResult = fence->SetEventOnCompletion(waitValue, eventHandle);
        if (SUCCEEDED(setEventResult))
        {
            WaitForSingleObject(eventHandle, INFINITE);
        }

        CloseHandle(eventHandle);
        return SUCCEEDED(setEventResult);
    }

    bool UploadBufferToDefaultHeap(
        ID3D12Device* _device,
        ID3D12Resource* _dstResource,
        uint64_t _dstSize,
        const ego::gpu::InitialGraphicResourceData& _initialData
    )
    {
        const uint64_t uploadSize = (std::min<uint64_t>)(_initialData.m_dataSize, _dstSize);
        Microsoft::WRL::ComPtr<ID3D12Resource> uploadResource;

        if (!CreateUploadBuffer(_device, uploadSize, uploadResource) ||
            !WriteToUploadBuffer(uploadResource.Get(), _initialData.m_data, uploadSize, uploadSize))
        {
            return false;
        }

        return ExecuteImmediateCommands(
            _device,
            [&](ID3D12GraphicsCommandList* _commandList)
            {
                _commandList->CopyBufferRegion(_dstResource, 0, uploadResource.Get(), 0, uploadSize);

                D3D12_RESOURCE_BARRIER barrier = {};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = _dstResource;
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                _commandList->ResourceBarrier(1, &barrier);
            }
        );
    }

    bool UploadTexture2DToDefaultHeap(
        ID3D12Device* _device,
        ID3D12Resource* _dstResource,
        const D3D12_RESOURCE_DESC& _dstDesc,
        const ego::gpu::InitialGraphicResourceData& _initialData
    )
    {
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
        UINT numRows = 0;
        UINT64 rowSizeInBytes = 0;
        UINT64 totalBytes = 0;
        _device->GetCopyableFootprints(
            &_dstDesc,
            0,
            1,
            0,
            &footprint,
            &numRows,
            &rowSizeInBytes,
            &totalBytes
        );

        Microsoft::WRL::ComPtr<ID3D12Resource> uploadResource;
        if (!CreateUploadBuffer(_device, totalBytes, uploadResource))
        {
            return false;
        }

        void* mappedData = nullptr;
        D3D12_RANGE readRange = {};
        if (FAILED(uploadResource->Map(0, &readRange, &mappedData)))
        {
            return false;
        }

        const uint8_t* srcBytes = static_cast<const uint8_t*>(_initialData.m_data);
        uint8_t* dstBytes = static_cast<uint8_t*>(mappedData) + footprint.Offset;
        const uint64_t srcRowPitch = _initialData.m_rowPitch ? _initialData.m_rowPitch : rowSizeInBytes;
        const uint64_t rowsToCopy = (std::min<uint64_t>)(numRows, _initialData.m_dataSize / srcRowPitch);
        const uint64_t rowBytesToCopy = (std::min<uint64_t>)(rowSizeInBytes, srcRowPitch);

        for (uint64_t row = 0; row < rowsToCopy; ++row)
        {
            std::memcpy(
                dstBytes + row * footprint.Footprint.RowPitch,
                srcBytes + row * srcRowPitch,
                static_cast<size_t>(rowBytesToCopy)
            );
        }

        D3D12_RANGE writtenRange = {};
        writtenRange.Begin = static_cast<SIZE_T>(footprint.Offset);
        writtenRange.End = static_cast<SIZE_T>(footprint.Offset + totalBytes);
        uploadResource->Unmap(0, &writtenRange);

        return ExecuteImmediateCommands(
            _device,
            [&](ID3D12GraphicsCommandList* _commandList)
            {
                D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
                srcLocation.pResource = uploadResource.Get();
                srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                srcLocation.PlacedFootprint = footprint;

                D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
                dstLocation.pResource = _dstResource;
                dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dstLocation.SubresourceIndex = 0;

                _commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

                D3D12_RESOURCE_BARRIER barrier = {};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = _dstResource;
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                _commandList->ResourceBarrier(1, &barrier);
            }
        );
    }

    uint64_t ResolveViewSize(uint64_t _resourceSize, uint64_t _offset, uint64_t _explicitSize)
    {
        if (_explicitSize)
        {
            return _explicitSize;
        }

        return _offset < _resourceSize ? _resourceSize - _offset : 0;
    }

    uint32_t GetFormatStride(ego::gpu::GraphicResourceFormat _format)
    {
        using ego::gpu::GraphicResourceFormat;

        switch (_format)
        {
        case GraphicResourceFormat::R8UNorm:
        case GraphicResourceFormat::R8SNorm:
        case GraphicResourceFormat::R8UInt:
        case GraphicResourceFormat::R8SInt:
            return 1;
        case GraphicResourceFormat::R8G8UNorm:
        case GraphicResourceFormat::R8G8SNorm:
        case GraphicResourceFormat::R8G8UInt:
        case GraphicResourceFormat::R8G8SInt:
        case GraphicResourceFormat::R16UNorm:
        case GraphicResourceFormat::R16SNorm:
        case GraphicResourceFormat::R16UInt:
        case GraphicResourceFormat::R16SInt:
        case GraphicResourceFormat::R16SFloat:
            return 2;
        case GraphicResourceFormat::R8G8B8A8UNorm:
        case GraphicResourceFormat::R8G8B8A8SNorm:
        case GraphicResourceFormat::R8G8B8A8UInt:
        case GraphicResourceFormat::R8G8B8A8SInt:
        case GraphicResourceFormat::B8G8R8A8UNorm:
        case GraphicResourceFormat::R16G16UNorm:
        case GraphicResourceFormat::R16G16SNorm:
        case GraphicResourceFormat::R16G16UInt:
        case GraphicResourceFormat::R16G16SInt:
        case GraphicResourceFormat::R16G16SFloat:
        case GraphicResourceFormat::R32UInt:
        case GraphicResourceFormat::R32SInt:
        case GraphicResourceFormat::R32SFloat:
        case GraphicResourceFormat::D24UNormS8UInt:
        case GraphicResourceFormat::D32SFloat:
            return 4;
        case GraphicResourceFormat::R16G16B16A16UNorm:
        case GraphicResourceFormat::R16G16B16A16SNorm:
        case GraphicResourceFormat::R16G16B16A16UInt:
        case GraphicResourceFormat::R16G16B16A16SInt:
        case GraphicResourceFormat::R16G16B16A16SFloat:
        case GraphicResourceFormat::R32G32UInt:
        case GraphicResourceFormat::R32G32SInt:
        case GraphicResourceFormat::R32G32SFloat:
        case GraphicResourceFormat::D32SFloatS8UInt:
            return 8;
        case GraphicResourceFormat::R32G32B32UInt:
        case GraphicResourceFormat::R32G32B32SInt:
        case GraphicResourceFormat::R32G32B32SFloat:
            return 12;
        case GraphicResourceFormat::R32G32B32A32UInt:
        case GraphicResourceFormat::R32G32B32A32SInt:
        case GraphicResourceFormat::R32G32B32A32SFloat:
            return 16;
        default:
            break;
        }

        return 0;
    }

    D3D12_TEXTURE_ADDRESS_MODE ToD3D12AddressMode(ego::gpu::SamplerAddressMode _mode)
    {
        switch (_mode)
        {
        case ego::gpu::SamplerAddressMode::Repeat:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case ego::gpu::SamplerAddressMode::MirroredRepeat:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case ego::gpu::SamplerAddressMode::ClampToBorder:
            return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case ego::gpu::SamplerAddressMode::ClampToEdge:
        default:
            break;
        }

        return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    }

    D3D12_FILTER ToD3D12Filter(const ego::gpu::SamplerDesc& _desc)
    {
        if (_desc.m_minFilter == ego::gpu::SamplerFilter::Anisotropic ||
            _desc.m_magFilter == ego::gpu::SamplerFilter::Anisotropic ||
            _desc.m_mipFilter == ego::gpu::SamplerFilter::Anisotropic)
        {
            return _desc.m_enableComparison ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
        }

        const bool minLinear = _desc.m_minFilter == ego::gpu::SamplerFilter::Linear;
        const bool magLinear = _desc.m_magFilter == ego::gpu::SamplerFilter::Linear;
        const bool mipLinear = _desc.m_mipFilter == ego::gpu::SamplerFilter::Linear;

        if (_desc.m_enableComparison)
        {
            return D3D12_ENCODE_BASIC_FILTER(
                minLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
                magLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
                mipLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
                D3D12_FILTER_REDUCTION_TYPE_COMPARISON
            );
        }

        return D3D12_ENCODE_BASIC_FILTER(
            minLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
            magLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
            mipLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
            D3D12_FILTER_REDUCTION_TYPE_STANDARD
        );
    }

    D3D12_RESOURCE_FLAGS GetBufferResourceFlags(const ego::gpu::BufferDesc& _desc)
    {
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
        if (_desc.m_usage & ego::gpu::GraphicResourceUsageAllowUnorderedAccess)
        {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        return flags;
    }

    bool FillBufferViewDesc(
        const ego::gpu::d3d12::D3D12Buffer* _buffer,
        const ego::gpu::BufferViewDesc& _desc,
        D3D12_BUFFER_SRV& _outBufferDesc,
        DXGI_FORMAT& _outFormat
    )
    {
        const uint64_t size = ResolveViewSize(_buffer->getDesc().m_size, _desc.m_offset, _desc.m_size);
        const uint32_t stride = _desc.m_format == ego::gpu::GraphicResourceFormat::Undefined ?
                                    (_desc.m_stride ? _desc.m_stride : _buffer->getDesc().m_stride) :
                                    GetFormatStride(_desc.m_format);
        if (!stride || !size)
        {
            return false;
        }

        _outFormat = _desc.m_format == ego::gpu::GraphicResourceFormat::Undefined ?
                         DXGI_FORMAT_UNKNOWN :
                         ego::gpu::d3d12::ToDXGIFormat(_desc.m_format);
        _outBufferDesc.FirstElement = static_cast<UINT64>(_desc.m_offset / stride);
        _outBufferDesc.NumElements = static_cast<UINT>(size / stride);
        _outBufferDesc.StructureByteStride = _outFormat == DXGI_FORMAT_UNKNOWN ? stride : 0;
        _outBufferDesc.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        return true;
    }
}

bool ego::gpu::d3d12::D3D12GraphicDevice::init(const GraphicDeviceInitParams& _params)
{
    EGO_CHECK_INITIALIZATION(initializeFactory(_params));
    EGO_CHECK_INITIALIZATION(initializeAdapter());
    EGO_CHECK_INITIALIZATION(initializeDevice());
    EGO_CHECK_INITIALIZATION(initializeDescriptorAllocators());

    return true;
}

void ego::gpu::d3d12::D3D12GraphicDevice::release()
{}

void* ego::gpu::d3d12::D3D12GraphicDevice::getNativeHandle() const
{
    return m_device.Get();
}

void ego::gpu::d3d12::D3D12GraphicDevice::setName(const char* _name)
{
    SetD3D12ObjectName(m_device.Get(), _name);
}

ego::gpu::CommandQueuePointer ego::gpu::d3d12::D3D12GraphicDevice::createCommandQueue(const CommandQueueDesc& _desc)
{
    if (!m_device)
    {
        return CommandQueuePointer();
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = ToD3D12CommandListType(_desc.m_type);
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;
    if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue))))
    {
        return CommandQueuePointer();
    }

    return CommandQueuePointer(new D3D12CommandQueue(this, _desc, std::move(queue)));
}

ego::gpu::GraphicCommandListPointer ego::gpu::d3d12::D3D12GraphicDevice::createGraphicCommandList()
{
    return CreateCommandList<GraphicCommandListPointer, D3D12GraphicCommandList>(
        this,
        D3D12_COMMAND_LIST_TYPE_DIRECT
    );
}

ego::gpu::ComputeCommandListPointer ego::gpu::d3d12::D3D12GraphicDevice::createComputeCommandList()
{
    return CreateCommandList<ComputeCommandListPointer, D3D12ComputeCommandList>(
        this,
        D3D12_COMMAND_LIST_TYPE_COMPUTE
    );
}

ego::gpu::CopyCommandListPointer ego::gpu::d3d12::D3D12GraphicDevice::createCopyCommandList()
{
    return CreateCommandList<CopyCommandListPointer, D3D12CopyCommandList>(
        this,
        D3D12_COMMAND_LIST_TYPE_COPY
    );
}

ego::gpu::BufferPointer ego::gpu::d3d12::D3D12GraphicDevice::createBuffer(
    const BufferDesc& _desc,
    const InitialGraphicResourceData& _initialData
)
{
    if (!m_device || !_desc.m_size)
    {
        return BufferPointer();
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
        m_device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            GetInitialBufferState(_desc, _initialData),
            nullptr,
            IID_PPV_ARGS(&resource)
        )
    ))
    {
        return BufferPointer();
    }

    if (_initialData.isValid())
    {
        const D3D12_HEAP_TYPE heapType = GetHeapType(_desc);
        bool uploaded = false;

        if (heapType == D3D12_HEAP_TYPE_UPLOAD)
        {
            uploaded = WriteToUploadBuffer(resource.Get(), _initialData.m_data, _initialData.m_dataSize, _desc.m_size);
        }
        else if (heapType == D3D12_HEAP_TYPE_DEFAULT)
        {
            uploaded = UploadBufferToDefaultHeap(m_device.Get(), resource.Get(), _desc.m_size, _initialData);
        }
        else
        {
            EGO_ASSERT_FAIL_MESSAGE("Initial data upload to readback heap is not supported");
        }

        if (!uploaded)
        {
            return BufferPointer();
        }
    }

    return BufferPointer(new D3D12Buffer(_desc, std::move(resource)));
}

ego::gpu::Texture2DPointer ego::gpu::d3d12::D3D12GraphicDevice::createTexture2D(
    const Texture2DDesc& _desc,
    const InitialGraphicResourceData& _initialData
)
{
    if (!m_device)
    {
        return Texture2DPointer();
    }

    EGO_ASSERT_MESSAGE(
        !(_desc.m_access & (GraphicResourceAccessCpuRead | GraphicResourceAccessCpuWrite)),
        "D3D12 textures are created in GPU-local memory by this backend"
    );
    EGO_ASSERT_MESSAGE(
        !_initialData.isValid() || _desc.m_samples.m_count <= 1,
        "Initial data upload for multisampled textures is not supported"
    );

    const DXGI_FORMAT format = ToDXGIFormat(_desc.m_format);
    if (format == DXGI_FORMAT_UNKNOWN || (_initialData.isValid() && _desc.m_samples.m_count > 1))
    {
        return Texture2DPointer();
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
        m_device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            _initialData.isValid() ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON,
            clearValuePtr,
            IID_PPV_ARGS(&resource)
        )
    ))
    {
        return Texture2DPointer();
    }

    if (_initialData.isValid() &&
        !UploadTexture2DToDefaultHeap(m_device.Get(), resource.Get(), resourceDesc, _initialData))
    {
        return Texture2DPointer();
    }

    return Texture2DPointer(new D3D12Texture2D(_desc, std::move(resource)));
}

ego::gpu::SamplerPointer ego::gpu::d3d12::D3D12GraphicDevice::createSampler(const SamplerDesc& _desc)
{
    const D3D12DescriptorIndex descriptorIndex = m_samplerDescriptorAllocator->allocate();
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
    samplerDesc.ComparisonFunc = _desc.m_enableComparison ?
                                     ToD3D12ComparisonFunc(_desc.m_comparisonOperation) :
                                     D3D12_COMPARISON_FUNC_ALWAYS;
    samplerDesc.MinLOD = _desc.m_minLod;
    samplerDesc.MaxLOD = _desc.m_maxLod;

    const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_samplerDescriptorAllocator->getCpuHandle(descriptorIndex);
    m_device->CreateSampler(&samplerDesc, cpuHandle);

    return SamplerPointer(new D3D12Sampler(_desc, descriptorIndex, m_samplerDescriptorAllocator));
}

ego::gpu::BufferViewPointer ego::gpu::d3d12::D3D12GraphicDevice::createBufferView(
    const BufferPointer& _buffer,
    const BufferViewDesc& _desc
)
{
    D3D12Buffer* buffer = static_cast<D3D12Buffer*>(_buffer.get());
    EGO_ASSERT_MESSAGE(buffer, "Buffer must be created by D3D12 device");
    if (!buffer)
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
        viewDesc.SizeInBytes = static_cast<UINT>(AlignTo(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));
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

    return BufferViewPointer(
        new D3D12BufferView(_buffer, _desc, descriptorIndex, allocator)
    );
}

ego::gpu::TextureViewPointer ego::gpu::d3d12::D3D12GraphicDevice::createTextureView(
    const TexturePointer& _texture,
    const TextureViewDesc& _desc
)
{
    D3D12Resource* texture = static_cast<D3D12Texture2D*>(_texture.get());
    EGO_ASSERT_MESSAGE(texture, "Texture must be created by D3D12 device");
    if (!texture)
    {
        return TextureViewPointer();
    }

    D3D12DescriptorAllocatorPointer allocator = nullptr;
    D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    switch (_desc.m_type)
    {
    case GraphicResourceViewType::RenderTarget:
        allocator = m_rtvDescriptorAllocator;
        heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        break;
    case GraphicResourceViewType::DepthStencil:
        allocator = m_dsvDescriptorAllocator;
        heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        break;
    case GraphicResourceViewType::ShaderResource:
    case GraphicResourceViewType::UnorderedAccess:
        allocator = m_viewDescriptorAllocator;
        heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
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
    const DXGI_FORMAT format = _desc.m_format == GraphicResourceFormat::Undefined ?
                                   resourceDesc.Format :
                                   ToDXGIFormat(_desc.m_format);
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

    return TextureViewPointer(new D3D12TextureView(_texture, _desc, descriptorIndex, allocator));
}

ego::gpu::VertexShaderPointer ego::gpu::d3d12::D3D12GraphicDevice::createVertexShader(const ShaderCodePointer& _code)
{
    if (!_code || !_code->getCode() || !_code->getCodeSize())
    {
        return VertexShaderPointer();
    }

    return VertexShaderPointer(new D3D12VertexShader(CompileShaderCode(_code, "VSMain", "vs_5_0")));
}

ego::gpu::PixelShaderPointer ego::gpu::d3d12::D3D12GraphicDevice::createPixelShader(const ShaderCodePointer& _code)
{
    if (!_code || !_code->getCode() || !_code->getCodeSize())
    {
        return PixelShaderPointer();
    }

    return PixelShaderPointer(new D3D12PixelShader(CompileShaderCode(_code, "PSMain", "ps_5_0")));
}

ego::gpu::ComputeShaderPointer ego::gpu::d3d12::D3D12GraphicDevice::createComputeShader(const ShaderCodePointer& _code)
{
    if (!_code || !_code->getCode() || !_code->getCodeSize())
    {
        return ComputeShaderPointer();
    }

    return ComputeShaderPointer(new D3D12ComputeShader(CompileShaderCode(_code, "CSMain", "cs_5_0")));
}

ego::gpu::GraphicPipelinePointer ego::gpu::d3d12::D3D12GraphicDevice::createGraphicPipeline(
    const GraphicPipelineDesc& _desc
)
{
    const bool hasValidVertexShader =
        !_desc.m_vertexShader || _desc.m_vertexShader->getShaderType() == ShaderStage::Vertex;
    const bool hasValidPixelShader =
        !_desc.m_pixelShader || _desc.m_pixelShader->getShaderType() == ShaderStage::Pixel;

    D3D12BindingLayout* layout = _desc.m_bindingLayout ?
                                     static_cast<D3D12BindingLayout*>(_desc.m_bindingLayout.get()) :
                                     nullptr;
    D3D12VertexShader* vertexShader = _desc.m_vertexShader && hasValidVertexShader ?
                                          static_cast<D3D12VertexShader*>(_desc.m_vertexShader.get()) :
                                          nullptr;
    D3D12PixelShader* pixelShader = _desc.m_pixelShader && hasValidPixelShader ?
                                        static_cast<D3D12PixelShader*>(_desc.m_pixelShader.get()) :
                                        nullptr;

    EGO_ASSERT_MESSAGE(!_desc.m_bindingLayout || layout, "BindingLayout must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(!_desc.m_vertexShader || vertexShader, "Vertex shader must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(!_desc.m_pixelShader || pixelShader, "Pixel shader must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(hasValidVertexShader, "Graphic pipeline vertex shader has invalid shader stage");
    EGO_ASSERT_MESSAGE(hasValidPixelShader, "Graphic pipeline pixel shader has invalid shader stage");

    if ((_desc.m_bindingLayout && !layout) ||
        (_desc.m_vertexShader && !vertexShader) ||
        (_desc.m_pixelShader && !pixelShader) ||
        !hasValidVertexShader ||
        !hasValidPixelShader)
    {
        return GraphicPipelinePointer();
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
                inputElement.InputSlotClass = binding.m_type == InputLayoutBindingType::InstanceBinding ?
                                                  D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA :
                                                  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                inputElement.InstanceDataStepRate = binding.m_type == InputLayoutBindingType::InstanceBinding ?
                                                        binding.m_instanceStepRate :
                                                        0;
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

    const size_t blendTargetCount = (std::min<size_t>)(
        _desc.m_blendStateDesc.m_renderTargets.size(),
        D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT
    );
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
    psoDesc.RasterizerState.DepthBias = _desc.m_rasterizationStateDesc.m_depthBiasEnable ?
                                            _desc.m_rasterizationStateDesc.m_depthBias :
                                            0;
    psoDesc.RasterizerState.DepthBiasClamp = _desc.m_rasterizationStateDesc.m_depthBiasClamp;
    psoDesc.RasterizerState.SlopeScaledDepthBias = _desc.m_rasterizationStateDesc.m_depthBiasSlopeScale;
    psoDesc.RasterizerState.DepthClipEnable = _desc.m_rasterizationStateDesc.m_depthClip;
    psoDesc.RasterizerState.MultisampleEnable = sampleCount > 1;
    psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
    psoDesc.RasterizerState.ForcedSampleCount = 0;
    psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    const bool hasDepth = _desc.m_depthFormat != GraphicResourceFormat::Undefined;
    psoDesc.DepthStencilState.DepthEnable = hasDepth && _desc.m_depthStencilStateDesc.m_depthTestEnable;
    psoDesc.DepthStencilState.DepthWriteMask = hasDepth && _desc.m_depthStencilStateDesc.m_depthWrite ?
                                                   D3D12_DEPTH_WRITE_MASK_ALL :
                                                   D3D12_DEPTH_WRITE_MASK_ZERO;
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

    const size_t colorFormatCount = (std::min<size_t>)(
        _desc.m_colorFormats.size(),
        D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT
    );
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
    if (FAILED(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState))))
    {
        return GraphicPipelinePointer();
    }

    return GraphicPipelinePointer(new D3D12GraphicPipeline(_desc, std::move(pipelineState), layout));
}

ego::gpu::ComputePipelinePointer ego::gpu::d3d12::D3D12GraphicDevice::createComputePipeline(
    const ComputePipelineDesc& _desc
)
{
    const bool hasValidComputeShader =
        !_desc.m_computeShader || _desc.m_computeShader->getShaderType() == ShaderStage::Compute;

    D3D12BindingLayout* layout = _desc.m_bindingLayout ?
                                     static_cast<D3D12BindingLayout*>(_desc.m_bindingLayout.get()) :
                                     nullptr;
    D3D12ComputeShader* computeShader = _desc.m_computeShader && hasValidComputeShader ?
                                            static_cast<D3D12ComputeShader*>(_desc.m_computeShader.get()) :
                                            nullptr;

    EGO_ASSERT_MESSAGE(!_desc.m_bindingLayout || layout, "BindingLayout must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(!_desc.m_computeShader || computeShader, "Compute shader must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(hasValidComputeShader, "Compute pipeline shader has invalid shader stage");

    if ((_desc.m_bindingLayout && !layout) ||
        (_desc.m_computeShader && !computeShader) ||
        !hasValidComputeShader)
    {
        return ComputePipelinePointer();
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = layout ? layout->getRootSignature() : nullptr;
    psoDesc.CS = computeShader ? computeShader->getD3D12ByteCode() : D3D12_SHADER_BYTECODE{};
    psoDesc.NodeMask = 0;
    psoDesc.CachedPSO = {};
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
    if (FAILED(m_device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState))))
    {
        return ComputePipelinePointer();
    }

    return ComputePipelinePointer(new D3D12ComputePipeline(_desc, std::move(pipelineState), layout));
}

ego::gpu::BindingLayoutPointer ego::gpu::d3d12::D3D12GraphicDevice::createBindingLayout(
    const BindingLayoutDesc& _desc
)
{
    std::vector<D3D12_ROOT_PARAMETER1> rootParameters;
    std::vector<D3D12BindingLayout::PushConstantInfo> pushConstants;
    pushConstants.reserve(_desc.m_pushConstants.size());

    for (const PushConstantRangeDesc& pushConstantRange : _desc.m_pushConstants)
    {
        EGO_ASSERT_MESSAGE(
            (pushConstantRange.m_size % sizeof(uint32_t)) == 0,
            "D3D12 push constants must be 32-bit aligned"
        );

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

    if (m_capabilities.m_supportsBindlessResources)
    {
        rootSignatureDesc.Desc_1_1.Flags |=
            D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
            D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;
    }

    Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSignature;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    if (FAILED(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &serializedRootSignature, &errorBlob)))
    {
        return BindingLayoutPointer();
    }

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    if (FAILED(
        m_device->CreateRootSignature(
            0,
            serializedRootSignature->GetBufferPointer(),
            serializedRootSignature->GetBufferSize(),
            IID_PPV_ARGS(&rootSignature)
        )
    ))
    {
        return BindingLayoutPointer();
    }

    return BindingLayoutPointer(
        new D3D12BindingLayout(_desc, std::move(rootSignature), std::move(pushConstants))
    );
}

ego::gpu::FencePointer ego::gpu::d3d12::D3D12GraphicDevice::createFence(Fence::FenceValue _initialValue)
{
    if (!m_device)
    {
        return FencePointer();
    }

    Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    if (FAILED(m_device->CreateFence(_initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
    {
        return FencePointer();
    }

    return FencePointer(new D3D12Fence(_initialValue, std::move(fence)));
}

ego::gpu::SwapChainPointer ego::gpu::d3d12::D3D12GraphicDevice::createSwapChain(
    const SwapChainDesc& _swapChainDesc,
    const Window& _window
)
{
    if (!m_factory || !m_device || !_window.getNativeHandle())
    {
        return SwapChainPointer();
    }

    if (_swapChainDesc.m_bufferCount == 0)
    {
        return SwapChainPointer();
    }

    const DXGI_FORMAT format = ToDXGIFormat(_swapChainDesc.m_format);
    if (format == DXGI_FORMAT_UNKNOWN)
    {
        return SwapChainPointer();
    }

    ID3D12CommandQueue* presentationQueue = nullptr;
    for (D3D12CommandQueue* queue : m_queues)
    {
        if (queue &&
            queue->getDesc().m_type == CommandType::Graphic &&
            queue->getDesc().m_supportsPresentation)
        {
            presentationQueue = queue->getQueue();
            break;
        }
    }

    for (D3D12CommandQueue* queue : m_queues)
    {
        if (!presentationQueue && queue && queue->getDesc().m_type == CommandType::Graphic)
        {
            presentationQueue = queue->getQueue();
            break;
        }
    }

    CommandQueuePointer ownedPresentationQueue;
    if (!presentationQueue)
    {
        CommandQueueDesc queueDesc;
        queueDesc.m_type = CommandType::Graphic;
        queueDesc.m_supportsPresentation = true;
        ownedPresentationQueue = createCommandQueue(queueDesc);

        D3D12CommandQueue* d3d12Queue = static_cast<D3D12CommandQueue*>(ownedPresentationQueue.get());
        presentationQueue = d3d12Queue ? d3d12Queue->getQueue() : nullptr;
    }

    if (!presentationQueue)
    {
        return SwapChainPointer();
    }

    const WindowSize& clientAreaSize = _window.getClientAreaSize();
    const uint32_t width = clientAreaSize.m_x;
    const uint32_t height = clientAreaSize.m_y;

    if (width == 0 || height == 0)
    {
        return SwapChainPointer();
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

    HWND windowHandle = reinterpret_cast<HWND>(_window.getNativeHandle());
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
    if (FAILED(
        m_factory->CreateSwapChainForHwnd(
            presentationQueue,
            windowHandle,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain
        )
    ))
    {
        return SwapChainPointer();
    }

    m_factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER);

    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain3;
    if (FAILED(swapChain.As(&swapChain3)))
    {
        return SwapChainPointer();
    }

    std::vector<Texture2DPointer> targetTextures;
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
            return SwapChainPointer();
        }

        targetTextures.push_back(Texture2DPointer(new D3D12Texture2D(textureDesc, std::move(resource))));
    }

    return SwapChainPointer(
        new D3D12SwapChain(
            _swapChainDesc,
            std::move(swapChain3),
            std::move(targetTextures),
            std::move(ownedPresentationQueue)
        )
    );
}

const ego::gpu::GraphicDeviceCapabilities& ego::gpu::d3d12::D3D12GraphicDevice::getCapabilities() const
{
    return m_capabilities;
}

void ego::gpu::d3d12::D3D12GraphicDevice::waitIdle()
{
    for (D3D12CommandQueue* queue : m_queues)
    {
        if (queue)
        {
            queue->waitIdle();
        }
    }
}

ID3D12Device* ego::gpu::d3d12::D3D12GraphicDevice::getDevice() const
{
    return m_device.Get();
}

IDXGIAdapter1* ego::gpu::d3d12::D3D12GraphicDevice::getAdapter() const
{
    return m_adapter.Get();
}

ego::gpu::d3d12::D3D12DescriptorAllocatorPointer ego::gpu::d3d12::D3D12GraphicDevice::getViewDescriptorAllocator()
{
    return m_viewDescriptorAllocator;
}

ego::gpu::d3d12::D3D12DescriptorAllocatorPointer ego::gpu::d3d12::D3D12GraphicDevice::getSamplerDescriptorAllocator()
{
    return m_samplerDescriptorAllocator;
}

ego::gpu::d3d12::D3D12DescriptorAllocatorPointer ego::gpu::d3d12::D3D12GraphicDevice::getRtvDescriptorAllocator()
{
    return m_rtvDescriptorAllocator;
}

ego::gpu::d3d12::D3D12DescriptorAllocatorPointer ego::gpu::d3d12::D3D12GraphicDevice::getDsvDescriptorAllocator()
{
    return m_dsvDescriptorAllocator;
}

void ego::gpu::d3d12::D3D12GraphicDevice::registerQueue(D3D12CommandQueue* _queue)
{
    if (_queue && std::find(m_queues.begin(), m_queues.end(), _queue) == m_queues.end())
    {
        m_queues.push_back(_queue);
    }
}

void ego::gpu::d3d12::D3D12GraphicDevice::unregisterQueue(D3D12CommandQueue* _queue)
{
    const auto iterator = std::find(m_queues.begin(), m_queues.end(), _queue);
    if (iterator != m_queues.end())
    {
        m_queues.erase(iterator);
    }
}

bool ego::gpu::d3d12::D3D12GraphicDevice::initializeFactory(const GraphicDeviceInitParams& _params)
{
    UINT factoryFlags = 0;

    if (_params.m_debugEnable)
    {
        Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
            debugController->SetEnableGPUBasedValidation(_params.m_gpuValidation);
            factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }

    return SUCCEEDED(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory)));
}

bool ego::gpu::d3d12::D3D12GraphicDevice::initializeAdapter()
{
    for (UINT adapterIndex = 0;; ++adapterIndex)
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
        if (FAILED(
            m_factory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&
                adapter))
        ))
        {
            break;
        }

        DXGI_ADAPTER_DESC1 adapterDesc = {};
        adapter->GetDesc1(&adapterDesc);
        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
        {
            m_adapter = adapter;
            return true;
        }
    }

    return false;
}

bool ego::gpu::d3d12::D3D12GraphicDevice::initializeDevice()
{
    if (FAILED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device))))
    {
        return false;
    }

    initializeCapabilities();

    return true;
}

bool ego::gpu::d3d12::D3D12GraphicDevice::initializeDescriptorAllocators()
{
    m_viewDescriptorAllocator.reset(new D3D12DescriptorAllocator());
    EGO_CHECK_INITIALIZATION(
        m_viewDescriptorAllocator &&
        m_viewDescriptorAllocator->init(
            m_device.Get(),
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            BindlessResourceDescriptorCapacity,
            true
        )
    );

    m_samplerDescriptorAllocator.reset(new D3D12DescriptorAllocator());
    EGO_CHECK_INITIALIZATION(
        m_samplerDescriptorAllocator &&
        m_samplerDescriptorAllocator->init(
            m_device.Get(),
            D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
            BindlessSamplerDescriptorCapacity,
            true
        )
    );

    m_rtvDescriptorAllocator.reset(new D3D12DescriptorAllocator());
    EGO_CHECK_INITIALIZATION(
        m_rtvDescriptorAllocator &&
        m_rtvDescriptorAllocator->init(
            m_device.Get(),
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            256,
            false
        )
    );

    m_dsvDescriptorAllocator.reset(new D3D12DescriptorAllocator());
    EGO_CHECK_INITIALIZATION(
        m_dsvDescriptorAllocator &&
        m_dsvDescriptorAllocator->init(
            m_device.Get(),
            D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            256,
            false
        )
    );

    return true;
}

void ego::gpu::d3d12::D3D12GraphicDevice::initializeCapabilities()
{
    m_capabilities.m_maxColorAttachments = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
    m_capabilities.m_supportsPresentation = true;
    m_capabilities.m_supportsIndependentComputeQueue = true;
    m_capabilities.m_supportsSamplerAnisotropy = true;
    m_capabilities.m_supportsMeshShaders = false;
    m_capabilities.m_supportsRayTracing = false;
    m_capabilities.m_supportsBindlessResources = false;
    m_capabilities.m_bindlessResourceDescriptorCount = BindlessResourceDescriptorCapacity;
    m_capabilities.m_bindlessSamplerDescriptorCount = BindlessSamplerDescriptorCapacity;

    bool supportsResourceBindingTier = false;
    D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
    if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options))))
    {
        supportsResourceBindingTier = options.ResourceBindingTier >= D3D12_RESOURCE_BINDING_TIER_2;
    }

    bool supportsDirectHeapIndexing = false;
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {};
    shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_6;
    if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))))
    {
        supportsDirectHeapIndexing = shaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_6;
    }

    m_capabilities.m_supportsBindlessResources = supportsResourceBindingTier && supportsDirectHeapIndexing;

    D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
    if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7))))
    {
        m_capabilities.m_supportsMeshShaders = options7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
    if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5))))
    {
        m_capabilities.m_supportsRayTracing = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
    }
}
