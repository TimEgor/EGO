#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include "EgoCore/Pointer/Pointer.h"

#include "Job.h"
#include "JobGraph.h"
#include "JobQueue.h"

namespace ego
{
    class JobController final : public EnableSharedFromThis<JobController>
    {
    public:
        JobController() = default;
        ~JobController()
        {
            release();
        }

        bool init(uint32_t _threadCount, const char* _name = "EgoJob");

        void addJob(const JobPointer& _job);
        void addJobGraph(const JobGraphPointer& _jobGraph);

        void wait(const JobPointer& job);
        void waitAndExecute(const JobPointer& job);

        void wait(const JobGraphPointer& _jobGraph);
        void waitAndExecute(const JobGraphPointer& _jobGraph);

        uint32_t getJobExecutorCount() const;

        static uint32_t GetHardwareThreadCount();

    private:
        void release();

        static void ThreadFunction(JobControllerWeakPointer _jobController);

        bool executeOnCurrentThread();

        using ThreadCollection = std::vector<std::thread>;

        ThreadCollection m_threads;
        JobQueue m_jobQueue;

        std::atomic_bool m_isStoped = true;
    };
} // namespace ego
