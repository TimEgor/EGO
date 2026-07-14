#include "DefaultRenderPlugin.h"

#include "EgoPlugin/ExternalModule.h"

#include "DefaultRender.h"

EGO_MODULE_ENTRY();

EGO_PLUGIN_CREATE(ego::render::default_render::DefaultRenderPlugin, RenderPlugin, ego::render::RenderPlugin);

ego::render::default_render::DefaultRenderPlugin::DefaultRenderPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : RenderPlugin(_module, _pluginType)
{
}

ego::render::RenderPointer ego::render::default_render::DefaultRenderPlugin::createRender()
{
    return RenderPointer(new DefaultRender());
}
