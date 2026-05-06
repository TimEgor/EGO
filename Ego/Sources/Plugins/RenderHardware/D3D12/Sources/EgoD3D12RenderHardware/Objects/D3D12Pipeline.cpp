#include "D3D12Pipeline.h"

#include "../Common/D3D12Utils.h"

namespace
{
    std::vector<uint8_t> CopyShaderCode(const ego::gpu::ShaderCodePointer& _code)
    {
        if (!_code || !_code->getCode() || !_code->getCodeSize())
        {
            return {};
        }

        const uint8_t* codeBegin = static_cast<const uint8_t*>(_code->getCode());
        return std::vector<uint8_t>(codeBegin, codeBegin + _code->getCodeSize());
    }
}

uint32_t ego::gpu::d3d12::D3D12BindingLayout::SetInfo::getResourceDescriptorCount() const
{
    uint32_t result = 0;
    for (const DescriptorRangeInfo& rangeInfo : m_resourceRanges)
    {
        result += rangeInfo.m_descriptorCount;
    }

    return result;
}

uint32_t ego::gpu::d3d12::D3D12BindingLayout::SetInfo::getSamplerDescriptorCount() const
{
    uint32_t result = 0;
    for (const DescriptorRangeInfo& rangeInfo : m_samplerRanges)
    {
        result += rangeInfo.m_descriptorCount;
    }

    return result;
}

ego::gpu::d3d12::D3D12BindingLayout::D3D12BindingLayout(
    const BindingLayoutDesc& _desc,
    Microsoft::WRL::ComPtr<ID3D12RootSignature>&& _rootSignature,
    std::vector<SetInfo>&& _setInfos,
    std::vector<PushConstantInfo>&& _pushConstants
)
    : BindingLayout(_desc),
      m_rootSignature(std::move(_rootSignature)),
      m_setInfos(std::move(_setInfos)),
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

const ego::gpu::d3d12::D3D12BindingLayout::SetInfo* ego::gpu::d3d12::D3D12BindingLayout::findSetInfo(
    uint32_t _set
) const
{
    for (const SetInfo& setInfo : m_setInfos)
    {
        if (setInfo.m_set == _set)
        {
            return &setInfo;
        }
    }

    return nullptr;
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

ego::gpu::d3d12::D3D12BindingSet::D3D12BindingSet(
    const BindingSetDesc& _desc,
    const D3D12BindingLayout* _layout,
    const D3D12BindingLayout::SetInfo* _setInfo
)
    : BindingSet(_desc),
      m_layout(_layout),
      m_setInfo(_setInfo)
{}

ego::gpu::d3d12::D3D12BindingSet::~D3D12BindingSet()
{}

void* ego::gpu::d3d12::D3D12BindingSet::getNativeHandle() const
{
    return nullptr;
}

void ego::gpu::d3d12::D3D12BindingSet::setName(const char* _name)
{}

const ego::gpu::d3d12::D3D12BindingLayout* ego::gpu::d3d12::D3D12BindingSet::getD3D12Layout() const
{
    return m_layout;
}

const ego::gpu::d3d12::D3D12BindingLayout::SetInfo* ego::gpu::d3d12::D3D12BindingSet::getSetInfo() const
{
    return m_setInfo;
}

bool ego::gpu::d3d12::D3D12BindingSet::hasResourceDescriptors() const
{
    return m_setInfo && m_setInfo->getResourceDescriptorCount() != 0;
}

bool ego::gpu::d3d12::D3D12BindingSet::hasSamplerDescriptors() const
{
    return m_setInfo && m_setInfo->getSamplerDescriptorCount() != 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE ego::gpu::d3d12::D3D12BindingSet::getResourceGpuHandle() const
{
    return D3D12_GPU_DESCRIPTOR_HANDLE{};
}

D3D12_GPU_DESCRIPTOR_HANDLE ego::gpu::d3d12::D3D12BindingSet::getSamplerGpuHandle() const
{
    return D3D12_GPU_DESCRIPTOR_HANDLE{};
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
