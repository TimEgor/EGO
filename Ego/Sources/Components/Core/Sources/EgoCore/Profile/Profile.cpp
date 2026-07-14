#include "Profile.h"

#include "EgoCore/Profile/ProfileController.h"
#include "EgoCore/Diagnostic/DiagnosticSubsystem.h"

void ego::profile::BeginEvent(const char* _titleName, const char* _contextName)
{
#if EGO_ENABLE_PROFILING
    const ProfilerControllerPointer controller = ego::GetProfilerController();
    if (controller)
    {
        controller->beginEvent(_titleName, _contextName);
    }
#endif
}

void ego::profile::EndEvent()
{
#if EGO_ENABLE_PROFILING
    const ProfilerControllerPointer controller = ego::GetProfilerController();
    if (controller)
    {
        controller->endEvent();
    }
#endif
}

ego::profile::ProfileEvent::ProfileEvent(const char* _titleName, const char* _contextName)
{
#if EGO_ENABLE_PROFILING
    BeginEvent(_titleName, _contextName);
    m_isActive = true;
#endif
}

ego::profile::ProfileEvent::~ProfileEvent()
{
#if EGO_ENABLE_PROFILING
    if (m_isActive)
    {
        EndEvent();
    }
#endif
}
