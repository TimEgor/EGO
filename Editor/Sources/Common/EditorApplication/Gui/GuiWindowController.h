#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGui/Rendering/GuiTexture.h"

#include "Menu/WindowMenuLayer.h"
#include "Modal/GuiModalWindow.h"

namespace ego::editor
{
    class GuiWindowController final
        : public NonCopyable
    {
    public:
        GuiWindowController() = default;

        bool init();
        void release();

        bool pushModalWindow(const GuiModalWindowPointer& _window);

        void drawWindows(gui::GuiFrameTextureID _sceneTextureID);

        bool isViewportVisible() const;
        void setViewportVisible(bool _isVisible);
        bool isSceneInspectorVisible() const;
        void setSceneInspectorVisible(bool _isVisible);
        bool isEntityInspectorVisible() const;
        void setEntityInspectorVisible(bool _isVisible);

    private:
        struct ModalWindowRecord final
        {
            uint64_t m_instanceID = 0;
            GuiModalWindowPointer m_window = nullptr;
        };

        using ModalWindowStack = std::vector<ModalWindowRecord>;

        void drawDockSpace();
        void drawViewport(gui::GuiFrameTextureID _sceneTextureID);
        void drawSceneInspector();
        void drawEntityInspector();
        void drawModalWindow(std::size_t _index);

        WindowMenuLayerPointer m_menuLayer = nullptr;

        ModalWindowStack m_modalWindowStack;
        uint64_t m_nextModalWindowInstanceID = 0;

        bool m_isViewportVisible = true;
        bool m_isSceneInspectorVisible = true;
        bool m_isEntityInspectorVisible = true;
    };
} // namespace ego::editor
