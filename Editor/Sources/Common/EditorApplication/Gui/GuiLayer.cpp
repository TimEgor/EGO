#include "GuiLayer.h"

void ego::editor::GuiLayer::setSurface(const PlatformSurfacePointer& _surface)
{
    m_surface = _surface;
}

void ego::editor::GuiLayer::setSceneTexture(const gpu::TextureViewPointer& _sceneTexture)
{
    m_sceneTexture = _sceneTexture;
}

void ego::editor::GuiLayer::reset()
{
    m_surface = nullptr;
    m_sceneTexture = nullptr;
    m_windowController.reset();
}

void ego::editor::GuiLayer::drawGui()
{
    if (m_surface)
    {
        m_titleBar.draw(*m_surface, m_windowController);
    }

    gui::GuiFrameTextureID sceneTextureID = gui::InvalidGuiFrameTextureID;
    if (m_windowController.isViewportVisible() && m_sceneTexture)
    {
        sceneTextureID = bindTexture(m_sceneTexture);
    }

    m_windowController.drawWindows(sceneTextureID);
}
