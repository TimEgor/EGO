#include "WindowGraphicPresenter.h"

#include "EgoCore/UtilsMacros.h"

ego::WindowGraphicPresenter::~WindowGraphicPresenter()
{
    release();
}

bool ego::WindowGraphicPresenter::init(
    GraphicDevice& _graphicDevice,
    const PresentationSurface& _surface,
    const gpu::SwapChainDesc& _swapChainDesc,
    const gpu::CommandQueueReference& _presentationQueue)
{
    m_swapChain = _graphicDevice.createSwapChain(_swapChainDesc, _surface, _presentationQueue);
    EGO_CHECK_RETURN_FALSE(m_swapChain);

    return true;
}

void ego::WindowGraphicPresenter::release()
{
    m_swapChain.reset();
}

ego::gpu::Texture2DReference ego::WindowGraphicPresenter::getTargetTexture()
{
    return m_swapChain ? m_swapChain->getTargetTexture() : gpu::Texture2DReference();
}

bool ego::WindowGraphicPresenter::resize(const gpu::Texture2DSize& _size)
{
    return m_swapChain && m_swapChain->resize(_size);
}

void ego::WindowGraphicPresenter::present()
{
    if (m_swapChain)
    {
        m_swapChain->present();
    }
}
