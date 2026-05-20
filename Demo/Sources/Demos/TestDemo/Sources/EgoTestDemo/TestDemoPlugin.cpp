#include "TestDemoPlugin.h"

#include "EgoCore/FileName/FileNameUtils.h"

#include "EgoEngine/Plugin/EngineExternalModuleCore.h"

#include "EgoDemoFramework/PluginController/DemoExternalModuleCore.h"

#include "TestDemo.h"

EGO_CORE_MODULE();
EGO_ENGINE_MODULE();
EGO_DEMO_MODULE();

EGO_PLUGIN_CREATE(ego::demo::TestDemoPlugin, DemoPlugin, ego::demo::DemoPlugin);

ego::demo::TestDemoPlugin::TestDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : DemoPlugin(_module, _pluginType)
{
}

ego::demo::DemoPointer ego::demo::TestDemoPlugin::createDemo()
{
    const FileName moduleDirPath = file_name_utils::GetFileDirPath(getModule()->getInfo().m_modulePath);
    return DemoPointer(new TestDemo(moduleDirPath + "/TestDemoConfig.xml"));
}
