#include "EgoGui/GuiLayer.h"

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Implementation/GuiBackend.h"

ego::gui::GuiFrameTextureID ego::gui::GuiLayer::bindTexture(const gpu::TextureViewReference& _textureView, TextureSamplingMode _samplingMode) const
{
    if (!m_activeBackend)
    {
        return InvalidGuiFrameTextureID;
    }

    return m_activeBackend->bindTexture(_textureView, _samplingMode);
}

bool ego::gui::GuiLayer::draw(GuiBackend& _backend)
{
    EGO_ASSERT(!m_activeBackend);
    if (m_activeBackend)
    {
        return false;
    }

    m_activeBackend = &_backend;
    drawGui();
    m_activeBackend = nullptr;

    return true;
}
