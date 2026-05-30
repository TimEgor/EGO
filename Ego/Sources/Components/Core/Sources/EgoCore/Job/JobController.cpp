#include "JobController.h"

#include "EgoCore/ThreadUtils.h"
#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/String/Format.h"

#include <thread>

void ego::JobController::ThreadFunction(JobControllerWeakPointer _jobController)
{
	while (true)
	{
        JobControllerPointer jobController = _jobController.lock();
        if (!jobController)
        {
            return;
        }

		JobReference job = jobController->m_jobQueue.getJob();

		if (jobController->m_isStoped)
		{
			return;
		}

		if (job)
		{
			job->execute();
		}
	}
}

bool ego::JobController::executeOnCurrentThread()
{
	JobReference job = m_jobQueue.tryGetJob();

	if (m_isStoped)
	{
		return false;
	}

	if (job)
	{
		job->execute();
	}
	else
	{
		std::this_thread::yield();
	}

	return true;
}

bool ego::JobController::init(uint32_t _threadCount, const char* _name)
{
	m_isStoped = false;

	m_jobQueue.init();

	for (uint32_t threadIndex = 0; threadIndex < _threadCount; ++threadIndex)
	{
		std::thread thread([jobController = weakFromThis()]() { ThreadFunction(jobController); });

		if (_name)
		{
			threadUtils::SetThreadName(thread.native_handle(), StringFormat("{}_{}", _name, threadIndex).c_str());
		}

		m_threads.push_back(std::move(thread));
	}

	return true;
}

void ego::JobController::release()
{
	m_isStoped = true;

	m_jobQueue.release();

	for (auto& thread : m_threads)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}

	m_threads.clear();
}

void ego::JobController::addJob(const JobReference& _job)
{
	if (!_job)
	{
		EGO_ASSERT_FAIL_MESSAGE("Job is null.");
		return;
	}

	if (m_isStoped)
	{
		EGO_ASSERT_FAIL_MESSAGE("Job controller has been stopped.");
		return;
	}

	if (_job->getState() != JobState::Undefined)
	{
		EGO_ASSERT_FAIL_MESSAGE("Job has been already scheduled.");
		return;
	}

	if (!_job->trySetExecutionContext(weakFromThis()))
	{
		return;
	}

	m_jobQueue.addJob(_job);
}

void ego::JobController::addJobGraph(const JobGraphReference& _jobGraph)
{
	if (!_jobGraph)
	{
		EGO_ASSERT_FAIL_MESSAGE("Job graph is null.");
		return;
	}

	if (m_isStoped)
	{
		EGO_ASSERT_FAIL_MESSAGE("Job controller has been stopped.");
		return;
	}

	JobGraph::JobCollection baseGraphJobs;
	if (!_jobGraph->schedule(weakFromThis(), baseGraphJobs))
	{
		return;
	}

	if (baseGraphJobs.empty())
	{
		EGO_ASSERT_FAIL_MESSAGE("Job graph has no base jobs.");
		return;
	}

	for (const JobReference& job : baseGraphJobs)
	{
		addJob(job);
	}
}

void ego::JobController::wait(const JobReference& job)
{
	if (job)
	{
		job->wait();
	}
}

void ego::JobController::waitAndExecute(const JobReference& job)
{
	if (!job)
	{
		return;
	}

	if (job->getState() == JobState::Undefined)
	{
		EGO_ASSERT_FAIL_MESSAGE("Job hasn't been scheduled.");
		return;
	}

	while(!job->isFinished())
	{
		if (!executeOnCurrentThread())
		{
			break;
		}
	}
}

void ego::JobController::wait(const JobGraphReference& _jobGraph)
{
	if (_jobGraph)
	{
		_jobGraph->wait();
	}
}

void ego::JobController::waitAndExecute(const JobGraphReference& _jobGraph)
{
	if (!_jobGraph)
	{
		return;
	}

	if (!_jobGraph->isScheduled())
	{
		EGO_ASSERT_FAIL_MESSAGE("Job graph hasn't been scheduled.");
		return;
	}

	while (!_jobGraph->isFinished())
	{
		if (!executeOnCurrentThread())
		{
			break;
		}
	}
}

uint32_t ego::JobController::getJobExecutorCount() const
{
	return static_cast<uint32_t>(m_threads.size());
}

uint32_t ego::JobController::GetHardwareThreadCount()
{
	return std::thread::hardware_concurrency();
}
