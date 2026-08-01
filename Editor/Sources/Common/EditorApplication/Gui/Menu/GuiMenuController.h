#pragma once

#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"

#include "GuiMenuLayer.h"
#include "GuiMenuOrder.h"

namespace ego::editor
{
    class GuiMenuController final : public NonCopyable
    {
    public:
        GuiMenuController() = default;

        bool init();
        void release();

        float draw();

        bool registerLayer(const GuiMenuLayerPointer& _layer, GuiMenuOrder _order);
        bool unregisterLayer(const GuiMenuLayerPointer& _layer);

    private:
        struct LayerRecord final
        {
            GuiMenuOrder m_order = GuiMenuOrder::Project;
            GuiMenuLayerPointer m_layer = nullptr;
        };

        std::vector<LayerRecord> m_layers;
    };
} // namespace ego::editor
