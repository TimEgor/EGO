#include "Job.h"

#include "EgoCore/UtilsMacros.h"
#include "EgoCore/Assert/AssertCore.h"
//#include "EgoCore/Profile/Profile.h"

ego::Job::JobEvent::JobEvent(Job* _job)
	: m_job(_job)
{
    
}

void ego::Job::JobEvent::wait()
{
	std::unique_lock locker(m_job->m_mutex);
	m_notifier.wait(locker, [this]() { return m_job->m_state == JobState::Finished; });
}

void ego::Job::JobEvent::set()
{
	m_notifier.notify_all();
}

ego::Job::Job(const char* _dbgName)
#ifdef EDGE_JOB_DEBUG
	: m_dbgName(dbgName)
#endif
{
    
}

ego::Job::~Job()
{
	EGO_SAFE_DESTROY(m_completionEvent);
}

void ego::Job::wait()
{
	if (isFinished())
	{
		return;
	}

	{
		std::lock_guard locker(m_mutex);
		if (!m_completionEvent)
		{
			m_completionEvent = new JobEvent(this);
		}
	}

	m_completionEvent->wait();
}

void ego::Job::execute()
{
#ifdef EDGE_JOB_DEBUG
	EDGE_PROFILE_BLOCK_EVENT_CONTEXT("Job", getDbgName());
#endif

	m_state = JobState::Executing;

	operate();

	m_state = JobState::Finished;

	std::lock_guard locker(m_mutex);
	if (m_completionEvent)
	{
		m_completionEvent->set();
	}
}

bool ego::Job::isFinished() const
{
	return m_state == JobState::Finished;
}

void ego::Job::setExecutionContext(const JobControllerWeakPointer& _jobController)
{
	EGO_ASSERT(m_jobController.isExpired());

	m_jobController = _jobController;
	m_state = JobState::Pending;
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
	: Job(_dbgName), m_function(_function)
{
	EGO_ASSERT_MESSAGE(_function, "Job lambda is null");
}

ego::JobReference ego::CreateLambdaJob(const LambdaJob::JobFunction& _function)
{
	return JobReference(new LambdaJob(_function));
}

ego::JobReference ego::CreateLambdaJob(const LambdaJob::JobFunction& _function, const char* _dbgName)
{
	return JobReference(new LambdaJob(_function, _dbgName));
}
