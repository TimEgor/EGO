#include "JobController.h"

#include "EgoCore/ThreadUtils.h"
#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/String/Format.h"

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
			threadUtils::SetThreadName(thread.native_handle(), StringFormat("%s_%d", _name, threadIndex).c_str());
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
}

void ego::JobController::addJob(const JobReference& _job)
{
	if (!_job)
	{
		EGO_ASSERT_FAIL_MESSAGE("Job is null.");
		return;
	}

	_job->setExecutionContext(weakFromThis());

	m_jobQueue.addJob(_job);
}

void ego::JobController::addJobGraph(const JobGraphReference& _jobGraph)
{
	if (!_jobGraph)
	{
		EGO_ASSERT_FAIL_MESSAGE("Job graph is null.");
		return;
	}

	_jobGraph->setExecutionContext(weakFromThis());

	const JobGraph::JobCollection baseGraphJobs = _jobGraph->getBaseJobs();

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

	while(!job->isFinished())
	{
		if (!executeOnCurrentThread())
		{
			break;;
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
	return m_threads.size();
}

uint32_t ego::JobController::GetHardwareThreadCount()
{
	return std::thread::hardware_concurrency();
}
