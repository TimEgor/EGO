#include "JobDescriptor.h"

#include "EgoCore/Assert/AssertCore.h"

ego::JobDescriptor::JobDescriptor(JobFactory _factory)
    : m_factory(std::move(_factory))
{
}

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

ego::JobDescriptor ego::CreateJobDescriptor(JobDescriptor::JobFactory _factory)
{
    if (!_factory)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job descriptor factory is null.");
        return JobDescriptor();
    }

    return JobDescriptor(std::move(_factory));
}

ego::JobDescriptor ego::CreateJobDescriptor(const LambdaJob::JobFunction& _function, const char* _dbgName)
{
    if (!_function)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job descriptor lambda is null.");
        return JobDescriptor();
    }

    return CreateJobDescriptor(
        JobDescriptor::JobFactory(
            [_function, _dbgName]()
            {
                return _dbgName ? CreateLambdaJob(_function, _dbgName) : CreateLambdaJob(_function);
            }));
}

ego::JobDescriptor ego::CreateEmptyJobDescriptor(const char* _dbgName)
{
    return CreateJobDescriptor([]() {}, _dbgName);
}
