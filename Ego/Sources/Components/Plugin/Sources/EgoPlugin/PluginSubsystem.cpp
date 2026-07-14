#include "PluginSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

#include "PluginCatalog.h"

bool ego::PluginSubsystem::init()
{
    m_pluginCatalog = new PluginCatalog();
    EGO_CHECK_INITIALIZATION(m_pluginCatalog);

    return true;
}

void ego::PluginSubsystem::release()
{
    if (m_pluginCatalog)
    {
        m_pluginCatalog->clear();
        m_pluginCatalog.reset();
    }
}

ego::PluginCatalogPointer ego::PluginSubsystem::getPluginCatalogPointer() const
{
    return m_pluginCatalog;
}

ego::PluginCatalog& ego::PluginSubsystem::getPluginCatalog() const
{
    EGO_ASSERT(m_pluginCatalog);
    return *m_pluginCatalog;
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
