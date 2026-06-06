#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "EgoCore/Job/Job.h"

#include "Resource.h"

namespace ego
{
    class ResourceDependencyGraph final
    {
    public:
        struct PendingLoading final
        {
            ResourceWeakPointer m_resource;
            Resource::DependencyCollection m_dependencies;
            std::atomic<uint32_t> m_remainingDependencyCount = 0;
            std::atomic_bool m_isCompletionScheduled = false;
        };

        struct PendingLoadingCompletionResult final
        {
            ResourcePointer m_resource;
            std::string m_loadingError;
            bool m_isCompleted = false;
            bool m_areDependenciesLoaded = false;
        };

        using PendingLoadingPointer = std::shared_ptr<PendingLoading>;
        using PendingLoadingCollection = std::vector<PendingLoadingPointer>;
        using LoadingJobResolver = std::function<JobReference(const ResourcePointer& _resource)>;

        bool addDependency(Resource& _resource, const ResourcePointer& _dependency) const;
        PendingLoadingPointer createPendingLoading(
            Resource& _resource,
            const ResourcePointer& _registeredResource,
            Resource::DependencyCollection&& _dependencies
        ) const;
        uint32_t registerPendingDependencyWaiters(const PendingLoadingPointer& _pendingLoading);
        PendingLoadingCollection notifyResourceLoadingFinished(const ResourcePointer& _resource);
        PendingLoadingCompletionResult completePendingLoading(const PendingLoadingPointer& _pendingLoading) const;
        ResourcePointer getPendingResource(const PendingLoadingPointer& _pendingLoading) const;

        void collectResourceLoadingJobs(
            const ResourcePointer& _resource,
            const LoadingJobResolver& _loadingJobResolver,
            std::vector<JobReference>& _jobs
        ) const;
        bool isChildResourcesLoaded(const ResourcePointer& _resource) const;
        void clear();

    private:
        using PendingLoadingWaiterCollection = std::unordered_map<const Resource*, PendingLoadingCollection>;

        void normalizeDependencies(Resource::DependencyCollection& _dependencies) const;
        PendingLoadingCollection finishDependencyLoadings(PendingLoadingCollection&& _pendingLoadings) const;
        bool checkPendingDependenciesLoaded(
            const PendingLoadingPointer& _pendingLoading,
            std::string& _loadingError
        ) const;
        void collectResourceLoadingJobs(
            const ResourcePointer& _resource,
            const LoadingJobResolver& _loadingJobResolver,
            std::unordered_set<const Resource*>& _checkedResources,
            std::vector<JobReference>& _jobs
        ) const;
        bool hasDependencyCycle(
            const Resource& _resource,
            const Resource::DependencyCollection& _dependencies
        ) const;
        bool hasDependencyCycle(const Resource& _resource, const ResourcePointer& _dependency) const;
        bool hasDependencyPath(
            const ResourcePointer& _resource,
            const Resource* _targetResource,
            std::unordered_set<const Resource*>& _checkedResources
        ) const;
        bool isChildResourcesLoaded(
            const ResourcePointer& _resource,
            std::unordered_set<const Resource*>& _checkedResources
        ) const;

        mutable std::mutex m_mutex;
        PendingLoadingWaiterCollection m_pendingLoadingWaiters;
    };
}
