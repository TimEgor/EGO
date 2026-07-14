#include "PlatformPluginSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

#include "PluginController.h"

bool ego::PlatformPluginSubsystem::init()
{
    m_pluginController = new PluginController();
    EGO_CHECK_INITIALIZATION(m_pluginController && m_pluginController->init());

    return true;
}

void ego::PlatformPluginSubsystem::release()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_pluginController);
}

ego::PluginControllerPointer ego::PlatformPluginSubsystem::getPluginControllerPointer() const
{
    return m_pluginController;
}

ego::PluginController& ego::PlatformPluginSubsystem::getPluginController() const
{
    EGO_ASSERT(m_pluginController);
    return *m_pluginController;
}

ego::PlatformPluginSubsystemPointer ego::GetPlatformPluginSubsystemPointer()
{
    return subsystem::FindSubsystem<PlatformPluginSubsystem>();
}

ego::PlatformPluginSubsystem& ego::GetPlatformPluginSubsystem()
{
    const PlatformPluginSubsystemPointer platformPluginSubsystem = GetPlatformPluginSubsystemPointer();
    EGO_ASSERT(platformPluginSubsystem);

    return *platformPluginSubsystem;
}
