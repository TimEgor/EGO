#pragma once

#include "EgoGui/Rendering/GuiRenderPlugin.h"

namespace ego::gui::default_gui_render
{
    class DefaultGuiRenderPlugin final : public GuiRenderPlugin
    {
    public:
        DefaultGuiRenderPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        GuiRenderPointer createGuiRender() override;

        EGO_PLUGIN(DefaultGuiRenderPlugin, GuiRenderPlugin);
    };

    EGO_POINTER(DefaultGuiRenderPlugin);
} // namespace ego::gui::default_gui_render
