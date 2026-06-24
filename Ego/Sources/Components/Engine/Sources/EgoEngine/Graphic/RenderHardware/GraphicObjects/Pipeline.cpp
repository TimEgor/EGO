#include "Pipeline.h"

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
