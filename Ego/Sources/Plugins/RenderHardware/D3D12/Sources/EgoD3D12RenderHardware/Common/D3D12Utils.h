#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/BindingLayout.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Buffer.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/CommandList.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Format.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/GraphicResource.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/InputLayout.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Pipeline.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Sampler.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Shader.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Texture.h"

namespace ego::gpu::d3d12
{
    using D3D12DescriptorIndex = uint32_t;
    inline constexpr D3D12DescriptorIndex D3D12InvalidDescriptorIndex = -1;

    inline constexpr uint32_t InvalidRootParameterIndex = -1;

    inline bool FitsUint32(uint64_t _value)
    {
        return _value <= (std::numeric_limits<uint32_t>::max)();
    }

    inline std::wstring ToWideString(const char* _value)
    {
        if (!_value || !_value[0])
        {
            return std::wstring();
        }

        const int size = MultiByteToWideChar(CP_UTF8, 0, _value, -1, nullptr, 0);
        if (size <= 1)
        {
            return std::wstring();
        }

        std::wstring result;
        result.resize(static_cast<size_t>(size));
        MultiByteToWideChar(CP_UTF8, 0, _value, -1, result.data(), size);
        result.pop_back();
        return result;
    }

    inline void SetD3D12ObjectName(ID3D12Object* _object, const char* _name)
    {
        const std::wstring name = ToWideString(_name);
        if (_object && !name.empty())
        {
            _object->SetName(name.c_str());
        }
    }

    inline D3D12_COMMAND_LIST_TYPE ToD3D12CommandListType(CommandType _type)
    {
        switch (_type)
        {
        case CommandType::Graphic:
            return D3D12_COMMAND_LIST_TYPE_DIRECT;
        case CommandType::Compute:
            return D3D12_COMMAND_LIST_TYPE_COMPUTE;
        case CommandType::Copy:
            return D3D12_COMMAND_LIST_TYPE_COPY;
        default:
            break;
        }

        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    }

    inline D3D12_SHADER_VISIBILITY ToD3D12ShaderVisibility(ShaderStageFlags _flags)
    {
        if (_flags == ShaderStageFlagVertex)
        {
            return D3D12_SHADER_VISIBILITY_VERTEX;
        }

        if (_flags == ShaderStageFlagPixel)
        {
            return D3D12_SHADER_VISIBILITY_PIXEL;
        }

        return D3D12_SHADER_VISIBILITY_ALL;
    }

    inline bool IsValidShaderCode(const ShaderCodeReference& _code)
    {
        return _code && _code->getCode() && _code->getCodeSize();
    }

    inline D3D12_TEXTURE_ADDRESS_MODE ToD3D12AddressMode(SamplerAddressMode _mode)
    {
        switch (_mode)
        {
        case SamplerAddressMode::Repeat:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case SamplerAddressMode::MirroredRepeat:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case SamplerAddressMode::ClampToBorder:
            return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case SamplerAddressMode::ClampToEdge:
        default:
            break;
        }

        return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    }

    inline D3D12_FILTER ToD3D12Filter(const SamplerDesc& _desc)
    {
        if (_desc.m_minFilter == SamplerFilter::Anisotropic || _desc.m_magFilter == SamplerFilter::Anisotropic || _desc.m_mipFilter == SamplerFilter::Anisotropic)
        {
            return _desc.m_enableComparison ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
        }

        const bool minLinear = _desc.m_minFilter == SamplerFilter::Linear;
        const bool magLinear = _desc.m_magFilter == SamplerFilter::Linear;
        const bool mipLinear = _desc.m_mipFilter == SamplerFilter::Linear;

        if (_desc.m_enableComparison)
        {
            return D3D12_ENCODE_BASIC_FILTER(
                minLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
                magLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
                mipLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
                D3D12_FILTER_REDUCTION_TYPE_COMPARISON);
        }

        return D3D12_ENCODE_BASIC_FILTER(
            minLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
            magLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
            mipLinear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
            D3D12_FILTER_REDUCTION_TYPE_STANDARD);
    }

    inline D3D12_RESOURCE_STATES ToD3D12ResourceState(GraphicResourceState _state)
    {
        switch (_state)
        {
        case GraphicResourceState::Undefined:
        case GraphicResourceState::Common:
            return D3D12_RESOURCE_STATE_COMMON;
        case GraphicResourceState::CopySrc:
            return D3D12_RESOURCE_STATE_COPY_SOURCE;
        case GraphicResourceState::CopyDst:
            return D3D12_RESOURCE_STATE_COPY_DEST;
        case GraphicResourceState::VertexBuffer:
            return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        case GraphicResourceState::IndexBuffer:
            return D3D12_RESOURCE_STATE_INDEX_BUFFER;
        case GraphicResourceState::ConstantBuffer:
            return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        case GraphicResourceState::ShaderRead:
            return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        case GraphicResourceState::UnorderedAccess:
            return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        case GraphicResourceState::RenderTarget:
            return D3D12_RESOURCE_STATE_RENDER_TARGET;
        case GraphicResourceState::DepthWrite:
            return D3D12_RESOURCE_STATE_DEPTH_WRITE;
        case GraphicResourceState::DepthRead:
            return D3D12_RESOURCE_STATE_DEPTH_READ;
        case GraphicResourceState::IndirectBuffer:
            return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        case GraphicResourceState::RayTracingAccelerationStructure:
            return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        case GraphicResourceState::Present:
            return D3D12_RESOURCE_STATE_PRESENT;
        default:
            break;
        }

        return D3D12_RESOURCE_STATE_COMMON;
    }

    inline D3D12_RESOURCE_FLAGS ToD3D12ResourceFlags(uint32_t _usage)
    {
        D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAG_NONE;

        if (_usage & GraphicResourceUsageAllowUnorderedAccess)
        {
            result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        if (_usage & TextureUsageRenderTarget)
        {
            result |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }

        if (_usage & TextureUsageDepthStencil)
        {
            result |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        }

        return result;
    }

    inline D3D12_HEAP_TYPE GetHeapType(const GraphicResourceDesc& _desc)
    {
        if (_desc.m_access & GraphicResourceAccessCpuWrite)
        {
            return D3D12_HEAP_TYPE_UPLOAD;
        }

        if (_desc.m_access & GraphicResourceAccessCpuRead)
        {
            return D3D12_HEAP_TYPE_READBACK;
        }

        return D3D12_HEAP_TYPE_DEFAULT;
    }

    inline D3D12_RESOURCE_STATES GetInitialBufferState(const BufferDesc& _desc, const InitialGraphicResourceData& _initialData)
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

        if (_desc.m_usage & GraphicResourceUsageRayTracingAccelerationStructure)
        {
            return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        return D3D12_RESOURCE_STATE_COMMON;
    }

    inline D3D12_RESOURCE_FLAGS GetBufferResourceFlags(const BufferDesc& _desc)
    {
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
        if (_desc.m_usage & GraphicResourceUsageAllowUnorderedAccess)
        {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        return flags;
    }

    inline bool WriteToUploadBuffer(ID3D12Resource* _resource, const void* _data, uint64_t _dataSize, uint64_t _resourceSize)
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

    inline uint64_t ResolveViewSize(uint64_t _resourceSize, uint64_t _offset, uint64_t _explicitSize)
    {
        if (_explicitSize)
        {
            return _explicitSize;
        }

        return _offset < _resourceSize ? _resourceSize - _offset : 0;
    }

    inline bool IsDepthFormat(GraphicResourceFormat _format)
    {
        return _format == GraphicResourceFormat::D16UNorm || _format == GraphicResourceFormat::D24UNormS8UInt || _format == GraphicResourceFormat::D32SFloat ||
               _format == GraphicResourceFormat::D32SFloatS8UInt;
    }

    inline DXGI_FORMAT ToDXGIFormat(GraphicResourceFormat _format)
    {
        switch (_format)
        {
        case GraphicResourceFormat::R8UNorm:
            return DXGI_FORMAT_R8_UNORM;
        case GraphicResourceFormat::R8SNorm:
            return DXGI_FORMAT_R8_SNORM;
        case GraphicResourceFormat::R8UInt:
            return DXGI_FORMAT_R8_UINT;
        case GraphicResourceFormat::R8SInt:
            return DXGI_FORMAT_R8_SINT;
        case GraphicResourceFormat::R8G8UNorm:
            return DXGI_FORMAT_R8G8_UNORM;
        case GraphicResourceFormat::R8G8SNorm:
            return DXGI_FORMAT_R8G8_SNORM;
        case GraphicResourceFormat::R8G8UInt:
            return DXGI_FORMAT_R8G8_UINT;
        case GraphicResourceFormat::R8G8SInt:
            return DXGI_FORMAT_R8G8_SINT;
        case GraphicResourceFormat::R8G8B8A8UNorm:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case GraphicResourceFormat::R8G8B8A8SNorm:
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        case GraphicResourceFormat::R8G8B8A8UInt:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case GraphicResourceFormat::R8G8B8A8SInt:
            return DXGI_FORMAT_R8G8B8A8_SINT;
        case GraphicResourceFormat::B8G8R8A8UNorm:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case GraphicResourceFormat::R10G10B10A2UNorm:
            return DXGI_FORMAT_R10G10B10A2_UNORM;
        case GraphicResourceFormat::R11G11B10SFloat:
            return DXGI_FORMAT_R11G11B10_FLOAT;
        case GraphicResourceFormat::R16UNorm:
            return DXGI_FORMAT_R16_UNORM;
        case GraphicResourceFormat::R16SNorm:
            return DXGI_FORMAT_R16_SNORM;
        case GraphicResourceFormat::R16UInt:
            return DXGI_FORMAT_R16_UINT;
        case GraphicResourceFormat::R16SInt:
            return DXGI_FORMAT_R16_SINT;
        case GraphicResourceFormat::R16SFloat:
            return DXGI_FORMAT_R16_FLOAT;
        case GraphicResourceFormat::R16G16UNorm:
            return DXGI_FORMAT_R16G16_UNORM;
        case GraphicResourceFormat::R16G16SNorm:
            return DXGI_FORMAT_R16G16_SNORM;
        case GraphicResourceFormat::R16G16UInt:
            return DXGI_FORMAT_R16G16_UINT;
        case GraphicResourceFormat::R16G16SInt:
            return DXGI_FORMAT_R16G16_SINT;
        case GraphicResourceFormat::R16G16SFloat:
            return DXGI_FORMAT_R16G16_FLOAT;
        case GraphicResourceFormat::R16G16B16A16UNorm:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case GraphicResourceFormat::R16G16B16A16SNorm:
            return DXGI_FORMAT_R16G16B16A16_SNORM;
        case GraphicResourceFormat::R16G16B16A16UInt:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case GraphicResourceFormat::R16G16B16A16SInt:
            return DXGI_FORMAT_R16G16B16A16_SINT;
        case GraphicResourceFormat::R16G16B16A16SFloat:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case GraphicResourceFormat::R32UInt:
            return DXGI_FORMAT_R32_UINT;
        case GraphicResourceFormat::R32SInt:
            return DXGI_FORMAT_R32_SINT;
        case GraphicResourceFormat::R32SFloat:
            return DXGI_FORMAT_R32_FLOAT;
        case GraphicResourceFormat::R32G32UInt:
            return DXGI_FORMAT_R32G32_UINT;
        case GraphicResourceFormat::R32G32SInt:
            return DXGI_FORMAT_R32G32_SINT;
        case GraphicResourceFormat::R32G32SFloat:
            return DXGI_FORMAT_R32G32_FLOAT;
        case GraphicResourceFormat::R32G32B32UInt:
            return DXGI_FORMAT_R32G32B32_UINT;
        case GraphicResourceFormat::R32G32B32SInt:
            return DXGI_FORMAT_R32G32B32_SINT;
        case GraphicResourceFormat::R32G32B32SFloat:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case GraphicResourceFormat::R32G32B32A32UInt:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        case GraphicResourceFormat::R32G32B32A32SInt:
            return DXGI_FORMAT_R32G32B32A32_SINT;
        case GraphicResourceFormat::R32G32B32A32SFloat:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case GraphicResourceFormat::D16UNorm:
            return DXGI_FORMAT_D16_UNORM;
        case GraphicResourceFormat::D24UNormS8UInt:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case GraphicResourceFormat::D32SFloat:
            return DXGI_FORMAT_D32_FLOAT;
        case GraphicResourceFormat::D32SFloatS8UInt:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        case GraphicResourceFormat::Undefined:
        default:
            break;
        }

        return DXGI_FORMAT_UNKNOWN;
    }

    inline uint32_t GetFormatStride(GraphicResourceFormat _format)
    {
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

    inline bool IsSupportedRayTracingIndexFormat(GraphicResourceFormat _format)
    {
        return _format == GraphicResourceFormat::Undefined || _format == GraphicResourceFormat::R16UInt || _format == GraphicResourceFormat::R32UInt;
    }

    inline DXGI_FORMAT ToDXGIFormat(InputLayoutElementType _type, uint32_t _componentsCount)
    {
        switch (_type)
        {
        case InputLayoutElementType::Int8:
            switch (_componentsCount)
            {
            case 1:
                return DXGI_FORMAT_R8_SINT;
            case 2:
                return DXGI_FORMAT_R8G8_SINT;
            case 4:
                return DXGI_FORMAT_R8G8B8A8_SINT;
            default:
                break;
            }
            break;
        case InputLayoutElementType::UInt8:
            switch (_componentsCount)
            {
            case 1:
                return DXGI_FORMAT_R8_UINT;
            case 2:
                return DXGI_FORMAT_R8G8_UINT;
            case 4:
                return DXGI_FORMAT_R8G8B8A8_UINT;
            default:
                break;
            }
            break;
        case InputLayoutElementType::Int16:
            switch (_componentsCount)
            {
            case 1:
                return DXGI_FORMAT_R16_SINT;
            case 2:
                return DXGI_FORMAT_R16G16_SINT;
            case 4:
                return DXGI_FORMAT_R16G16B16A16_SINT;
            default:
                break;
            }
            break;
        case InputLayoutElementType::UInt16:
            switch (_componentsCount)
            {
            case 1:
                return DXGI_FORMAT_R16_UINT;
            case 2:
                return DXGI_FORMAT_R16G16_UINT;
            case 4:
                return DXGI_FORMAT_R16G16B16A16_UINT;
            default:
                break;
            }
            break;
        case InputLayoutElementType::Int32:
            switch (_componentsCount)
            {
            case 1:
                return DXGI_FORMAT_R32_SINT;
            case 2:
                return DXGI_FORMAT_R32G32_SINT;
            case 3:
                return DXGI_FORMAT_R32G32B32_SINT;
            case 4:
                return DXGI_FORMAT_R32G32B32A32_SINT;
            default:
                break;
            }
            break;
        case InputLayoutElementType::UInt32:
            switch (_componentsCount)
            {
            case 1:
                return DXGI_FORMAT_R32_UINT;
            case 2:
                return DXGI_FORMAT_R32G32_UINT;
            case 3:
                return DXGI_FORMAT_R32G32B32_UINT;
            case 4:
                return DXGI_FORMAT_R32G32B32A32_UINT;
            default:
                break;
            }
            break;
        case InputLayoutElementType::Float16:
            switch (_componentsCount)
            {
            case 1:
                return DXGI_FORMAT_R16_FLOAT;
            case 2:
                return DXGI_FORMAT_R16G16_FLOAT;
            case 4:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;
            default:
                break;
            }
            break;
        case InputLayoutElementType::Float32:
            switch (_componentsCount)
            {
            case 1:
                return DXGI_FORMAT_R32_FLOAT;
            case 2:
                return DXGI_FORMAT_R32G32_FLOAT;
            case 3:
                return DXGI_FORMAT_R32G32B32_FLOAT;
            case 4:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            default:
                break;
            }
            break;
        default:
            break;
        }

        return DXGI_FORMAT_UNKNOWN;
    }

    inline D3D12_FILL_MODE ToD3D12FillMode(RasterizationFillMode _mode)
    {
        return _mode == RasterizationFillMode::Wireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
    }

    inline D3D12_CULL_MODE ToD3D12CullMode(RasterizationCullMode _mode)
    {
        switch (_mode)
        {
        case RasterizationCullMode::BackFace:
            return D3D12_CULL_MODE_BACK;
        case RasterizationCullMode::FrontFace:
            return D3D12_CULL_MODE_FRONT;
        case RasterizationCullMode::None:
        default:
            break;
        }

        return D3D12_CULL_MODE_NONE;
    }

    inline D3D12_COMPARISON_FUNC ToD3D12ComparisonFunc(CompareOperation _operation)
    {
        switch (_operation)
        {
        case CompareOperation::Never:
            return D3D12_COMPARISON_FUNC_NEVER;
        case CompareOperation::Less:
            return D3D12_COMPARISON_FUNC_LESS;
        case CompareOperation::Equal:
            return D3D12_COMPARISON_FUNC_EQUAL;
        case CompareOperation::LessEqual:
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case CompareOperation::Greater:
            return D3D12_COMPARISON_FUNC_GREATER;
        case CompareOperation::NotEqual:
            return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case CompareOperation::GreaterEqual:
            return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case CompareOperation::Always:
        default:
            break;
        }

        return D3D12_COMPARISON_FUNC_ALWAYS;
    }

    inline D3D12_BLEND ToD3D12Blend(BlendFactor _factor)
    {
        switch (_factor)
        {
        case BlendFactor::Zero:
            return D3D12_BLEND_ZERO;
        case BlendFactor::One:
            return D3D12_BLEND_ONE;
        case BlendFactor::SrcColor:
            return D3D12_BLEND_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor:
            return D3D12_BLEND_INV_SRC_COLOR;
        case BlendFactor::DstColor:
            return D3D12_BLEND_DEST_COLOR;
        case BlendFactor::OneMinusDstColor:
            return D3D12_BLEND_INV_DEST_COLOR;
        case BlendFactor::SrcAlpha:
            return D3D12_BLEND_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha:
            return D3D12_BLEND_INV_SRC_ALPHA;
        case BlendFactor::DstAlpha:
            return D3D12_BLEND_DEST_ALPHA;
        case BlendFactor::OneMinusDstAlpha:
            return D3D12_BLEND_INV_DEST_ALPHA;
        default:
            break;
        }

        return D3D12_BLEND_ONE;
    }

    inline D3D12_BLEND_OP ToD3D12BlendOperation(BlendOperation _operation)
    {
        switch (_operation)
        {
        case BlendOperation::Add:
            return D3D12_BLEND_OP_ADD;
        case BlendOperation::Subtract:
            return D3D12_BLEND_OP_SUBTRACT;
        case BlendOperation::ReverseSubtract:
            return D3D12_BLEND_OP_REV_SUBTRACT;
        case BlendOperation::Min:
            return D3D12_BLEND_OP_MIN;
        case BlendOperation::Max:
            return D3D12_BLEND_OP_MAX;
        default:
            break;
        }

        return D3D12_BLEND_OP_ADD;
    }

    inline D3D12_PRIMITIVE_TOPOLOGY_TYPE ToD3D12PrimitiveTopologyType(PrimitiveTopology _topology)
    {
        switch (_topology)
        {
        case PrimitiveTopology::PointList:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        case PrimitiveTopology::LineList:
        case PrimitiveTopology::LineStrip:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case PrimitiveTopology::TriangleList:
        case PrimitiveTopology::TriangleStrip:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        default:
            break;
        }

        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    }

    inline D3D_PRIMITIVE_TOPOLOGY ToD3DPrimitiveTopology(PrimitiveTopology _topology)
    {
        switch (_topology)
        {
        case PrimitiveTopology::PointList:
            return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case PrimitiveTopology::LineList:
            return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case PrimitiveTopology::LineStrip:
            return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case PrimitiveTopology::TriangleList:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case PrimitiveTopology::TriangleStrip:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        default:
            break;
        }

        return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }
} // namespace ego::gpu::d3d12
