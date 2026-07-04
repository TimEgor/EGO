#include "ResourceRegistry.h"

#include <mutex>

#include "EgoCore/Assert/AssertCore.h"

ego::ResourcePointer ego::ResourceRegistry::getResource(FileNameID _id) const
{
    std::lock_guard locker(m_mutex);
    const auto resourceIt = m_resources.find(_id);

    if (resourceIt == m_resources.end())
    {
        return nullptr;
    }

    return resourceIt->second.m_resource.lock();
}

ego::ResourcePointer ego::ResourceRegistry::getRegisteredResource(Resource& _resource) const
{
    const FileName resourcePath = _resource.getPath();
    const ResourceID resourceID = resourcePath.hash();
    if (!resourcePath || resourceID == InvalidResourceID)
    {
        EGO_ASSERT_FAIL_MESSAGE("Resource path is invalid.");
        return nullptr;
    }

    std::lock_guard locker(m_mutex);

    const auto resourceIt = m_resources.find(resourceID);
    ResourcePointer resource = resourceIt != m_resources.end() ? resourceIt->second.m_resource.lock() : nullptr;

    if (!resource || resource.get() != &_resource)
    {
        EGO_ASSERT_FAIL_MESSAGE("Resource isn't registered in resource controller.");
        return nullptr;
    }

    return resource;
}

ego::ResourcePointer ego::ResourceRegistry::getOrCreateResource(ResourceType _type, const FileName& _path, ResourceID _id, const ResourceFactory& _factory, bool& _needLoading)
{
    _needLoading = false;

    if (_type == InvalidResourceType || !_path || _id == InvalidResourceID)
    {
        EGO_ASSERT_FAIL_MESSAGE("Resource name is invalid.");
        return nullptr;
    }

    if (!_factory)
    {
        EGO_ASSERT_FAIL_MESSAGE("Resource factory is null.");
        return nullptr;
    }

    std::lock_guard locker(m_mutex);

    auto resourceIt = m_resources.find(_id);
    if (resourceIt != m_resources.end())
    {
        ResourcePointer resource = resourceIt->second.m_resource.lock();
        if (!resource)
        {
            m_resources.erase(resourceIt);
            _needLoading = true;
        }
        else
        {
            if (resource->getType() != _type)
            {
                EGO_ASSERT_FAIL_MESSAGE("Resource with requested name has different type.");
                return nullptr;
            }

            const JobReference loadingJob = resourceIt->second.m_loadingJob.lock();
            if (loadingJob && !loadingJob->isFinished())
            {
                return resource;
            }

            if (resource->isLoaded() || resource->isLoading())
            {
                return resource;
            }

            Resource::ResourceAccessor::PrepareLoading(resource.get(), _path);
            resourceIt->second.m_loadingJob.reset();
            _needLoading = true;
            return resource;
        }
    }

    ResourcePointer resource = _factory();
    if (!resource)
    {
        EGO_ASSERT_FAIL_MESSAGE("Resource factory returned null.");
        return nullptr;
    }

    if (resource->getType() != _type)
    {
        EGO_ASSERT_FAIL_MESSAGE("Resource factory returned unexpected resource type.");
        return nullptr;
    }

    Resource::ResourceAccessor::PrepareLoading(resource.get(), _path);

    m_resources.emplace(_id, ResourceEntry{_type, _path, _factory, resource, resource.get(), nullptr});
    _needLoading = true;

    return resource;
}

bool ego::ResourceRegistry::removeResource(Resource* _resource)
{
    if (!_resource)
    {
        return false;
    }

    const FileName resourcePath = _resource->getPath();
    if (!resourcePath)
    {
        return false;
    }

    const ResourceID resourceID = resourcePath.hash();
    if (resourceID == InvalidResourceID)
    {
        return false;
    }

    std::lock_guard locker(m_mutex);

    const auto resourceIt = m_resources.find(resourceID);
    if (resourceIt == m_resources.end() || resourceIt->second.m_resourcePtr != _resource)
    {
        return false;
    }

    m_resources.erase(resourceIt);
    return true;
}

void ego::ResourceRegistry::setLoadingJob(const ResourcePointer& _resource, const JobReference& _job)
{
    if (!_resource)
    {
        return;
    }

    const FileName resourcePath = _resource->getPath();
    const ResourceID resourceID = resourcePath.hash();
    if (!resourcePath || resourceID == InvalidResourceID)
    {
        return;
    }

    std::lock_guard locker(m_mutex);

    const auto resourceIt = m_resources.find(resourceID);
    if (resourceIt != m_resources.end() && resourceIt->second.m_resourcePtr == _resource.get())
    {
        resourceIt->second.m_loadingJob = _job;
    }
}

ego::JobReference ego::ResourceRegistry::getLoadingJob(const ResourcePointer& _resource) const
{
    if (!_resource)
    {
        return nullptr;
    }

    const FileName resourcePath = _resource->getPath();
    const ResourceID resourceID = resourcePath.hash();
    if (!resourcePath || resourceID == InvalidResourceID)
    {
        return nullptr;
    }

    std::lock_guard locker(m_mutex);

    const auto resourceIt = m_resources.find(resourceID);
    return resourceIt != m_resources.end() && resourceIt->second.m_resourcePtr == _resource.get() ? resourceIt->second.m_loadingJob.lock() : nullptr;
}

void ego::ResourceRegistry::collectLoadingJobs(std::vector<JobReference>& _jobs) const
{
    std::lock_guard locker(m_mutex);

    for (const auto& resourceEntry : m_resources)
    {
        const JobReference loadingJob = resourceEntry.second.m_loadingJob.lock();
        if (loadingJob && loadingJob->getState() != JobState::Undefined && !loadingJob->isFinished())
        {
            _jobs.push_back(loadingJob);
        }
    }
}

void ego::ResourceRegistry::clear()
{
    std::lock_guard locker(m_mutex);
    m_resources.clear();
}
