#include "Texture.h"

ego::gpu::Texture2D::Texture2D(const Texture2DDesc& _desc)
    : m_desc(_desc)
{
}

const ego::gpu::Texture2DDesc& ego::gpu::Texture2D::getDesc() const
{
    return m_desc;
}

ego::gpu::TextureDimension ego::gpu::Texture2D::getDimension() const
{
    return TextureDimension::D2;
}

ego::gpu::TextureView::TextureView(const TextureReference& _texture, const TextureViewDesc& _desc)
    : ResourceView(_texture),
      m_desc(_desc)
{
}

const ego::gpu::TextureViewDesc& ego::gpu::TextureView::getDesc() const
{
    return m_desc;
}

ego::gpu::GraphicResourceViewType ego::gpu::TextureView::getViewType() const
{
    return m_desc.m_type;
}
