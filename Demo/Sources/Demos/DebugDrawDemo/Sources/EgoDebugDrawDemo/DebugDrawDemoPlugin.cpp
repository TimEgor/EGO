#include "DebugDrawDemoPlugin.h"

#include "EgoPlugin/ExternalModule.h"

#include "DebugDrawDemo.h"

EGO_MODULE_ENTRY();

EGO_PLUGIN_CREATE(ego::demo::DebugDrawDemoPlugin, EngineLogicPlugin, ego::engine::EngineLogicPlugin);

ego::demo::DebugDrawDemoPlugin::DebugDrawDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : engine::EngineLogicPlugin(_module, _pluginType)
{
}

ego::engine::EngineLogicPointer ego::demo::DebugDrawDemoPlugin::createEngineLogic()
{
    return ego::engine::EngineLogicPointer(new DebugDrawDemo());
}
