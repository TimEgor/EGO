#include "PluginSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

#include "PluginController.h"

bool ego::PluginSubsystem::init()
{
    m_pluginCatalog.clear();

    m_pluginController = new PluginController();
    EGO_CHECK_INITIALIZATION(m_pluginController && m_pluginController->init());

    return true;
}

void ego::PluginSubsystem::release()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_pluginController);
    m_pluginCatalog.clear();
}

ego::PluginCatalog& ego::PluginSubsystem::getPluginCatalog()
{
    return m_pluginCatalog;
}

const ego::PluginCatalog& ego::PluginSubsystem::getPluginCatalog() const
{
    return m_pluginCatalog;
}

ego::PluginControllerPointer ego::PluginSubsystem::getPluginControllerPointer() const
{
    return m_pluginController;
}

ego::PluginSubsystemPointer ego::GetPluginSubsystemPointer()
{
    return subsystem::FindSubsystem<PluginSubsystem>();
}

ego::PluginSubsystem& ego::GetPluginSubsystem()
{
    const PluginSubsystemPointer pluginSubsystem = GetPluginSubsystemPointer();
    EGO_ASSERT(pluginSubsystem);

    return *pluginSubsystem;
}
