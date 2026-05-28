#include "JobDescriptor.h"

#include "EgoCore/Assert/AssertCore.h"

ego::JobDescriptor::JobDescriptor(const JobFactory& _factory)
    : m_factory(_factory)
{}

ego::JobReference ego::JobDescriptor::createJob() const
{
    if (!m_factory)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job descriptor factory is null.");
        return nullptr;
    }

    JobReference job = m_factory();
    if (!job)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job descriptor factory returned null job.");
    }

    return job;
}

bool ego::JobDescriptor::isValid() const
{
    return static_cast<bool>(m_factory);
}

ego::JobDescriptor::operator bool() const
{
    return isValid();
}

ego::JobDescriptor ego::CreateJobDescriptor(const LambdaJob::JobFunction& _function, const char* _dbgName)
{
    if (!_function)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job descriptor lambda is null.");
        return JobDescriptor();
    }

    return JobDescriptor(
        [_function, _dbgName]()
        {
            return _dbgName
                ? CreateLambdaJob(_function, _dbgName)
                : CreateLambdaJob(_function);
        }
    );
}
