#include "DebugDrawDemoPlugin.h"

#include "EgoEngine/Plugin/EngineExternalModuleCore.h"

#include "EgoFramework/Plugin/FrameworkExternalModuleCore.h"

#include "DebugDrawDemo.h"

EGO_CORE_MODULE();
EGO_ENGINE_MODULE();
EGO_FRAMEWORK_MODULE();

EGO_PLUGIN_CREATE(ego::demo::DebugDrawDemoPlugin, GameLogicPlugin, ego::framework::GameLogicPlugin);

ego::demo::DebugDrawDemoPlugin::DebugDrawDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : framework::GameLogicPlugin(_module, _pluginType)
{}

ego::framework::GameLogicPointer ego::demo::DebugDrawDemoPlugin::createGameLogic()
{
    return ego::framework::GameLogicPointer(new DebugDrawDemo());
}
