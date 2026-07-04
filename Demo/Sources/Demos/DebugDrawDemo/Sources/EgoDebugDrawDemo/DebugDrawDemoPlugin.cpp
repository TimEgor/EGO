#include "DebugDrawDemoPlugin.h"

#include "EgoRuntime/Plugin/ExternalModule.h"

#include "DebugDrawDemo.h"

EGO_MODULE_ENTRY();

EGO_PLUGIN_CREATE(ego::demo::DebugDrawDemoPlugin, EngineLogicPlugin, ego::engine_framework::EngineLogicPlugin);

ego::demo::DebugDrawDemoPlugin::DebugDrawDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : engine_framework::EngineLogicPlugin(_module, _pluginType)
{
}

ego::engine_framework::EngineLogicPointer ego::demo::DebugDrawDemoPlugin::createEngineLogic()
{
    return ego::engine_framework::EngineLogicPointer(new DebugDrawDemo());
}
