#include "ApplicationProfiler.h"

#include "EgoCore/Diagnostic/DiagnosticSubsystem.h"
#include "EgoCore/Profile/ProfileController.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoPlugin/PluginController.h"

#include "ProfilerPlugin.h"

ego::application::ApplicationProfiler::~ApplicationProfiler()
{
    release();
}

bool ego::application::ApplicationProfiler::init(const FileName& _pluginModuleName)
{
    EGO_CHECK_RETURN_FALSE(!m_plugin);
    EGO_CHECK_RETURN_FALSE(!m_profiler);
    if (!_pluginModuleName)
    {
        return true;
    }

    const PluginControllerPointer pluginController = GetPluginControllerPointer();
    EGO_CHECK_RETURN_FALSE(pluginController);

    const ProfilerPluginPointer plugin = pluginController->loadPlugin<ProfilerPlugin>(_pluginModuleName);
    EGO_CHECK_RETURN_FALSE(plugin);

    const profile::ProfilerPointer profiler = plugin->createProfiler();
    EGO_CHECK_RETURN_FALSE(profiler);

    const profile::ProfilerControllerPointer profilerController = GetProfilerController();
    EGO_CHECK_RETURN_FALSE(profilerController);
    EGO_CHECK_RETURN_FALSE(profilerController->setProfiler(profiler));

    m_plugin = plugin;
    m_profiler = profiler;
    return true;
}

void ego::application::ApplicationProfiler::release()
{
    const profile::ProfilerControllerPointer profilerController = GetProfilerController();
    if (m_profiler && profilerController)
    {
        profilerController->resetProfiler(m_profiler);
    }

    m_profiler = nullptr;
    m_plugin = nullptr;
}
