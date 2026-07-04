#include "TestDemoPlugin.h"

#include "EgoRuntime/Plugin/ExternalModule.h"

#include "TestDemo.h"

EGO_MODULE_ENTRY();

EGO_PLUGIN_CREATE(ego::demo::TestDemoPlugin, EngineLogicPlugin, ego::engine_framework::EngineLogicPlugin);

ego::demo::TestDemoPlugin::TestDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : engine_framework::EngineLogicPlugin(_module, _pluginType)
{
}

ego::engine_framework::EngineLogicPointer ego::demo::TestDemoPlugin::createEngineLogic()
{
    return ego::engine_framework::EngineLogicPointer(new TestDemo());
}
