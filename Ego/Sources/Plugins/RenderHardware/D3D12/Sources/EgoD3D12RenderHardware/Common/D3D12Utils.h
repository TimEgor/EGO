#pragma once

#include <cstdint>
#include <string>

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "EgoEngine/RenderHardware/GraphicObjects/BindingLayout.h"
#include "EgoEngine/RenderHardware/GraphicObjects/CommandList.h"
#include "EgoEngine/RenderHardware/GraphicObjects/Format.h"
#include "EgoEngine/RenderHardware/GraphicObjects/GraphicResource.h"
#include "EgoEngine/RenderHardware/GraphicObjects/InputLayout.h"
#include "EgoEngine/RenderHardware/GraphicObjects/Pipeline.h"
#include "EgoEngine/RenderHardware/GraphicObjects/Sampler.h"
#include "EgoEngine/RenderHardware/GraphicObjects/Texture.h"

namespace ego::gpu::d3d12
{
    using D3D12DescriptorIndex = uint32_t;
    inline constexpr D3D12DescriptorIndex D3D12InvalidDescriptorIndex = -1;

    inline constexpr uint32_t InvalidRootParameterIndex = -1;

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

    inline uint64_t AlignTo(uint64_t _value, uint64_t _alignment)
    {
        return (_value + (_alignment - 1)) & ~(_alignment - 1);
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

    inline D3D12_DESCRIPTOR_RANGE_TYPE ToD3D12DescriptorRangeType(BindingResourceType _type)
    {
        switch (_type)
        {
        case BindingResourceType::ConstantBuffer:
            return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        case BindingResourceType::Texture:
        case BindingResourceType::StructuredBuffer:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        case BindingResourceType::StorageTexture:
        case BindingResourceType::StorageBuffer:
            return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        case BindingResourceType::Sampler:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        default:
            break;
        }

        return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    }

    inline bool IsSamplerBindingType(BindingResourceType _type)
    {
        return _type == BindingResourceType::Sampler;
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

    inline bool IsDepthFormat(GraphicResourceFormat _format)
    {
        return _format == GraphicResourceFormat::D16UNorm ||
            _format == GraphicResourceFormat::D24UNormS8UInt ||
            _format == GraphicResourceFormat::D32SFloat ||
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
}
