#pragma once

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGraphicHardware/Presentation/TextureGraphicPresenter.h"

#include "EgoGui/Gui.h"

#include "GuiLayer.h"

namespace ego::editor
{
    class EditorGuiController final : public NonCopyable
    {
    public:
        EditorGuiController() = default;
        ~EditorGuiController() override;

        bool init(const XmlDocument& _config, const PlatformSurfacePointer& _surface, const TextureGraphicPresenterPointer& _simulationGraphicPresenter);
        void release();

        bool isInitialized() const;

    private:
        static gui::GuiControllerPointer GetGuiControllerPointer();
        bool readDefaultFont(const XmlDocument& _config, FileName& _path, float& _size) const;

        GuiLayer m_guiLayer;
        gui::GuiLayerID m_layerID = gui::InvalidGuiLayerID;
    };
} // namespace ego::editor
