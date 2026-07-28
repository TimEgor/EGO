#include "DefaultGuiRenderPlugin.h"

#include "EgoPlugin/ExternalModule.h"

#include "DefaultGuiRender.h"

EGO_MODULE_ENTRY();

EGO_PLUGIN_CREATE(ego::gui::default_gui_render::DefaultGuiRenderPlugin, GuiRenderPlugin, ego::gui::GuiRenderPlugin);

ego::gui::default_gui_render::DefaultGuiRenderPlugin::DefaultGuiRenderPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : GuiRenderPlugin(_module, _pluginType)
{
}

ego::gui::GuiRenderPointer ego::gui::default_gui_render::DefaultGuiRenderPlugin::createGuiRender()
{
    return MakePointer<DefaultGuiRender>();
}
