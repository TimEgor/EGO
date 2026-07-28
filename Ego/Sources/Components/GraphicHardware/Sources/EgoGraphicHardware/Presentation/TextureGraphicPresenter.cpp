#include "TextureGraphicPresenter.h"

#include "EgoCore/UtilsMacros.h"

ego::TextureGraphicPresenter::~TextureGraphicPresenter()
{
    release();
}

bool ego::TextureGraphicPresenter::init(GraphicDevice& _graphicDevice, const gpu::Texture2DSize& _size, gpu::GraphicResourceFormat _format)
{
    release();

    EGO_CHECK_INITIALIZATION(_size.m_x > 0 && _size.m_y > 0);
    EGO_CHECK_INITIALIZATION(_format != gpu::GraphicResourceFormat::Undefined);

    gpu::Texture2DDesc textureDesc;
    textureDesc.m_usage =
        static_cast<gpu::GraphicResourceUsage>(gpu::TextureUsageRenderTarget | gpu::GraphicResourceUsageTransferDst | gpu::GraphicResourceUsageShaderResource);
    textureDesc.m_access = static_cast<gpu::CommonGraphicResourceAccess>(gpu::GraphicResourceAccessGpuRead | gpu::GraphicResourceAccessGpuWrite);
    textureDesc.m_size = _size;
    textureDesc.m_format = _format;
    m_targetTexture = _graphicDevice.createTexture2D(textureDesc);
    EGO_CHECK_INITIALIZATION(m_targetTexture);

    gpu::TextureViewDesc textureViewDesc;
    textureViewDesc.m_type = gpu::GraphicResourceViewType::ShaderResource;
    textureViewDesc.m_dimension = gpu::TextureViewDimension::D2;
    textureViewDesc.m_format = _format;
    m_textureView = _graphicDevice.createTextureView(m_targetTexture, textureViewDesc);
    if (!m_textureView)
    {
        release();
        return false;
    }

    return true;
}

void ego::TextureGraphicPresenter::release()
{
    m_textureView = nullptr;
    m_targetTexture = nullptr;
}

bool ego::TextureGraphicPresenter::prepare()
{
    return m_targetTexture && m_textureView;
}

ego::gpu::Texture2DPointer ego::TextureGraphicPresenter::getTargetTexture()
{
    return m_targetTexture;
}

bool ego::TextureGraphicPresenter::shouldClearTarget() const
{
    return false;
}

ego::gpu::GraphicResourceState ego::TextureGraphicPresenter::getPresentationState() const
{
    return gpu::GraphicResourceState::ShaderRead;
}

const ego::gpu::TextureViewPointer& ego::TextureGraphicPresenter::getTextureView() const
{
    return m_textureView;
}

void ego::TextureGraphicPresenter::present() {}
