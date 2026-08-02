#pragma once

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGui/Gui.h"

#include "EditorTitleBar.h"
#include "GuiWindowController.h"
#include "Menu/GuiMenuController.h"
#include "Menu/GuiMenuOrder.h"
#include "Modal/GuiModalWindow.h"

namespace ego::editor
{
    class EditorGuiController final
        : public gui::GuiLayer, public NonCopyable
    {
    public:
        EditorGuiController() = default;
        ~EditorGuiController() override;

        bool init(const XmlDocument& _config);
        void release();

        void setSceneTexture(const gpu::TextureViewPointer& _sceneTexture);

        bool registerMenuLayer(const GuiMenuLayerPointer& _layer, GuiMenuOrder _order);
        bool unregisterMenuLayer(const GuiMenuLayerPointer& _layer);

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
