#pragma once

#include "Job.h"

#include <functional>

namespace ego
{
    class JobDescriptor final
    {
    public:
        using JobFactory = std::function<JobReference()>;

        JobDescriptor() = default;
        explicit JobDescriptor(const JobFactory& _factory);

        JobReference createJob() const;

        bool isValid() const;
        explicit operator bool() const;

    private:
        JobFactory m_factory;
    };

    JobDescriptor CreateJobDescriptor(const LambdaJob::JobFunction& _function, const char* _dbgName = nullptr);
}
