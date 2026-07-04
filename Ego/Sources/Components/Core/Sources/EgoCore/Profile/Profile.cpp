#include "Profile.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Context/DiagnosticContext.h"

ego::profile::ProfilerController::ProfilerController()
    : m_profilerRaw(nullptr)
{
}

bool ego::profile::ProfilerController::setProfiler(const ProfilerPointer& _profiler)
{
    EGO_CHECK_RETURN_FALSE(_profiler);

    std::lock_guard lock(m_lock);
    if (m_profiler && m_profiler.get() != _profiler.get())
    {
        EGO_ASSERT_FAIL_MESSAGE("Profiler has been already set.");
        return false;
    }

    m_profiler = _profiler;
    m_profilerRaw.store(m_profiler.get(), std::memory_order_release);

    return true;
}

void ego::profile::ProfilerController::resetProfiler(const ProfilerPointer& _profiler)
{
    std::lock_guard lock(m_lock);
    if (_profiler && m_profiler && m_profiler.get() != _profiler.get())
    {
        EGO_ASSERT_FAIL_MESSAGE("Profiler reset request doesn't match active profiler.");
        return;
    }

    m_profilerRaw.store(nullptr, std::memory_order_release);
    m_profiler = nullptr;
}

ego::profile::ProfilerPointer ego::profile::ProfilerController::getProfiler() const
{
    std::lock_guard lock(m_lock);
    return m_profiler;
}

void ego::profile::ProfilerController::beginEvent(const char* _titleName, const char* _contextName) const
{
    Profiler* profiler = m_profilerRaw.load(std::memory_order_acquire);
    if (profiler)
    {
        profiler->beginEvent(_titleName, _contextName);
    }
}

void ego::profile::ProfilerController::endEvent() const
{
    Profiler* profiler = m_profilerRaw.load(std::memory_order_acquire);
    if (profiler)
    {
        profiler->endEvent();
    }
}

namespace
{
    ego::profile::ProfilerControllerPointer GetDefaultProfilerController()
    {
        static const ego::profile::ProfilerControllerPointer Controller = new ego::profile::ProfilerController();
        return Controller;
    }
} // namespace

ego::profile::ProfilerControllerPointer ego::profile::GetProfilerController()
{
    const context::DiagnosticContextPointer diagnosticContext = context::GetDiagnosticContextPointer();
    if (diagnosticContext && diagnosticContext->getProfilerController())
    {
        return diagnosticContext->getProfilerController();
    }

    return GetDefaultProfilerController();
}

void ego::profile::BeginEvent(const char* _titleName, const char* _contextName)
{
#if EGO_ENABLE_PROFILING
    const ProfilerControllerPointer controller = GetProfilerController();
    if (controller)
    {
        controller->beginEvent(_titleName, _contextName);
    }
#endif
}

void ego::profile::EndEvent()
{
#if EGO_ENABLE_PROFILING
    const ProfilerControllerPointer controller = GetProfilerController();
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
