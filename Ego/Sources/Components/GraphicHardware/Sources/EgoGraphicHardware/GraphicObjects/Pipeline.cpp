#include "Pipeline.h"

bool ego::gpu::RasterizationStateDesc::operator==(const RasterizationStateDesc& _other) const
{
    return m_fillMode == _other.m_fillMode && m_cullMode == _other.m_cullMode && m_frontCounterClockwise == _other.m_frontCounterClockwise &&
           m_depthClip == _other.m_depthClip && m_depthBiasEnable == _other.m_depthBiasEnable && m_depthBias == _other.m_depthBias &&
           m_depthBiasClamp == _other.m_depthBiasClamp && m_depthBiasSlopeScale == _other.m_depthBiasSlopeScale;
}

bool ego::gpu::RasterizationStateDesc::operator!=(const RasterizationStateDesc& _other) const
{
    return !(*this == _other);
}

bool ego::gpu::DepthStencilStateDesc::operator==(const DepthStencilStateDesc& _other) const
{
    return m_depthTestEnable == _other.m_depthTestEnable && m_depthWrite == _other.m_depthWrite && m_depthCompareOperation == _other.m_depthCompareOperation &&
           m_stencilEnable == _other.m_stencilEnable;
}

bool ego::gpu::DepthStencilStateDesc::operator!=(const DepthStencilStateDesc& _other) const
{
    return !(*this == _other);
}

bool ego::gpu::RenderTargetBlendDesc::operator==(const RenderTargetBlendDesc& _other) const
{
    return m_blendEnable == _other.m_blendEnable && m_srcColorFactor == _other.m_srcColorFactor && m_dstColorFactor == _other.m_dstColorFactor &&
           m_colorOperation == _other.m_colorOperation && m_srcAlphaFactor == _other.m_srcAlphaFactor && m_dstAlphaFactor == _other.m_dstAlphaFactor &&
           m_alphaOperation == _other.m_alphaOperation && m_colorWriteMask == _other.m_colorWriteMask;
}

bool ego::gpu::RenderTargetBlendDesc::operator!=(const RenderTargetBlendDesc& _other) const
{
    return !(*this == _other);
}

bool ego::gpu::BlendStateDesc::operator==(const BlendStateDesc& _other) const
{
    return m_alphaToCoverageEnable == _other.m_alphaToCoverageEnable && m_renderTargets == _other.m_renderTargets;
}

bool ego::gpu::BlendStateDesc::operator!=(const BlendStateDesc& _other) const
{
    return !(*this == _other);
}

bool ego::gpu::GraphicPipelineDesc::operator==(const GraphicPipelineDesc& _other) const
{
    return m_bindingLayout == _other.m_bindingLayout && m_vertexShader == _other.m_vertexShader && m_pixelShader == _other.m_pixelShader &&
           m_inputLayoutDesc == _other.m_inputLayoutDesc && m_rasterizationStateDesc == _other.m_rasterizationStateDesc &&
           m_depthStencilStateDesc == _other.m_depthStencilStateDesc && m_blendStateDesc == _other.m_blendStateDesc &&
           m_multisampleCount == _other.m_multisampleCount && m_topology == _other.m_topology && m_colorFormats == _other.m_colorFormats &&
           m_depthFormat == _other.m_depthFormat;
}

bool ego::gpu::GraphicPipelineDesc::operator!=(const GraphicPipelineDesc& _other) const
{
    return !(*this == _other);
}

bool ego::gpu::ComputePipelineDesc::operator==(const ComputePipelineDesc& _other) const
{
    return m_bindingLayout == _other.m_bindingLayout && m_computeShader == _other.m_computeShader;
}

bool ego::gpu::ComputePipelineDesc::operator!=(const ComputePipelineDesc& _other) const
{
    return !(*this == _other);
}

bool ego::gpu::RayTracingHitGroupDesc::operator==(const RayTracingHitGroupDesc& _other) const
{
    return m_type == _other.m_type && m_closestHitShader == _other.m_closestHitShader && m_anyHitShader == _other.m_anyHitShader &&
           m_intersectionShader == _other.m_intersectionShader;
}

bool ego::gpu::RayTracingHitGroupDesc::operator!=(const RayTracingHitGroupDesc& _other) const
{
    return !(*this == _other);
}

bool ego::gpu::RayTracingPipelineDesc::operator==(const RayTracingPipelineDesc& _other) const
{
    return m_bindingLayout == _other.m_bindingLayout && m_rayGenerationShader == _other.m_rayGenerationShader && m_missShader == _other.m_missShader &&
           m_hitGroups == _other.m_hitGroups && m_maxPayloadSize == _other.m_maxPayloadSize && m_maxAttributeSize == _other.m_maxAttributeSize &&
           m_maxRecursionDepth == _other.m_maxRecursionDepth;
}

bool ego::gpu::RayTracingPipelineDesc::operator!=(const RayTracingPipelineDesc& _other) const
{
    return !(*this == _other);
}

ego::gpu::GraphicPipeline::GraphicPipeline(const GraphicPipelineDesc& _desc)
    : m_desc(_desc)
{
}

const ego::gpu::GraphicPipelineDesc& ego::gpu::GraphicPipeline::getDesc() const
{
    return m_desc;
}

ego::gpu::PipelineType ego::gpu::GraphicPipeline::getPipelineType() const
{
    return PipelineType::Graphic;
}

ego::gpu::ComputePipeline::ComputePipeline(const ComputePipelineDesc& _desc)
    : m_desc(_desc)
{
}

const ego::gpu::ComputePipelineDesc& ego::gpu::ComputePipeline::getDesc() const
{
    return m_desc;
}

ego::gpu::PipelineType ego::gpu::ComputePipeline::getPipelineType() const
{
    return PipelineType::Compute;
}

ego::gpu::RayTracingPipeline::RayTracingPipeline(const RayTracingPipelineDesc& _desc)
    : m_desc(_desc)
{
}

const ego::gpu::RayTracingPipelineDesc& ego::gpu::RayTracingPipeline::getDesc() const
{
    return m_desc;
}

ego::gpu::PipelineType ego::gpu::RayTracingPipeline::getPipelineType() const
{
    return PipelineType::RayTracing;
}
