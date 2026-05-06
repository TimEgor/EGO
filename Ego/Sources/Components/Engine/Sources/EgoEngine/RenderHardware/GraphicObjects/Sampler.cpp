#include "Sampler.h"

ego::gpu::Sampler::Sampler(const SamplerDesc& _desc)
    : m_desc(_desc)
{
}

const ego::gpu::SamplerDesc& ego::gpu::Sampler::getDesc() const
{
    return m_desc;
}

uint32_t ego::gpu::Sampler::getBindlessIndex() const
{
    return InvalidBindlessIndex;
}
