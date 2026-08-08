#include "FrameTextures.h"

#include <cstddef>

#include "EgoCore/Assert/Assert.h"

void ego::gui::FrameTextures::reset()
{
    m_textures.clear();
}

ego::gui::GuiFrameTextureID ego::gui::FrameTextures::bind(const gpu::TextureViewPointer& _textureView, TextureSamplingMode _samplingMode)
{
    EGO_ASSERT(IsTextureViewValid(_textureView));
    if (!IsTextureViewValid(_textureView))
    {
        return InvalidGuiFrameTextureID;
    }

    for (size_t textureIndex = 0; textureIndex < m_textures.size(); ++textureIndex)
    {
        const Texture& texture = m_textures[textureIndex];
        if (texture.m_textureView.getObject() == _textureView.getObject() && texture.m_samplingMode == _samplingMode)
        {
            return static_cast<GuiFrameTextureID>(textureIndex) + 1;
        }
    }

    m_textures.push_back({.m_textureView = _textureView, .m_samplingMode = _samplingMode});

    return static_cast<GuiFrameTextureID>(m_textures.size());
}

bool ego::gui::FrameTextures::resolve(GuiFrameTextureID _textureID, Texture& _texture) const
{
    if (_textureID == InvalidGuiFrameTextureID || _textureID > static_cast<GuiFrameTextureID>(m_textures.size()))
    {
        _texture = Texture();

        return false;
    }

    _texture = m_textures[static_cast<size_t>(_textureID - 1)];

    return true;
}

bool ego::gui::FrameTextures::IsTextureViewValid(const gpu::TextureViewPointer& _textureView)
{
    return _textureView && _textureView->getViewType() == gpu::GraphicResourceViewType::ShaderResource &&
           _textureView->getDesc().m_dimension == gpu::TextureViewDimension::D2 && _textureView->getBindlessIndex() != gpu::InvalidBindlessIndex;
}
