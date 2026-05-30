#include "ProfilerPlugin.h"

#include "EgoCore/Assert/AssertCore.h"

void ego::ProfilerPlugin::onLoaded()
{
    if (m_profiler)
    {
        return;
    }

    m_profiler = createProfiler();
    EGO_ASSERT(m_profiler);

    const profile::ProfilerControllerPointer profilerController = profile::GetProfilerController();
    EGO_ASSERT(profilerController);

    if (!m_profiler || !profilerController || !profilerController->setProfiler(m_profiler))
    {
        m_profiler = nullptr;
    }
}

void ego::ProfilerPlugin::onUnloaded()
{
    const profile::ProfilerControllerPointer profilerController = profile::GetProfilerController();
    if (m_profiler && profilerController)
    {
        profilerController->resetProfiler(m_profiler);
    }

    m_profiler = nullptr;
}
