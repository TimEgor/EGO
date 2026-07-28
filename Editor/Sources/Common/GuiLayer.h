#pragma once

#include "EgoGui/GuiLayer.h"
#include "EgoGui/Rendering/GuiTexture.h"

namespace ego::editor
{
    class GuiLayer final : public gui::GuiLayer
    {
    public:
        void setSceneTexture(const gpu::TextureViewReference& _sceneTexture);
        void reset();

    private:
        void initializeDefaultLayout();
        void drawGui() override;

        gpu::TextureViewReference m_sceneTexture = nullptr;
        bool m_showSceneWindow = true;
        bool m_showHierarchyWindow = true;
        bool m_showInspectorWindow = true;
        bool m_showConsoleWindow = true;
        bool m_isDefaultLayoutInitialized = false;
    };
} // namespace ego::editor
