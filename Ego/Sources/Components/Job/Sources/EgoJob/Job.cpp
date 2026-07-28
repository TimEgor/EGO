#include "Job.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Profile/Profile.h"

ego::Job::Job(const char* _dbgName)
#ifdef EGO_JOB_DEBUG
    : m_dbgName(_dbgName)
#endif
{
}

void ego::Job::wait()
{
    std::unique_lock locker(m_mutex);

    if (m_state == JobState::Finished)
    {
        return;
    }

    if (m_state == JobState::Undefined)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job hasn't been scheduled.");
        return;
    }

    m_completionNotifier.wait(
        locker,
        [this]()
        {
            return m_state == JobState::Finished;
        });
}

void ego::Job::execute()
{
    tryExecute();
}

bool ego::Job::isFinished() const
{
    return m_state == JobState::Finished;
}

void ego::Job::setExecutionContext(const JobControllerWeakPointer& _jobController)
{
    trySetExecutionContext(_jobController);
}

bool ego::Job::trySetExecutionContext(const JobControllerWeakPointer& _jobController)
{
    if (!m_jobController.isExpired())
    {
        EGO_ASSERT_FAIL_MESSAGE("Job execution context has been already assigned.");
        return false;
    }

    auto expectedState = JobState::Undefined;
    if (!m_state.compare_exchange_strong(expectedState, JobState::Pending))
    {
        EGO_ASSERT_FAIL_MESSAGE("Job has been already scheduled.");
        return false;
    }

    m_jobController = _jobController;
    return true;
}

bool ego::Job::tryExecute()
{
    auto expectedState = JobState::Pending;
    if (!m_state.compare_exchange_strong(expectedState, JobState::Executing))
    {
        EGO_ASSERT_FAIL_MESSAGE("Job execution state is invalid.");
        return false;
    }

    {
#ifdef EGO_JOB_DEBUG
        EGO_PROFILE_BLOCK_EVENT(m_dbgName ? m_dbgName : "Job");
#else
        EGO_PROFILE_BLOCK_EVENT("Job");
#endif
        operate();
    }

    {
        std::lock_guard locker(m_mutex);
        m_state = JobState::Finished;
    }
    m_completionNotifier.notify_all();

    return true;
}

void ego::LambdaJob::operate()
{
    if (!m_function)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job lambda is null");
        return;
    }

    m_function();
}

ego::LambdaJob::LambdaJob(const JobFunction& _function)
    : m_function(_function)
{
    EGO_ASSERT_MESSAGE(_function, "Job lambda is null");
}

ego::LambdaJob::LambdaJob(const JobFunction& _function, const char* _dbgName)
    : Job(_dbgName),
      m_function(_function)
{
    EGO_ASSERT_MESSAGE(_function, "Job lambda is null");
}

ego::JobPointer ego::CreateLambdaJob(const LambdaJob::JobFunction& _function)
{
    return MakePointer<LambdaJob>(_function);
}

ego::JobPointer ego::CreateLambdaJob(const LambdaJob::JobFunction& _function, const char* _dbgName)
{
    return MakePointer<LambdaJob>(_function, _dbgName);
}
