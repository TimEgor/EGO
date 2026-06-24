#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/Reference/Reference.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>

#ifndef EGO_ENABLE_JOB_DEBUG
    #if defined(EGO_CONFIG_DEBUG) || defined(EGO_CONFIG_RELEASE) || defined(EGO_CONFIG_PROFILE)
        #define EGO_ENABLE_JOB_DEBUG 1
    #else
        #define EGO_ENABLE_JOB_DEBUG 0
    #endif
#endif

#if EGO_ENABLE_JOB_DEBUG
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
        friend class JobController;

    public:
        Job() = default;
        Job(const char* _dbgName);
        ~Job() override = default;

        void wait();
        void execute();

        bool isFinished() const;

        JobControllerPointer getJobController() const
        {
            return m_jobController.lock();
        }
        void setExecutionContext(const JobControllerWeakPointer& _jobController);

        JobState getState() const
        {
            return m_state;
        }

#ifdef EGO_JOB_DEBUG
        const char* getDbgName() const
        {
            return m_dbgName;
        }
#endif

    protected:
        virtual void operate() = 0;

    private:
        bool trySetExecutionContext(const JobControllerWeakPointer& _jobController);
        bool tryExecute();

        mutable std::mutex m_mutex;
        std::condition_variable m_completionNotifier;

        JobControllerWeakPointer m_jobController;

        std::atomic<JobState> m_state = JobState::Undefined;

#ifdef EGO_JOB_DEBUG
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
        void operate() override;

        JobFunction m_function;
    };

    JobReference CreateLambdaJob(const LambdaJob::JobFunction& _function);
    JobReference CreateLambdaJob(const LambdaJob::JobFunction& _function, const char* _dbgName);
} // namespace ego
