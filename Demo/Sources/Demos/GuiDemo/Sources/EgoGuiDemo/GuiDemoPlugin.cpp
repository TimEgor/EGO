#include "GuiDemoPlugin.h"

#include "EgoPlugin/ExternalModule.h"

#include "GuiDemo.h"

EGO_MODULE_ENTRY();

EGO_PLUGIN_CREATE(ego::demo::GuiDemoPlugin, EngineLogicPlugin, ego::engine::EngineLogicPlugin);

ego::demo::GuiDemoPlugin::GuiDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : EngineLogicPlugin(_module, _pluginType)
{
}

ego::engine::EngineLogicPointer ego::demo::GuiDemoPlugin::createEngineLogic()
{
    return engine::EngineLogicPointer(new GuiDemo());
}
