#pragma once

#include "EgoCore/Platform/Surface/PlatformSurface.h"

#include "EgoGui/GuiLayer.h"
#include "EgoGui/Rendering/GuiTexture.h"

#include "EditorTitleBar.h"

namespace ego::editor
{
    class GuiLayer final : public gui::GuiLayer
    {
    public:
        void setSurface(const PlatformSurfacePointer& _surface);
        void setSceneTexture(const gpu::TextureViewPointer& _sceneTexture);
        void reset();

    private:
        void initializeDefaultLayout();
        void drawGui() override;

        PlatformSurfacePointer m_surface = nullptr;
        gpu::TextureViewPointer m_sceneTexture = nullptr;
        EditorTitleBar m_titleBar;
        bool m_showViewport = true;
        bool m_showSceneInspector = true;
        bool m_showEntityInspector = true;
        bool m_isDefaultLayoutInitialized = false;
    };
} // namespace ego::editor
