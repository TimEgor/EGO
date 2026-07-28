#include "RenderPipelineStateCache.h"

#include <utility>

#include "EgoCore/Hash/HashCombine.h"

#include "EgoGraphicHardware/GraphicDevice.h"

uint64_t ego::render::RenderPipelineStateCache::HashRasterizationState(const gpu::RasterizationStateDesc& _desc)
{
    return ego::HashValues(
        0,
        _desc.m_fillMode,
        _desc.m_cullMode,
        _desc.m_frontCounterClockwise,
        _desc.m_depthClip,
        _desc.m_depthBiasEnable,
        _desc.m_depthBias,
        _desc.m_depthBiasClamp,
        _desc.m_depthBiasSlopeScale);
}

uint64_t ego::render::RenderPipelineStateCache::HashDepthStencilState(const gpu::DepthStencilStateDesc& _desc)
{
    return ego::HashValues(0, _desc.m_depthTestEnable, _desc.m_depthWrite, _desc.m_depthCompareOperation, _desc.m_stencilEnable);
}

uint64_t ego::render::RenderPipelineStateCache::HashRenderTargetBlend(const gpu::RenderTargetBlendDesc& _desc)
{
    return ego::HashValues(
        0,
        _desc.m_blendEnable,
        _desc.m_srcColorFactor,
        _desc.m_dstColorFactor,
        _desc.m_colorOperation,
        _desc.m_srcAlphaFactor,
        _desc.m_dstAlphaFactor,
        _desc.m_alphaOperation,
        _desc.m_colorWriteMask);
}

uint64_t ego::render::RenderPipelineStateCache::HashBlendState(const gpu::BlendStateDesc& _desc)
{
    uint64_t hash = ego::HashValues(0, _desc.m_alphaToCoverageEnable, _desc.m_renderTargets.size());
    for (const ego::gpu::RenderTargetBlendDesc& target : _desc.m_renderTargets)
    {
        hash = ego::HashCombine(hash, HashRenderTargetBlend(target));
    }

    return hash;
}

uint64_t ego::render::RenderPipelineStateCache::HashGraphicPipelineDesc(const gpu::GraphicPipelineDesc& _desc)
{
    uint64_t hash = ego::HashValues(
        0,
        IntrusivePointerIdentityHash{}(_desc.m_bindingLayout),
        IntrusivePointerIdentityHash{}(_desc.m_vertexShader),
        IntrusivePointerIdentityHash{}(_desc.m_pixelShader),
        _desc.m_inputLayoutDesc.getHash(),
        HashRasterizationState(_desc.m_rasterizationStateDesc),
        HashDepthStencilState(_desc.m_depthStencilStateDesc),
        HashBlendState(_desc.m_blendStateDesc),
        _desc.m_multisampleCount,
        _desc.m_topology,
        _desc.m_colorFormats.size());

    for (ego::gpu::GraphicResourceFormat format : _desc.m_colorFormats)
    {
        hash = ego::HashCombine(hash, format);
    }

    return ego::HashCombine(hash, _desc.m_depthFormat);
}

uint64_t ego::render::RenderPipelineStateCache::HashRayTracingPipelineDesc(const gpu::RayTracingPipelineDesc& _desc)
{
    uint64_t hash = ego::HashValues(
        0,
        IntrusivePointerIdentityHash{}(_desc.m_bindingLayout),
        IntrusivePointerIdentityHash{}(_desc.m_rayGenerationShader),
        IntrusivePointerIdentityHash{}(_desc.m_missShader),
        _desc.m_hitGroups.size(),
        _desc.m_maxPayloadSize,
        _desc.m_maxAttributeSize,
        _desc.m_maxRecursionDepth);

    for (const gpu::RayTracingHitGroupDesc& hitGroup : _desc.m_hitGroups)
    {
        hash = ego::HashValues(
            hash,
            hitGroup.m_type,
            IntrusivePointerIdentityHash{}(hitGroup.m_closestHitShader),
            IntrusivePointerIdentityHash{}(hitGroup.m_anyHitShader),
            IntrusivePointerIdentityHash{}(hitGroup.m_intersectionShader));
    }

    return hash;
}

bool ego::render::RenderPipelineStateCache::IsPipelineHandlerUsedOnlyByCache(const RenderGraphicPipeline& _pipeline)
{
    const RenderGraphicPipeline::SourcePointer& source = _pipeline.getSource();
    return !source || source->getReferenceCount() <= 1;
}

bool ego::render::RenderPipelineStateCache::IsPipelineHandlerUsedOnlyByCache(const RenderRayTracingPipeline& _pipeline)
{
    const RenderRayTracingPipeline::SourcePointer& source = _pipeline.getSource();
    return !source || source->getReferenceCount() <= 1;
}

ego::render::RenderGraphicPipeline ego::render::RenderPipelineStateCache::getOrCreateGraphicPipeline(GraphicDevice& _graphicDevice, const gpu::GraphicPipelineDesc& _desc)
{
    const uint64_t hash = HashGraphicPipelineDesc(_desc);
    std::pair<GraphicPipelineMap::iterator, GraphicPipelineMap::iterator> pipelineRange = m_graphicPipelines.equal_range(hash);
    for (GraphicPipelineMap::iterator pipelineIter = pipelineRange.first; pipelineIter != pipelineRange.second; ++pipelineIter)
    {
        GraphicPipelineEntry& entry = pipelineIter->second;
        if (entry.m_desc == _desc)
        {
            return entry.m_pipeline;
        }
    }

    const RenderGraphicPipeline pipeline = _graphicDevice.createGraphicPipeline(_desc);
    if (!pipeline)
    {
        return nullptr;
    }

    GraphicPipelineEntry entry;
    entry.m_desc = _desc;
    entry.m_pipeline = pipeline;

    GraphicPipelineMap::iterator pipelineIter = m_graphicPipelines.emplace(hash, entry);
    return pipelineIter->second.m_pipeline;
}

ego::render::RenderRayTracingPipeline ego::render::RenderPipelineStateCache::getOrCreateRayTracingPipeline(
    GraphicDevice& _graphicDevice,
    const gpu::RayTracingPipelineDesc& _desc)
{
    const uint64_t hash = HashRayTracingPipelineDesc(_desc);
    std::pair<RayTracingPipelineMap::iterator, RayTracingPipelineMap::iterator> pipelineRange = m_rayTracingPipelines.equal_range(hash);
    for (RayTracingPipelineMap::iterator pipelineIter = pipelineRange.first; pipelineIter != pipelineRange.second; ++pipelineIter)
    {
        RayTracingPipelineEntry& entry = pipelineIter->second;
        if (entry.m_desc == _desc)
        {
            return entry.m_pipeline;
        }
    }

    const RenderRayTracingPipeline pipeline = _graphicDevice.createRayTracingPipeline(_desc);
    if (!pipeline)
    {
        return nullptr;
    }

    RayTracingPipelineEntry entry;
    entry.m_desc = _desc;
    entry.m_pipeline = pipeline;

    RayTracingPipelineMap::iterator pipelineIter = m_rayTracingPipelines.emplace(hash, entry);
    return pipelineIter->second.m_pipeline;
}

void ego::render::RenderPipelineStateCache::releaseUnused()
{
    for (GraphicPipelineMap::iterator pipelineIter = m_graphicPipelines.begin(); pipelineIter != m_graphicPipelines.end();)
    {
        if (IsPipelineHandlerUsedOnlyByCache(pipelineIter->second.m_pipeline))
        {
            pipelineIter = m_graphicPipelines.erase(pipelineIter);
        }
        else
        {
            ++pipelineIter;
        }
    }

    for (RayTracingPipelineMap::iterator pipelineIter = m_rayTracingPipelines.begin(); pipelineIter != m_rayTracingPipelines.end();)
    {
        if (IsPipelineHandlerUsedOnlyByCache(pipelineIter->second.m_pipeline))
        {
            pipelineIter = m_rayTracingPipelines.erase(pipelineIter);
        }
        else
        {
            ++pipelineIter;
        }
    }
}

void ego::render::RenderPipelineStateCache::clear()
{
    m_rayTracingPipelines.clear();
    m_graphicPipelines.clear();
}

size_t ego::render::RenderPipelineStateCache::getGraphicPipelineCount() const
{
    return m_graphicPipelines.size();
}

size_t ego::render::RenderPipelineStateCache::getRayTracingPipelineCount() const
{
    return m_rayTracingPipelines.size();
}
