#pragma once

#include "Job.h"
#include "JobGraph.h"
#include "JobQueue.h"
#include "EgoCore/Reference/Pointer.h"

#include <vector>


namespace ego
{
	class JobController final : public EnableSharedFromThis<JobController>
	{
	public:
		JobController() = default;
		~JobController() { release(); }

		bool init(uint32_t _threadCount, const char* _name = "EgoJob");
		void release();

		void addJob(const JobReference& _job);
		void addJobGraph(const JobGraphReference& _jobGraph);

		void wait(const JobReference& job);
		void waitAndExecute(const JobReference& job);

		void wait(const JobGraphReference& _jobGraph);
		void waitAndExecute(const JobGraphReference& _jobGraph);

		uint32_t getJobExecutorCount() const;

		static uint32_t GetHardwareThreadCount();

	private:
		static void ThreadFunction(JobControllerWeakPointer _jobController);

		bool executeOnCurrentThread();

		using ThreadCollection = std::vector<std::thread>;

		ThreadCollection m_threads;
		JobQueue m_jobQueue;

		std::atomic_bool m_isStoped = false;
	};
}
