#include "Plugin.h"

#include "PluginController.h"

ego::Plugin::Plugin(const PluginModulePointer& _module)
    : m_module(_module)
{
    
}

ego::Plugin::~Plugin()
{
    PluginController::PluginControllerAccessor::ReleasePlugin(this);
}

ego::PluginModulePointer ego::Plugin::getModule() const
{
    return m_module;
}

ego::PluginID ego::Plugin::getPluginID() const
{
    return CalcPluginID(m_module->getInfo().m_moduleId, getType());
}