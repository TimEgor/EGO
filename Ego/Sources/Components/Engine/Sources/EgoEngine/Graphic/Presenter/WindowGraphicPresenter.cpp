#include "WindowGraphicPresenter.h"

#include "EgoCore/UtilsMacros.h"

ego::WindowGraphicPresenter::~WindowGraphicPresenter()
{
    release();
}

bool ego::WindowGraphicPresenter::init(
    GraphicDevice& _graphicDevice,
    const Window& _window,
    const gpu::SwapChainDesc& _swapChainDesc,
    const gpu::CommandQueueReference& _presentationQueue)
{
    m_swapChain = _graphicDevice.createSwapChain(_swapChainDesc, _window, _presentationQueue);
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

void ego::WindowGraphicPresenter::present()
{
    if (m_swapChain)
    {
        m_swapChain->present();
    }
}
