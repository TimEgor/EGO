#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGui/Rendering/GuiTexture.h"

namespace ego::editor
{
    class GuiWindowController final : public NonCopyable
    {
    public:
        void reset();

        float drawWindowMenu();
        void drawWindows(gui::GuiFrameTextureID _sceneTextureID);

        bool isViewportVisible() const;

    private:
        void drawDockSpace();
        void drawViewport(gui::GuiFrameTextureID _sceneTextureID);
        void drawSceneInspector();
        void drawEntityInspector();

        bool m_isViewportVisible = true;
        bool m_isSceneInspectorVisible = true;
        bool m_isEntityInspectorVisible = true;
    };
} // namespace ego::editor
