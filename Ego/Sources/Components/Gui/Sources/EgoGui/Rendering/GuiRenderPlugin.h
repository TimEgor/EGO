#pragma once

#include "EgoPlugin/Plugin.h"

#include "GuiRender.h"

namespace ego::gui
{
    class GuiRenderPlugin : public Plugin
    {
    public:
        GuiRenderPlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : Plugin(_module, _pluginType)
        {
        }

        virtual GuiRenderPointer createGuiRender() = 0;

        EGO_PLUGIN(GuiRenderPlugin, Plugin);
    };

    EGO_POINTER(GuiRenderPlugin);
} // namespace ego::gui
