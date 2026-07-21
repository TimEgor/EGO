#include "D3D12SwapChain.h"

#include <cstring>
#include <limits>

#include "EgoCore/Assert/Assert.h"

#include "../Common/D3D12Utils.h"
#include "../Common/D3D12Utils.h"
#include "D3D12Texture.h"

ego::gpu::d3d12::D3D12SwapChain::D3D12SwapChain(
    const SwapChainDesc& _desc,
    Microsoft::WRL::ComPtr<IDXGISwapChain3>&& _swapChain,
    std::vector<Texture2DReference>&& _targetTextures,
    const CommandQueueReference& _presentationQueue)
    : SwapChain(_desc),
      m_swapChain(std::move(_swapChain)),
      m_targetTextures(std::move(_targetTextures)),
      m_presentationQueue(_presentationQueue)
{
}

void* ego::gpu::d3d12::D3D12SwapChain::getNativeHandle() const
{
    return m_swapChain.Get();
}

void ego::gpu::d3d12::D3D12SwapChain::setName(const char* _name)
{
    if (!m_swapChain || !_name)
    {
        return;
    }

    const size_t nameLength = std::strlen(_name);
    EGO_ASSERT(nameLength <= std::numeric_limits<UINT>::max());
    m_swapChain->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(nameLength), _name);
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

bool ego::gpu::d3d12::D3D12SwapChain::resize(const Texture2DSize& _size)
{
    if (!m_swapChain || !m_presentationQueue || _size.m_x == 0 || _size.m_y == 0)
    {
        return false;
    }

    if (!m_targetTextures.empty())
    {
        const Texture2DSize& currentSize = m_targetTextures.front()->getDesc().m_size;
        if (currentSize.m_x == _size.m_x && currentSize.m_y == _size.m_y)
        {
            return true;
        }
    }

    const SwapChainDesc& desc = getDesc();
    const DXGI_FORMAT format = ToDXGIFormat(desc.m_format);
    if (desc.m_bufferCount == 0 || format == DXGI_FORMAT_UNKNOWN)
    {
        return false;
    }

    m_presentationQueue->waitIdle();
    m_targetTextures.clear();

    if (FAILED(m_swapChain->ResizeBuffers(desc.m_bufferCount, _size.m_x, _size.m_y, format, 0)))
    {
        return false;
    }

    return createTargetTextures(_size);
}

void ego::gpu::d3d12::D3D12SwapChain::present()
{
    if (m_swapChain)
    {
        m_swapChain->Present(1, 0);
    }
}

bool ego::gpu::d3d12::D3D12SwapChain::createTargetTextures(const Texture2DSize& _size)
{
    const SwapChainDesc& desc = getDesc();

    Texture2DDesc textureDesc;
    textureDesc.m_usage = static_cast<GraphicResourceUsage>(TextureUsageRenderTarget | GraphicResourceUsageTransferDst);
    textureDesc.m_size = _size;
    textureDesc.m_arrayLayers = 1;
    textureDesc.m_mipLevels = 1;
    textureDesc.m_samples.m_count = 1;
    textureDesc.m_samples.m_quality = 0;
    textureDesc.m_format = desc.m_format;

    std::vector<Texture2DReference> targetTextures;
    targetTextures.reserve(desc.m_bufferCount);
    for (uint32_t bufferIndex = 0; bufferIndex < desc.m_bufferCount; ++bufferIndex)
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        if (FAILED(m_swapChain->GetBuffer(bufferIndex, IID_PPV_ARGS(&resource))))
        {
            return false;
        }

        Texture2DReference targetTexture = new D3D12Texture2D(textureDesc, std::move(resource));
        targetTexture->setState(GraphicResourceState::Present);
        targetTextures.push_back(targetTexture);
    }

    m_targetTextures = std::move(targetTextures);
    return true;
}
