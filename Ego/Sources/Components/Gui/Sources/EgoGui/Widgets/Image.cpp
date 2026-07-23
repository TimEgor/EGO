#include "Image.h"

#include <algorithm>

#include "EgoCore/RTTI/RTTI.h"

ego::gui::ImagePointer ego::gui::Image::Create()
{
    return new Image();
}

ego::gui::ImagePointer ego::gui::Image::Create(const gpu::TextureViewReference& _textureView)
{
    const ImagePointer image = Create();
    image->setTextureView(_textureView);
    return image;
}

void ego::gui::Image::setTextureView(const gpu::TextureViewReference& _textureView)
{
    if (m_textureView.getObject() == _textureView.getObject())
    {
        return;
    }

    m_textureView = _textureView;
    invalidateLayout();
}

const ego::gpu::TextureViewReference& ego::gui::Image::getTextureView() const
{
    return m_textureView;
}

ego::gui::Size ego::gui::Image::calculatePreferredSize(const LayoutContext&, const LayoutConstraints& _constraints)
{
    const gpu::GraphicResourceReference textureResource = m_textureView ? m_textureView->getResource() : nullptr;
    if (!textureResource || !rtti::IsObjectBasedOn<gpu::Texture2D>(*textureResource))
    {
        return SizeZero;
    }

    const gpu::Texture2DReference texture = textureResource.getObjectCast<gpu::Texture2D>();
    const gpu::Texture2DSize& textureSize = texture->getDesc().m_size;
    return Size(
        (std::min)(static_cast<float>(textureSize.m_x), _constraints.m_maximumSize.m_x),
        (std::min)(static_cast<float>(textureSize.m_y), _constraints.m_maximumSize.m_y));
}

void ego::gui::Image::drawBaseLayer(PaintContext& _context) const
{
    _context.drawTexture(getLayoutBounds(), m_textureView);
}
