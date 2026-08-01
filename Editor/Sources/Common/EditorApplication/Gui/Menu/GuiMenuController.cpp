#include "GuiMenuController.h"

#include "EgoCore/UtilsMacros.h"

#include <imgui.h>

bool ego::editor::GuiMenuController::init()
{
    m_layers.reserve(static_cast<size_t>(GuiMenuOrder::COUNT));

    return true;
}

void ego::editor::GuiMenuController::release()
{
    m_layers.clear();
}

float ego::editor::GuiMenuController::draw()
{
    float menuMaxX = ImGui::GetCursorScreenPos().x;

    for (LayerRecord& layerRecord : m_layers)
    {
        const float layerMenuMaxX = layerRecord.m_layer->drawMenu();
        menuMaxX = std::max(layerMenuMaxX, menuMaxX);
    }

    return menuMaxX;
}

bool ego::editor::GuiMenuController::registerLayer(const GuiMenuLayerPointer& _layer, GuiMenuOrder _order)
{
    EGO_CHECK_RETURN_FALSE(_layer);

    for (const LayerRecord& layerRecord : m_layers)
    {
        if (layerRecord.m_layer == _layer)
        {
            return true;
        }
    }

    auto layerPosition = m_layers.begin();
    while (layerPosition != m_layers.end() && layerPosition->m_order <= _order)
    {
        ++layerPosition;
    }

    m_layers.insert(layerPosition, {.m_order = _order, .m_layer = _layer});

    return true;
}

bool ego::editor::GuiMenuController::unregisterLayer(const GuiMenuLayerPointer& _layer)
{
    EGO_CHECK_RETURN_FALSE(_layer);

    auto layer = m_layers.begin();
    while (layer != m_layers.end() && layer->m_layer != _layer)
    {
        ++layer;
    }

    EGO_CHECK_RETURN_FALSE(layer != m_layers.end());

    m_layers.erase(layer);

    return true;
}
