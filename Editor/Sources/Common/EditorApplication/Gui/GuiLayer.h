#pragma once

#include "EgoCore/Platform/Surface/PlatformSurface.h"

#include "EgoGui/GuiLayer.h"
#include "EgoGui/Rendering/GuiTexture.h"

#include "EditorTitleBar.h"
#include "GuiWindowController.h"

namespace ego::editor
{
    class GuiLayer final : public gui::GuiLayer
    {
    public:
        void setSurface(const PlatformSurfacePointer& _surface);
        void setSceneTexture(const gpu::TextureViewPointer& _sceneTexture);
        void reset();

    private:
        void drawGui() override;

        EditorTitleBar m_titleBar;
        GuiWindowController m_windowController;

        PlatformSurfacePointer m_surface = nullptr;
        gpu::TextureViewPointer m_sceneTexture = nullptr;
    };
} // namespace ego::editor
