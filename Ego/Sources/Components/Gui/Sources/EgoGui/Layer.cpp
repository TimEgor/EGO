#include "EgoGui/Layer.h"

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Backend.h"
#include "EgoGui/ContextScope.h"

ego::gui::GuiFrameTextureID ego::gui::Layer::bindTexture(const gpu::TextureViewPointer& _textureView, TextureSamplingMode _samplingMode) const
{
    if (!m_activeBackend)
    {
        return InvalidGuiFrameTextureID;
    }

    return m_activeBackend->bindTexture(_textureView, _samplingMode);
}

bool ego::gui::Layer::draw(Backend& _backend)
{
    EGO_ASSERT(!m_activeBackend);
    if (m_activeBackend)
    {
        return false;
    }

    const ContextScope contextScope(_backend.getContext());

    m_activeBackend = &_backend;
    drawGui();
    m_activeBackend = nullptr;

    return true;
}
