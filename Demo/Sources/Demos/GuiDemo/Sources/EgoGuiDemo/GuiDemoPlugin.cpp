#include "GuiDemoPlugin.h"

#include "EgoRuntime/Plugin/ExternalModule.h"

#include "GuiDemo.h"

EGO_MODULE_ENTRY();

EGO_PLUGIN_CREATE(ego::demo::GuiDemoPlugin, EngineLogicPlugin, ego::engine_framework::EngineLogicPlugin);

ego::demo::GuiDemoPlugin::GuiDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : EngineLogicPlugin(_module, _pluginType)
{
}

ego::engine_framework::EngineLogicPointer ego::demo::GuiDemoPlugin::createEngineLogic()
{
    return engine_framework::EngineLogicPointer(new GuiDemo());
}
