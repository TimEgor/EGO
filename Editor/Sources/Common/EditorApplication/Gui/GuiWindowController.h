#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGui/Rendering/GuiTexture.h"

#include "Menu/WindowMenuLayer.h"

namespace ego::editor
{
    class GuiWindowController final : public NonCopyable
    {
    public:
        GuiWindowController() = default;

        bool init();
        void release();

        void drawWindows(gui::GuiFrameTextureID _sceneTextureID);

        bool isViewportVisible() const;
        void setViewportVisible(bool _isVisible);
        bool isSceneInspectorVisible() const;
        void setSceneInspectorVisible(bool _isVisible);
        bool isEntityInspectorVisible() const;
        void setEntityInspectorVisible(bool _isVisible);

    private:
        void drawDockSpace();
        void drawViewport(gui::GuiFrameTextureID _sceneTextureID);
        void drawSceneInspector();
        void drawEntityInspector();

        WindowMenuLayerPointer m_menuLayer = nullptr;

        bool m_isViewportVisible = true;
        bool m_isSceneInspectorVisible = true;
        bool m_isEntityInspectorVisible = true;
    };
} // namespace ego::editor
