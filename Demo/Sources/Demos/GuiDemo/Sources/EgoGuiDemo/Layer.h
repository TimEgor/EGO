#pragma once

#include <cstdint>
#include <string>

#include "EgoGui/Layer.h"

#include "GuiDemoPropertyValues.h"

namespace ego::demo
{
    class Layer final : public gui::Layer
    {
    public:
        void reset();

    private:
        void drawGui() override;

        void drawMenu();
        void drawPropertyValuesWindow();

        std::string m_name = "Directional Light";
        uint32_t m_clickCount = 0;
        int m_renderMode = 0;
        GuiDemoPropertyValues m_propertyValues;
        GuiDemoReadOnlyPropertyValues m_readOnlyPropertyValues;

        bool m_enableShadows = true;
        bool m_showMainWindow = true;
        bool m_showSceneWindow = true;
        bool m_showInspectorWindow = true;
        bool m_showPropertyValuesWindow = true;
        bool m_showDearImGuiDemo = false;
    };
} // namespace ego::demo
