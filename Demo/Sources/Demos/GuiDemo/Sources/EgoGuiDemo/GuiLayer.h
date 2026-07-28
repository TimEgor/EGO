#pragma once

#include <cstdint>
#include <string>

#include "EgoGui/GuiLayer.h"

namespace ego::demo
{
    class GuiLayer final : public gui::GuiLayer
    {
    public:
        void reset();

    private:
        void drawGui() override;

        std::string m_name = "Directional Light";
        uint32_t m_clickCount = 0;
        int m_renderMode = 0;

        bool m_enableShadows = true;
        bool m_showMainWindow = true;
        bool m_showSceneWindow = true;
        bool m_showInspectorWindow = true;
        bool m_showDearImGuiDemo = false;
    };
} // namespace ego::demo
