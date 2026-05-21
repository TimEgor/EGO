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

        const uint8_t* codeBegin = static_cast<const uint8_t*>(_code->getCode());
        return std::vector<uint8_t>(codeBegin, codeBegin + _code->getCodeSize());
    }
}

ego::gpu::d3d12::D3D12BindingLayout::D3D12BindingLayout(
    const BindingLayoutDesc& _desc,
    Microsoft::WRL::ComPtr<ID3D12RootSignature>&& _rootSignature,
    std::vector<PushConstantInfo>&& _pushConstants
)
    : BindingLayout(_desc),
      m_rootSignature(std::move(_rootSignature)),
      m_pushConstants(std::move(_pushConstants))
{}

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
    uint32_t _size
) const
{
    for (const PushConstantInfo& pushConstant : m_pushConstants)
    {
        const bool stageMatch = (pushConstant.m_stageFlags & _stageFlags) == _stageFlags || pushConstant.m_stageFlags ==
            ShaderStageFlagAll;
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
    const D3D12BindingLayout* _layout
)
    : GraphicPipeline(_desc),
      m_pipelineState(std::move(_pipelineState)),
      m_layout(_layout),
      m_primitiveTopology(ToD3DPrimitiveTopology(_desc.m_topology))
{}

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
    const D3D12BindingLayout* _layout
)
    : ComputePipeline(_desc),
      m_pipelineState(std::move(_pipelineState)),
      m_layout(_layout)
{}

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
