#include "SwapChain.h"

ego::gpu::SwapChain::SwapChain(const SwapChainDesc& _desc)
    : m_desc(_desc)
{
}

const ego::gpu::SwapChainDesc& ego::gpu::SwapChain::getDesc() const
{
    return m_desc;
}
