#include "TestDemoPlugin.h"

#include "EgoPlugin/ExternalModule.h"

#include "TestDemo.h"

EGO_MODULE_ENTRY();

EGO_PLUGIN_CREATE(ego::demo::TestDemoPlugin, EngineLogicPlugin, ego::engine::EngineLogicPlugin);

ego::demo::TestDemoPlugin::TestDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : engine::EngineLogicPlugin(_module, _pluginType)
{
}

ego::engine::EngineLogicPointer ego::demo::TestDemoPlugin::createEngineLogic()
{
    return ego::engine::EngineLogicPointer(new TestDemo());
}
