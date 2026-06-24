#include "Plugin.h"

#include "PluginController.h"

ego::Plugin::Plugin(const PluginModulePointer& _module, PluginType _pluginType)
    : m_module(_module),
      m_loadedType(_pluginType)
{
}

ego::Plugin::~Plugin() = default;

void ego::PluginDeleter::operator()(Plugin* _plugin) const
{
    if (!_plugin)
    {
        return;
    }

    PluginModulePointer module = _plugin->getModule();
    PluginControllerPointer pluginController = PluginControllerCore::GetInstance().getPluginController();
    EGO_ASSERT(pluginController);

    if (pluginController)
    {
        PluginController::PluginControllerAccessor::ReleasePlugin(*pluginController, _plugin);
    }

    delete _plugin;
}

ego::PluginModulePointer ego::Plugin::getModule() const
{
    return m_module;
}

ego::PluginID ego::Plugin::getPluginID() const
{
    return CalcPluginID(m_module->getInfo().m_moduleId, m_loadedType);
}
