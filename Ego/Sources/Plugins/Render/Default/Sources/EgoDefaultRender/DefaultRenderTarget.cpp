#include "DefaultRenderTarget.h"

#include "EgoCore/UtilsMacros.h"

bool ego::render::DefaultRenderTarget::prepare(GraphicDevice& _graphicDevice, const gpu::Texture2DSize& _resolution, gpu::GraphicResourceFormat _format)
{
    if (_resolution.m_x == 0 || _resolution.m_y == 0)
    {
        return false;
    }

    m_resolution = _resolution;

    const gpu::Texture2DDesc* currentDesc = m_texture ? &m_texture->getDesc() : nullptr;
    if (currentDesc && currentDesc->m_size.m_x == _resolution.m_x && currentDesc->m_size.m_y == _resolution.m_y && currentDesc->m_format == _format && m_renderTargetView &&
        m_unorderedAccessView)
    {
        return true;
    }

    gpu::Texture2DDesc textureDesc;
    textureDesc.m_usage =
        static_cast<gpu::GraphicResourceUsage>(gpu::TextureUsageRenderTarget | gpu::GraphicResourceUsageTransferSrc | gpu::GraphicResourceUsageAllowUnorderedAccess);
    textureDesc.m_size = m_resolution;
    textureDesc.m_arrayLayers = 1;
    textureDesc.m_mipLevels = 1;
    textureDesc.m_samples.m_count = 1;
    textureDesc.m_samples.m_quality = 0;
    textureDesc.m_format = _format;

    m_texture = _graphicDevice.createTexture2D(textureDesc);
    EGO_CHECK_RETURN_FALSE(m_texture);

    gpu::TextureViewDesc renderTargetViewDesc;
    renderTargetViewDesc.m_type = gpu::GraphicResourceViewType::RenderTarget;
    renderTargetViewDesc.m_dimension = gpu::TextureViewDimension::D2;
    renderTargetViewDesc.m_format = textureDesc.m_format;

    m_renderTargetView = _graphicDevice.createTextureView(m_texture.getObject(), renderTargetViewDesc);
    EGO_CHECK_RETURN_FALSE(m_renderTargetView);

    gpu::TextureViewDesc unorderedAccessViewDesc;
    unorderedAccessViewDesc.m_type = gpu::GraphicResourceViewType::UnorderedAccess;
    unorderedAccessViewDesc.m_dimension = gpu::TextureViewDimension::D2;
    unorderedAccessViewDesc.m_format = textureDesc.m_format;

    m_unorderedAccessView = _graphicDevice.createTextureView(m_texture.getObject(), unorderedAccessViewDesc);

    return static_cast<bool>(m_unorderedAccessView);
}

void ego::render::DefaultRenderTarget::release()
{
    m_unorderedAccessView = nullptr;
    m_renderTargetView = nullptr;
    m_texture = nullptr;
}

void ego::render::DefaultRenderTarget::transition(const RenderGraphicCommandList& _commandList, gpu::GraphicResourceState _nextState)
{
    if (!_commandList || !m_texture || m_texture->getState() == _nextState)
    {
        return;
    }

    _commandList->resourceBarrier(m_texture.getObject(), _nextState);
}

bool ego::render::DefaultRenderTarget::isReady() const
{
    return m_texture && m_renderTargetView;
}

const ego::render::RenderTexture2D& ego::render::DefaultRenderTarget::getTexture() const
{
    return m_texture;
}

const ego::render::RenderTextureView& ego::render::DefaultRenderTarget::getRenderTargetView() const
{
    return m_renderTargetView;
}

const ego::render::RenderTextureView& ego::render::DefaultRenderTarget::getUnorderedAccessView() const
{
    return m_unorderedAccessView;
}

const ego::gpu::Texture2DSize& ego::render::DefaultRenderTarget::getResolution() const
{
    return m_resolution;
}
