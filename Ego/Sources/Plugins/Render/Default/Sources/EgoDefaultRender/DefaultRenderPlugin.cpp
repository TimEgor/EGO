#include "DefaultRenderPlugin.h"

#include "EgoEngine/Plugin/EngineExternalModuleCore.h"

#include "DefaultRender.h"

EGO_CORE_MODULE();
EGO_ENGINE_MODULE();

EGO_PLUGIN_CREATE(ego::default_render::DefaultRenderPlugin, RenderPlugin, ego::RenderPlugin);

ego::default_render::DefaultRenderPlugin::DefaultRenderPlugin(
    const PluginModulePointer& _module,
    PluginType _pluginType
)
    : RenderPlugin(_module, _pluginType)
{}

ego::RenderPointer ego::default_render::DefaultRenderPlugin::createRender()
{
    return RenderPointer(new DefaultRender());
}
