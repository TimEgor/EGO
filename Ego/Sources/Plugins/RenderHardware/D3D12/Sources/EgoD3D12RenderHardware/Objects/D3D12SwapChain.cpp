#include "D3D12SwapChain.h"

#include "../Common/D3D12Utils.h"

ego::gpu::d3d12::D3D12SwapChain::D3D12SwapChain(
    const SwapChainDesc& _desc,
    Microsoft::WRL::ComPtr<IDXGISwapChain3>&& _swapChain,
    std::vector<Texture2DReference>&& _targetTextures,
    CommandQueueReference&& _ownedPresentationQueue
)
    : SwapChain(_desc),
      m_swapChain(std::move(_swapChain)),
      m_targetTextures(std::move(_targetTextures)),
      m_ownedPresentationQueue(std::move(_ownedPresentationQueue))
{}

void* ego::gpu::d3d12::D3D12SwapChain::getNativeHandle() const
{
    return m_swapChain.Get();
}

void ego::gpu::d3d12::D3D12SwapChain::setName(const char* _name)
{
    m_swapChain->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(_name) - 1, _name);
}

ego::gpu::Texture2DReference ego::gpu::d3d12::D3D12SwapChain::getTargetTexture()
{
    if (!m_swapChain || m_targetTextures.empty())
    {
        return Texture2DReference();
    }

    const UINT bufferIndex = m_swapChain->GetCurrentBackBufferIndex();
    if (bufferIndex >= m_targetTextures.size())
    {
        return Texture2DReference();
    }

    return m_targetTextures[bufferIndex];
}

void ego::gpu::d3d12::D3D12SwapChain::present()
{
    if (m_swapChain)
    {
        m_swapChain->Present(1, 0);
    }
}
