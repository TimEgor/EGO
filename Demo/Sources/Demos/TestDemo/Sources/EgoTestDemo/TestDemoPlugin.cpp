#include "TestDemoPlugin.h"

#include "EgoEngine/Plugin/EngineExternalModuleCore.h"

#include "EgoFramework/Plugin/FrameworkExternalModuleCore.h"

#include "TestDemo.h"

EGO_CORE_MODULE();
EGO_ENGINE_MODULE();
EGO_FRAMEWORK_MODULE();

EGO_PLUGIN_CREATE(ego::demo::TestDemoPlugin, GameLogicPlugin, ego::framework::GameLogicPlugin);

ego::demo::TestDemoPlugin::TestDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : framework::GameLogicPlugin(_module, _pluginType)
{
}

ego::framework::GameLogicPointer ego::demo::TestDemoPlugin::createGameLogic()
{
    return ego::framework::GameLogicPointer(new TestDemo());
}
