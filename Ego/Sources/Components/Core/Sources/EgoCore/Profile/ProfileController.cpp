#include "ProfileController.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

bool ego::profile::ProfilerController::setProfiler(const ProfilerPointer& _profiler)
{
    EGO_CHECK_RETURN_FALSE(_profiler);

    std::unique_lock lock(m_lock);
    if (m_profiler && m_profiler.get() != _profiler.get())
    {
        EGO_ASSERT_FAIL_MESSAGE("Profiler has been already set.");
        return false;
    }

    m_profiler = _profiler;

    return true;
}

void ego::profile::ProfilerController::resetProfiler(const ProfilerPointer& _profiler)
{
    std::unique_lock lock(m_lock);
    if (_profiler && m_profiler && m_profiler.get() != _profiler.get())
    {
        EGO_ASSERT_FAIL_MESSAGE("Profiler reset request doesn't match active profiler.");
        return;
    }

    m_profiler = nullptr;
}

ego::profile::ProfilerPointer ego::profile::ProfilerController::getProfiler() const
{
    std::shared_lock lock(m_lock);
    return m_profiler;
}

void ego::profile::ProfilerController::beginEvent(const char* _titleName, const char* _contextName) const
{
    std::shared_lock lock(m_lock);
    if (m_profiler)
    {
        m_profiler->beginEvent(_titleName, _contextName);
    }
}

void ego::profile::ProfilerController::endEvent() const
{
    std::shared_lock lock(m_lock);
    if (m_profiler)
    {
        m_profiler->endEvent();
    }
}
