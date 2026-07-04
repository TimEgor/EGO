#include "ResourceDependencyGraph.h"

#include <algorithm>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "EgoCore/Assert/AssertCore.h"

bool ego::ResourceDependencyGraph::addDependency(Resource& _resource, const ResourcePointer& _dependency) const
{
    if (!_dependency)
    {
        return false;
    }

    if (hasDependencyCycle(_resource, _dependency))
    {
        return false;
    }

    Resource::ResourceAccessor::AddDependency(&_resource, _dependency);
    return true;
}

ego::ResourceDependencyGraph::PendingLoadingPointer ego::ResourceDependencyGraph::createPendingLoading(
    Resource& _resource,
    const ResourcePointer& _registeredResource,
    Resource::DependencyCollection&& _dependencies) const
{
    if (!_registeredResource || _registeredResource.get() != &_resource)
    {
        EGO_ASSERT_FAIL_MESSAGE("Resource isn't registered in resource controller.");
        return nullptr;
    }

    auto pendingLoading = std::make_shared<PendingLoading>();
    pendingLoading->m_resource = _registeredResource;
    pendingLoading->m_dependencies = std::move(_dependencies);

    normalizeDependencies(pendingLoading->m_dependencies);

    if (hasDependencyCycle(_resource, pendingLoading->m_dependencies))
    {
        return nullptr;
    }

    return pendingLoading;
}

uint32_t ego::ResourceDependencyGraph::registerPendingDependencyWaiters(const PendingLoadingPointer& _pendingLoading)
{
    if (!_pendingLoading)
    {
        return 0;
    }

    uint32_t remainingDependencyCount = 0;
    std::lock_guard locker(m_mutex);

    for (const ResourcePointer& dependency : _pendingLoading->m_dependencies)
    {
        if (!dependency || !dependency->isLoading())
        {
            continue;
        }

        m_pendingLoadingWaiters[dependency.get()].push_back(_pendingLoading);
        ++remainingDependencyCount;
    }

    _pendingLoading->m_remainingDependencyCount = remainingDependencyCount;
    return remainingDependencyCount;
}

ego::ResourceDependencyGraph::PendingLoadingCollection ego::ResourceDependencyGraph::notifyResourceLoadingFinished(const ResourcePointer& _resource)
{
    if (!_resource || _resource->isLoading())
    {
        return {};
    }

    PendingLoadingCollection pendingLoadings;
    {
        std::lock_guard locker(m_mutex);

        const auto pendingLoadingIt = m_pendingLoadingWaiters.find(_resource.get());
        if (pendingLoadingIt == m_pendingLoadingWaiters.end())
        {
            return {};
        }

        pendingLoadings.swap(pendingLoadingIt->second);
        m_pendingLoadingWaiters.erase(pendingLoadingIt);
    }

    return finishDependencyLoadings(std::move(pendingLoadings));
}

ego::ResourceDependencyGraph::PendingLoadingCompletionResult ego::ResourceDependencyGraph::completePendingLoading(const PendingLoadingPointer& _pendingLoading) const
{
    PendingLoadingCompletionResult result;
    if (!_pendingLoading)
    {
        return result;
    }

    result.m_resource = _pendingLoading->m_resource.lock();
    if (!result.m_resource || !result.m_resource->isLoadingDependencies())
    {
        result.m_resource = nullptr;
        return result;
    }

    result.m_isCompleted = true;

    if (!checkPendingDependenciesLoaded(_pendingLoading, result.m_loadingError))
    {
        return result;
    }

    result.m_areDependenciesLoaded = true;
    return result;
}

ego::ResourcePointer ego::ResourceDependencyGraph::getPendingResource(const PendingLoadingPointer& _pendingLoading) const
{
    return _pendingLoading ? _pendingLoading->m_resource.lock() : nullptr;
}

void ego::ResourceDependencyGraph::collectResourceLoadingJobs(
    const ResourcePointer& _resource,
    const LoadingJobResolver& _loadingJobResolver,
    std::vector<JobReference>& _jobs) const
{
    std::unordered_set<const Resource*> checkedResources;
    collectResourceLoadingJobs(_resource, _loadingJobResolver, checkedResources, _jobs);
}

bool ego::ResourceDependencyGraph::isChildResourcesLoaded(const ResourcePointer& _resource) const
{
    std::unordered_set<const Resource*> checkedResources;
    checkedResources.insert(_resource.get());

    return isChildResourcesLoaded(_resource, checkedResources);
}

void ego::ResourceDependencyGraph::clear()
{
    std::lock_guard locker(m_mutex);
    m_pendingLoadingWaiters.clear();
}

void ego::ResourceDependencyGraph::normalizeDependencies(Resource::DependencyCollection& _dependencies) const
{
    std::unordered_set<const Resource*> uniqueDependencies;
    _dependencies.erase(
        std::remove_if(
            _dependencies.begin(),
            _dependencies.end(),
            [&uniqueDependencies](const ResourcePointer& _dependency)
            {
                return !uniqueDependencies.insert(_dependency.get()).second;
            }),
        _dependencies.end());
}

ego::ResourceDependencyGraph::PendingLoadingCollection ego::ResourceDependencyGraph::finishDependencyLoadings(PendingLoadingCollection&& _pendingLoadings) const
{
    PendingLoadingCollection finishedLoadings;
    for (const PendingLoadingPointer& pendingLoading : _pendingLoadings)
    {
        if (!pendingLoading)
        {
            continue;
        }

        const uint32_t previousDependencyCount = pendingLoading->m_remainingDependencyCount.fetch_sub(1);
        if (previousDependencyCount == 0)
        {
            pendingLoading->m_remainingDependencyCount = 0;
            EGO_ASSERT_FAIL_MESSAGE("Pending resource loading dependency counter is invalid.");
            continue;
        }

        if (previousDependencyCount == 1)
        {
            finishedLoadings.push_back(pendingLoading);
        }
    }

    return finishedLoadings;
}

bool ego::ResourceDependencyGraph::checkPendingDependenciesLoaded(const PendingLoadingPointer& _pendingLoading, std::string& _loadingError) const
{
    if (!_pendingLoading)
    {
        _loadingError = "Pending resource loading is null.";
        return false;
    }

    for (const ResourcePointer& dependency : _pendingLoading->m_dependencies)
    {
        if (dependency && dependency->isLoaded())
        {
            continue;
        }

        if (!dependency)
        {
            _loadingError = "Resource dependency is null.";
            return false;
        }

        _loadingError = std::string("Resource dependency failed to load: ") + dependency->getPath().c_str();

        const std::string dependencyLoadingError = dependency->getLoadingError();
        if (!dependencyLoadingError.empty())
        {
            _loadingError += ". ";
            _loadingError += dependencyLoadingError;
        }

        return false;
    }

    return true;
}

void ego::ResourceDependencyGraph::collectResourceLoadingJobs(
    const ResourcePointer& _resource,
    const LoadingJobResolver& _loadingJobResolver,
    std::unordered_set<const Resource*>& _checkedResources,
    std::vector<JobReference>& _jobs) const
{
    if (!_resource || !_resource->isLoading() || !_checkedResources.insert(_resource.get()).second)
    {
        return;
    }

    const JobReference loadingJob = _loadingJobResolver ? _loadingJobResolver(_resource) : JobReference();
    if (loadingJob && loadingJob->getState() != JobState::Undefined && !loadingJob->isFinished())
    {
        _jobs.push_back(loadingJob);
    }

    Resource::DependencyCollection dependencies;
    Resource::ResourceAccessor::GetDependencies(_resource.get(), dependencies);

    for (const ResourcePointer& dependency : dependencies)
    {
        collectResourceLoadingJobs(dependency, _loadingJobResolver, _checkedResources, _jobs);
    }
}

bool ego::ResourceDependencyGraph::hasDependencyCycle(const Resource& _resource, const Resource::DependencyCollection& _dependencies) const
{
    for (const ResourcePointer& dependency : _dependencies)
    {
        if (hasDependencyCycle(_resource, dependency))
        {
            return true;
        }
    }

    return false;
}

bool ego::ResourceDependencyGraph::hasDependencyCycle(const Resource& _resource, const ResourcePointer& _dependency) const
{
    if (!_dependency)
    {
        return false;
    }

    if (_dependency.get() == &_resource)
    {
        EGO_ASSERT_FAIL_MESSAGE("Resource dependency cycle detected.");
        return true;
    }

    std::unordered_set<const Resource*> checkedResources;
    checkedResources.insert(&_resource);
    if (hasDependencyPath(_dependency, &_resource, checkedResources))
    {
        EGO_ASSERT_FAIL_MESSAGE("Resource dependency cycle detected.");
        return true;
    }

    return false;
}

bool ego::ResourceDependencyGraph::hasDependencyPath(
    const ResourcePointer& _resource,
    const Resource* _targetResource,
    std::unordered_set<const Resource*>& _checkedResources) const
{
    if (!_resource || !_targetResource)
    {
        return false;
    }

    if (_resource.get() == _targetResource)
    {
        return true;
    }

    if (!_checkedResources.insert(_resource.get()).second)
    {
        return false;
    }

    Resource::DependencyCollection dependencies;
    Resource::ResourceAccessor::GetDependencies(_resource.get(), dependencies);

    for (const ResourcePointer& dependency : dependencies)
    {
        if (hasDependencyPath(dependency, _targetResource, _checkedResources))
        {
            return true;
        }
    }

    return false;
}

bool ego::ResourceDependencyGraph::isChildResourcesLoaded(const ResourcePointer& _resource, std::unordered_set<const Resource*>& _checkedResources) const
{
    if (!_resource || !_resource->isLoaded())
    {
        return false;
    }

    Resource::DependencyCollection dependencies;
    Resource::ResourceAccessor::GetDependencies(_resource.get(), dependencies);

    for (const ResourcePointer& dependency : dependencies)
    {
        if (!dependency || !dependency->isLoaded())
        {
            return false;
        }

        if (!_checkedResources.insert(dependency.get()).second)
        {
            continue;
        }

        if (!isChildResourcesLoaded(dependency, _checkedResources))
        {
            return false;
        }
    }

    return true;
}
