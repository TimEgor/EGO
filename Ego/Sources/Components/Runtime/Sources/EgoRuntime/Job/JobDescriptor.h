#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Job.h"

namespace ego
{
    class JobDescriptor final
    {
    public:
        using JobFactory = std::function<JobReference()>;

        JobDescriptor() = default;
        explicit JobDescriptor(JobFactory _factory);

        JobReference createJob() const;

        bool isValid() const;
        explicit operator bool() const;

    private:
        JobFactory m_factory;
    };

    JobDescriptor CreateJobDescriptor(JobDescriptor::JobFactory _factory);

    template <typename TFactory>
    std::enable_if_t<std::is_invocable_r_v<JobReference, TFactory>, JobDescriptor> CreateJobDescriptor(TFactory _factory)
    {
        return CreateJobDescriptor(JobDescriptor::JobFactory(std::move(_factory)));
    }

    template <typename TJob, typename... TArgs>
    std::enable_if_t<std::is_base_of_v<Job, TJob>, JobDescriptor> CreateJobDescriptor(TArgs&&... _args)
    {
        static_assert(!std::is_abstract_v<TJob>, "Job type must not be abstract.");
        static_assert(std::is_constructible_v<TJob, std::decay_t<TArgs>&...>, "Job type isn't constructible from stored arguments.");
        static_assert((std::is_copy_constructible_v<std::decay_t<TArgs>> && ...), "Job descriptor arguments must be copy constructible.");

        using ArgsTuple = std::tuple<std::decay_t<TArgs>...>;
        ArgsTuple args(std::forward<TArgs>(_args)...);

        return CreateJobDescriptor(
            JobDescriptor::JobFactory(
                [args = std::move(args)]() mutable -> JobReference
                {
                    return std::apply(
                        [](auto&... _storedArgs) -> JobReference
                        {
                            return JobReference(new TJob(_storedArgs...));
                        },
                        args);
                }));
    }

    JobDescriptor CreateJobDescriptor(const LambdaJob::JobFunction& _function, const char* _dbgName = nullptr);
    JobDescriptor CreateEmptyJobDescriptor(const char* _dbgName = nullptr);
} // namespace ego
