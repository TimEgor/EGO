#include "WindowGraphicPresenter.h"

#include "EgoCore/UtilsMacros.h"

ego::application::WindowGraphicPresenter::~WindowGraphicPresenter()
{
    release();
}

bool ego::application::WindowGraphicPresenter::init(
    GraphicDevice& _graphicDevice,
    const PresentationSurface& _surface,
    const gpu::SwapChainDesc& _swapChainDesc,
    const gpu::CommandQueueReference& _presentationQueue)
{
    m_swapChain = _graphicDevice.createSwapChain(_swapChainDesc, _surface, _presentationQueue);
    EGO_CHECK_RETURN_FALSE(m_swapChain);

    return true;
}

void ego::application::WindowGraphicPresenter::release()
{
    m_swapChain.reset();
    m_pendingSize = UInt32Vector2Zero;
}

ego::gpu::Texture2DReference ego::application::WindowGraphicPresenter::getTargetTexture()
{
    return m_swapChain ? m_swapChain->getTargetTexture() : gpu::Texture2DReference();
}

bool ego::application::WindowGraphicPresenter::prepare()
{
    if (!m_swapChain)
    {
        return false;
    }

    if (m_pendingSize.m_x == 0 || m_pendingSize.m_y == 0)
    {
        return true;
    }

    if (!m_swapChain->resize(m_pendingSize))
    {
        return false;
    }

    m_pendingSize = UInt32Vector2Zero;
    return true;
}

bool ego::application::WindowGraphicPresenter::resize(const gpu::Texture2DSize& _size)
{
    if (!m_swapChain || _size.m_x == 0 || _size.m_y == 0)
    {
        return false;
    }

    const gpu::Texture2DReference targetTexture = m_swapChain->getTargetTexture();
    if (targetTexture)
    {
        const gpu::Texture2DSize& currentSize = targetTexture->getDesc().m_size;
        if (currentSize.m_x == _size.m_x && currentSize.m_y == _size.m_y)
        {
            m_pendingSize = UInt32Vector2Zero;
            return true;
        }
    }

    m_pendingSize = _size;
    return true;
}

void ego::application::WindowGraphicPresenter::present()
{
    if (m_swapChain)
    {
        m_swapChain->present();
    }
}
