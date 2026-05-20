#pragma once

#include <deque>

#include "Job.h"

namespace ego
{
	class JobQueue final
	{
	public:
		JobQueue() = default;
		~JobQueue() { release(); }

		void init();
		void release();

		void addJob(const JobReference& _job);
		JobReference getJob();
		JobReference tryGetJob();

	private:
		std::deque<JobReference> m_queue;
		std::condition_variable m_wakeCondition;
		std::mutex m_mutex;

		std::atomic_bool m_isReleased = true;
	};
}
