#include "D3D12GraphicDevice.h"

#include <algorithm>

#include "Common/D3D12Utils.h"
#include "Objects/D3D12Pipeline.h"
#include "Objects/D3D12Shader.h"

#include "EgoCore/Assert/AssertCore.h"

namespace
{
    bool IsValidShaderCode(const ego::gpu::ShaderCodePointer& _code)
    {
        return _code && _code->getCode() && _code->getCodeSize();
    }

    void FillDefaultRenderTargetBlend(D3D12_RENDER_TARGET_BLEND_DESC& _desc)
    {
        _desc.BlendEnable = FALSE;
        _desc.LogicOpEnable = FALSE;
        _desc.SrcBlend = D3D12_BLEND_ONE;
        _desc.DestBlend = D3D12_BLEND_ZERO;
        _desc.BlendOp = D3D12_BLEND_OP_ADD;
        _desc.SrcBlendAlpha = D3D12_BLEND_ONE;
        _desc.DestBlendAlpha = D3D12_BLEND_ZERO;
        _desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        _desc.LogicOp = D3D12_LOGIC_OP_NOOP;
        _desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }
}

ego::gpu::ShaderPointer ego::gpu::d3d12::D3D12GraphicDevice::createVertexShader(const ShaderCodePointer& _code)
{
    if (!IsValidShaderCode(_code))
    {
        return ShaderPointer();
    }

    return ShaderPointer(new D3D12VertexShader(_code));
}

ego::gpu::ShaderPointer ego::gpu::d3d12::D3D12GraphicDevice::createPixelShader(const ShaderCodePointer& _code)
{
    if (!IsValidShaderCode(_code))
    {
        return ShaderPointer();
    }

    return ShaderPointer(new D3D12PixelShader(_code));
}

ego::gpu::ShaderPointer ego::gpu::d3d12::D3D12GraphicDevice::createComputeShader(const ShaderCodePointer& _code)
{
    if (!IsValidShaderCode(_code))
    {
        return ShaderPointer();
    }

    return ShaderPointer(new D3D12ComputeShader(_code));
}

ego::gpu::GraphicPipelinePointer ego::gpu::d3d12::D3D12GraphicDevice::createGraphicPipeline(
    const GraphicPipelineDesc& _desc
)
{
    D3D12BindingLayout* layout = dynamic_cast<D3D12BindingLayout*>(_desc.m_bindingLayout.get());
    D3D12ShaderAccess* vertexShader = dynamic_cast<D3D12ShaderAccess*>(_desc.m_vertexShader.get());
    D3D12ShaderAccess* pixelShader = dynamic_cast<D3D12ShaderAccess*>(_desc.m_pixelShader.get());

    EGO_ASSERT_MESSAGE(!_desc.m_bindingLayout || layout, "BindingLayout must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(!_desc.m_vertexShader || vertexShader, "Vertex shader must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(!_desc.m_pixelShader || pixelShader, "Pixel shader must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(
        !_desc.m_vertexShader || _desc.m_vertexShader->getShaderType() == ShaderStage::Vertex,
        "Graphic pipeline vertex shader has invalid shader stage"
    );
    EGO_ASSERT_MESSAGE(
        !_desc.m_pixelShader || _desc.m_pixelShader->getShaderType() == ShaderStage::Pixel,
        "Graphic pipeline pixel shader has invalid shader stage"
    );

    if ((_desc.m_bindingLayout && !layout) ||
        (_desc.m_vertexShader && !vertexShader) ||
        (_desc.m_pixelShader && !pixelShader) ||
        (_desc.m_vertexShader && _desc.m_vertexShader->getShaderType() != ShaderStage::Vertex) ||
        (_desc.m_pixelShader && _desc.m_pixelShader->getShaderType() != ShaderStage::Pixel))
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
        FillDefaultRenderTargetBlend(renderTargetBlend);
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
    D3D12BindingLayout* layout = dynamic_cast<D3D12BindingLayout*>(_desc.m_bindingLayout.get());
    D3D12ShaderAccess* computeShader = dynamic_cast<D3D12ShaderAccess*>(_desc.m_computeShader.get());

    EGO_ASSERT_MESSAGE(!_desc.m_bindingLayout || layout, "BindingLayout must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(!_desc.m_computeShader || computeShader, "Compute shader must be created by D3D12 device");
    EGO_ASSERT_MESSAGE(
        !_desc.m_computeShader || _desc.m_computeShader->getShaderType() == ShaderStage::Compute,
        "Compute pipeline shader has invalid shader stage"
    );

    if ((_desc.m_bindingLayout && !layout) ||
        (_desc.m_computeShader && !computeShader) ||
        (_desc.m_computeShader && _desc.m_computeShader->getShaderType() != ShaderStage::Compute))
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
