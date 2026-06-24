#include "D3D12Pipeline.h"

#include <utility>

#include "../Common/D3D12Utils.h"

namespace
{
    std::vector<uint8_t> CopyShaderCode(const ego::gpu::ShaderCodeReference& _code)
    {
        if (!_code || !_code->getCode() || !_code->getCodeSize())
        {
            return {};
        }

        auto codeBegin = static_cast<const uint8_t*>(_code->getCode());
        return std::vector<uint8_t>(codeBegin, codeBegin + _code->getCodeSize());
    }
} // namespace

ego::gpu::d3d12::D3D12BindingLayout::D3D12BindingLayout(
    const BindingLayoutDesc& _desc,
    Microsoft::WRL::ComPtr<ID3D12RootSignature>&& _rootSignature,
    std::vector<PushConstantInfo>&& _pushConstants)
    : BindingLayout(_desc),
      m_rootSignature(std::move(_rootSignature)),
      m_pushConstants(std::move(_pushConstants))
{
}

void* ego::gpu::d3d12::D3D12BindingLayout::getNativeHandle() const
{
    return m_rootSignature.Get();
}

void ego::gpu::d3d12::D3D12BindingLayout::setName(const char* _name)
{
    SetD3D12ObjectName(m_rootSignature.Get(), _name);
}

ID3D12RootSignature* ego::gpu::d3d12::D3D12BindingLayout::getRootSignature() const
{
    return m_rootSignature.Get();
}

const ego::gpu::d3d12::D3D12BindingLayout::PushConstantInfo* ego::gpu::d3d12::D3D12BindingLayout::findPushConstantRange(
    ShaderStageFlags _stageFlags,
    uint32_t _offset,
    uint32_t _size) const
{
    for (const PushConstantInfo& pushConstant : m_pushConstants)
    {
        const bool stageMatch = (pushConstant.m_stageFlags & _stageFlags) == _stageFlags || pushConstant.m_stageFlags == ShaderStageFlagAll;
        const bool offsetMatch = _offset >= pushConstant.m_offset;
        const bool sizeMatch = (_offset + _size) <= (pushConstant.m_offset + pushConstant.m_size);

        if (stageMatch && offsetMatch && sizeMatch)
        {
            return &pushConstant;
        }
    }

    return nullptr;
}

ego::gpu::d3d12::D3D12GraphicPipeline::D3D12GraphicPipeline(
    const GraphicPipelineDesc& _desc,
    Microsoft::WRL::ComPtr<ID3D12PipelineState>&& _pipelineState,
    const D3D12BindingLayout* _layout)
    : GraphicPipeline(_desc),
      m_pipelineState(std::move(_pipelineState)),
      m_layout(_layout),
      m_primitiveTopology(ToD3DPrimitiveTopology(_desc.m_topology))
{
}

void* ego::gpu::d3d12::D3D12GraphicPipeline::getNativeHandle() const
{
    return m_pipelineState.Get();
}

void ego::gpu::d3d12::D3D12GraphicPipeline::setName(const char* _name)
{
    SetD3D12ObjectName(m_pipelineState.Get(), _name);
}

ID3D12PipelineState* ego::gpu::d3d12::D3D12GraphicPipeline::getPipelineState() const
{
    return m_pipelineState.Get();
}

const ego::gpu::d3d12::D3D12BindingLayout* ego::gpu::d3d12::D3D12GraphicPipeline::getBindingLayout() const
{
    return m_layout;
}

D3D_PRIMITIVE_TOPOLOGY ego::gpu::d3d12::D3D12GraphicPipeline::getPrimitiveTopology() const
{
    return m_primitiveTopology;
}

ego::gpu::d3d12::D3D12ComputePipeline::D3D12ComputePipeline(
    const ComputePipelineDesc& _desc,
    Microsoft::WRL::ComPtr<ID3D12PipelineState>&& _pipelineState,
    const D3D12BindingLayout* _layout)
    : ComputePipeline(_desc),
      m_pipelineState(std::move(_pipelineState)),
      m_layout(_layout)
{
}

void* ego::gpu::d3d12::D3D12ComputePipeline::getNativeHandle() const
{
    return m_pipelineState.Get();
}

void ego::gpu::d3d12::D3D12ComputePipeline::setName(const char* _name)
{
    SetD3D12ObjectName(m_pipelineState.Get(), _name);
}

ID3D12PipelineState* ego::gpu::d3d12::D3D12ComputePipeline::getPipelineState() const
{
    return m_pipelineState.Get();
}

const ego::gpu::d3d12::D3D12BindingLayout* ego::gpu::d3d12::D3D12ComputePipeline::getBindingLayout() const
{
    return m_layout;
}

ego::gpu::d3d12::D3D12RayTracingPipeline::D3D12RayTracingPipeline(
    const RayTracingPipelineDesc& _desc,
    Microsoft::WRL::ComPtr<ID3D12StateObject>&& _stateObject,
    Microsoft::WRL::ComPtr<ID3D12Resource>&& _shaderTable,
    uint64_t _shaderRecordSize,
    const D3D12BindingLayout* _layout)
    : RayTracingPipeline(_desc),
      m_stateObject(std::move(_stateObject)),
      m_shaderTable(std::move(_shaderTable)),
      m_shaderRecordSize(_shaderRecordSize),
      m_layout(_layout)
{
}

void* ego::gpu::d3d12::D3D12RayTracingPipeline::getNativeHandle() const
{
    return m_stateObject.Get();
}

void ego::gpu::d3d12::D3D12RayTracingPipeline::setName(const char* _name)
{
    SetD3D12ObjectName(m_stateObject.Get(), _name);
}

ID3D12StateObject* ego::gpu::d3d12::D3D12RayTracingPipeline::getStateObject() const
{
    return m_stateObject.Get();
}

const ego::gpu::d3d12::D3D12BindingLayout* ego::gpu::d3d12::D3D12RayTracingPipeline::getBindingLayout() const
{
    return m_layout;
}

D3D12_DISPATCH_RAYS_DESC ego::gpu::d3d12::D3D12RayTracingPipeline::getDispatchRaysDesc(uint32_t _width, uint32_t _height, uint32_t _depth) const
{
    D3D12_DISPATCH_RAYS_DESC desc = {};
    if (!m_shaderTable || m_shaderRecordSize == 0)
    {
        return desc;
    }

    const D3D12_GPU_VIRTUAL_ADDRESS shaderTableAddress = m_shaderTable->GetGPUVirtualAddress();

    desc.RayGenerationShaderRecord.StartAddress = shaderTableAddress;
    desc.RayGenerationShaderRecord.SizeInBytes = m_shaderRecordSize;
    desc.MissShaderTable.StartAddress = shaderTableAddress + m_shaderRecordSize;
    desc.MissShaderTable.SizeInBytes = m_shaderRecordSize;
    desc.MissShaderTable.StrideInBytes = m_shaderRecordSize;
    desc.HitGroupTable.StartAddress = shaderTableAddress + m_shaderRecordSize * 2;
    desc.HitGroupTable.SizeInBytes = m_shaderRecordSize;
    desc.HitGroupTable.StrideInBytes = m_shaderRecordSize;
    desc.Width = _width;
    desc.Height = _height;
    desc.Depth = _depth;

    return desc;
}
