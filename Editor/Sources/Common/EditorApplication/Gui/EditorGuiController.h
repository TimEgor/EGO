#pragma once

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGui/Gui.h"

#include "EditorTitleBar.h"
#include "Menu/GuiMenuController.h"
#include "Window/GuiModalWindow.h"
#include "Window/GuiWindowController.h"

namespace ego::editor
{
    class EditorGuiController final : public gui::GuiLayer, public NonCopyable
    {
    public:
        EditorGuiController() = default;
        ~EditorGuiController() override;

        bool init(const XmlDocument& _config);
        void release();

        void setSceneTexture(const gpu::TextureViewPointer& _sceneTexture);
        gui::GuiFrameTextureID getSceneTextureID();

        GuiWindowController& getWindowController();
        const GuiWindowController& getWindowController() const;

        bool pushModalWindow(const GuiModalWindowPointer& _window);

    private:
        static gui::GuiControllerPointer GetGuiControllerPointer();

        bool readDefaultFont(const XmlDocument& _config, FileName& _path, float& _size) const;
        void drawGui() override;

        EditorTitleBar m_titleBar;

        GuiMenuController m_menuController;
        GuiWindowController m_windowController;

        gpu::TextureViewPointer m_sceneTexture = nullptr;
    };
} // namespace ego::editor
