#pragma once

#include <deque>

#include "Job.h"

namespace ego
{
    class JobQueue final
    {
    public:
        JobQueue() = default;
        ~JobQueue()
        {
            release();
        }

        void init();
        void release();

        void addJob(const JobPointer& _job);
        JobPointer getJob();
        JobPointer tryGetJob();

    private:
        std::deque<JobPointer> m_queue;
        std::condition_variable m_wakeCondition;
        std::mutex m_mutex;

        std::atomic_bool m_isReleased = true;
    };
} // namespace ego
