#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/Reference/Reference.h"

#include <condition_variable>
#include <functional>

#ifndef EGO_CONFIG_RETAIL
#define EGO_JOB_DEBUG
#endif

namespace ego
{
	class JobController;
	EGO_POINTER(JobController);
	EGO_WEAK_POINTER(JobController);

	enum class JobState
	{
		Undefined,
		Pending,
		Executing,
		Finished
	};

    class Job : public STDDestroyMTCountable
    {
        struct JobEvent final
        {
            std::condition_variable m_notifier;
            Job* m_job = nullptr;

            JobEvent(Job* _job);

            void wait();
            void set();
        };

        friend JobEvent;

    public:
        Job() = default;
        Job(const char* _dbgName);
        ~Job() override;

        void wait();
        void execute();

        bool isFinished() const;

        JobControllerPointer getJobController() const { return m_jobController.lock(); }
        void setExecutionContext(const JobControllerWeakPointer& _jobController);

        JobState getState() const { return m_state; }

#ifdef EDGE_JOB_DEBUG
        const char* getDbgName() const { return m_dbgName; }
#endif

    protected:
        virtual void operate() = 0;

    private:
        mutable std::mutex m_mutex;

        JobControllerWeakPointer m_jobController;
        JobEvent* m_completionEvent = nullptr;

        std::atomic<JobState> m_state;

#ifdef EDGE_JOB_DEBUG
        const char* m_dbgName = nullptr;
#endif
    };

    EGO_POINTER(Job);
    EGO_WEAK_POINTER(Job);
    using JobReference = JobPointer;
    using JobWeakReference = JobWeakPointer;

    class LambdaJob final : public Job
    {
    public:
        using JobFunction = std::function<void()>;

        LambdaJob(const JobFunction& _function);
        LambdaJob(const JobFunction& _function, const char* _dbgName);

    private:
        virtual void operate() override;

        JobFunction m_function;
    };

    JobReference CreateLambdaJob(const LambdaJob::JobFunction& _function);
    JobReference CreateLambdaJob(const LambdaJob::JobFunction& _function, const char* _dbgName);
}
