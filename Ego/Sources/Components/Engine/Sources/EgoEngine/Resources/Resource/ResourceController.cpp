#include "ResourceController.h"

#include "ResourceLoadingContext.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/UtilsMacros.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

namespace
{
    std::string NormalizeProviderExtension(const ego::FileName& _extension)
    {
        std::string extension = _extension.c_str();
        if (extension.empty())
        {
            return extension;
        }

        if (extension.front() != '.')
        {
            extension.insert(extension.begin(), '.');
        }

        std::transform(
            extension.begin(),
            extension.end(),
            extension.begin(),
            [](unsigned char _ch)
            {
                return static_cast<char>(std::tolower(_ch));
            }
        );
        return extension;
    }

    std::string GetProviderExtension(const ego::FileName& _path)
    {
        return NormalizeProviderExtension(ego::file_name_utils::GetFileExtension(_path));
    }
}

ego::ResourceController::~ResourceController()
{
    release();
}

bool ego::ResourceController::ResourceControllerAccessor::RemoveResource(
    ResourceController* _controller,
    Resource* _resource
)
{
    EGO_ASSERT(_controller);
    return _controller->removeResource(_resource);
}

bool ego::ResourceController::ResourceControllerAccessor::LoadResourceContent(
    const ResourceController* _controller,
    const FileName& _path,
    FileContent& _content
)
{
    EGO_ASSERT(_controller);
    return _controller->loadResourceContent(_path, _content);
}

bool ego::ResourceController::init(uint32_t _threadCount, const char* _jobThreadName)
{
    if (m_isInitialized)
    {
        return true;
    }

    const uint32_t threadCount = _threadCount != 0
        ? _threadCount
        : 1;

    m_jobController = new JobController();
    EGO_CHECK_INITIALIZATION(m_jobController && m_jobController->init(threadCount != 0 ? threadCount : 1, _jobThreadName));

    m_isInitialized = true;
    return true;
}

void ego::ResourceController::release()
{
    if (!m_isInitialized)
    {
        return;
    }

    waitAllLoadingJobs();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_jobController);

    std::lock_guard locker(m_mutex);
    m_resources.clear();
    m_fileSystems.clear();
    m_resourceProviders.clear();
    m_isInitialized = false;
}

bool ego::ResourceController::isInitialized() const
{
    return m_isInitialized;
}

void ego::ResourceController::addFileSystem(const FileSystemPointer& _fileSystem)
{
    if (!_fileSystem)
    {
        return;
    }

    std::lock_guard locker(m_mutex);

    const auto foundIt = std::find_if(
        m_fileSystems.begin(),
        m_fileSystems.end(),
        [&_fileSystem](const FileSystemPointer& _registeredFileSystem)
        {
            return _registeredFileSystem.get() == _fileSystem.get();
        }
    );
    if (foundIt != m_fileSystems.end())
    {
        return;
    }

    m_fileSystems.push_back(_fileSystem);
}

bool ego::ResourceController::removeFileSystem(const FileSystemPointer& _fileSystem)
{
    if (!_fileSystem)
    {
        return false;
    }

    std::lock_guard locker(m_mutex);

    const auto foundIt = std::find_if(
        m_fileSystems.begin(),
        m_fileSystems.end(),
        [&_fileSystem](const FileSystemPointer& _registeredFileSystem)
        {
            return _registeredFileSystem.get() == _fileSystem.get();
        }
    );
    if (foundIt == m_fileSystems.end())
    {
        return false;
    }

    m_fileSystems.erase(foundIt);
    return true;
}

void ego::ResourceController::clearFileSystems()
{
    std::lock_guard locker(m_mutex);
    m_fileSystems.clear();
}

bool ego::ResourceController::addResourceProvider(
    const FileName& _extension,
    const ResourceProviderPointer& _provider
)
{
    const std::string extension = NormalizeProviderExtension(_extension);
    if (extension.empty() || !_provider)
    {
        return false;
    }

    std::lock_guard locker(m_mutex);

    auto [providerIt, added] = m_resourceProviders.emplace(extension, _provider);
    if (!added)
    {
        providerIt->second = _provider;
    }

    return true;
}

bool ego::ResourceController::removeResourceProvider(const FileName& _extension)
{
    const std::string extension = NormalizeProviderExtension(_extension);
    if (extension.empty())
    {
        return false;
    }

    std::lock_guard locker(m_mutex);
    return m_resourceProviders.erase(extension) != 0;
}

void ego::ResourceController::clearResourceProviders()
{
    std::lock_guard locker(m_mutex);
    m_resourceProviders.clear();
}

ego::ResourcePointer ego::ResourceController::loadResource(
    ResourceType _type,
    const FileName& _path,
    const ResourceFactory& _factory
)
{
    if (!m_isInitialized)
    {
        EGO_ASSERT_FAIL_MESSAGE("ResourceController isn't initialized.");
        return nullptr;
    }

    bool needLoading = false;
    const ResourceID resourceID = _path.hash();
    ResourcePointer resource = getOrCreateResource(_type, _path, resourceID, _factory, needLoading);

    if (!resource)
    {
        return nullptr;
    }

    if (!needLoading)
    {
        JobReference loadingJob;
        {
            std::lock_guard locker(m_mutex);
            const auto resourceIt = m_resources.find(resourceID);
            if (resourceIt != m_resources.end())
            {
                loadingJob = resourceIt->second.m_loadingJob.lock();
            }
        }

        if (loadingJob && !loadingJob->isFinished())
        {
            m_jobController->wait(loadingJob);
        }

        return resource->isLoaded() ? resource : nullptr;
    }

    return loadResourceData(resource, _path, false) ? resource : nullptr;
}

ego::ResourcePointer ego::ResourceController::loadResourceAsync(
    ResourceType _type,
    const FileName& _path,
    const ResourceFactory& _factory,
    JobReference* _job
)
{
    if (_job)
    {
        *_job = JobReference();
    }

    if (!m_isInitialized)
    {
        EGO_ASSERT_FAIL_MESSAGE("ResourceController isn't initialized.");
        return nullptr;
    }

    bool needLoading = false;
    const ResourceID resourceID = _path.hash();
    ResourcePointer resource = getOrCreateResource(_type, _path, resourceID, _factory, needLoading);

    if (!resource)
    {
        return nullptr;
    }

    JobReference loadingJob;

    if (needLoading)
    {
        ResourceWeakPointer resourceWeakPointer = resource;

        loadingJob = CreateLambdaJob(
            [this, resourceWeakPointer, path = _path]()
            {
                ResourcePointer resource = resourceWeakPointer.lock();
                if (resource)
                {
                    loadResourceData(resource, path, true);
                }
            },
            "LoadResource"
        );

        {
            std::lock_guard locker(m_mutex);
            auto resourceIt = m_resources.find(resourceID);
            ResourcePointer storedResource = resourceIt != m_resources.end()
                ? resourceIt->second.m_resource.lock()
                : nullptr;
            if (resourceIt != m_resources.end() && storedResource.get() == resource.get())
            {
                resourceIt->second.m_loadingJob = loadingJob;
            }
        }

        m_jobController->addJob(loadingJob);
    }
    else
    {
        std::lock_guard locker(m_mutex);
        const auto resourceIt = m_resources.find(resourceID);
        if (resourceIt != m_resources.end())
        {
            loadingJob = resourceIt->second.m_loadingJob.lock();
        }
    }

    if (_job)
    {
        *_job = loadingJob;
    }

    return resource;
}

ego::ResourcePointer ego::ResourceController::getResource(const FileName& _path) const
{
    return getResource(_path.hash());
}

ego::ResourcePointer ego::ResourceController::getResource(FileNameID _id) const
{
    std::lock_guard locker(m_mutex);
    const auto resourceIt = m_resources.find(_id);

    if (resourceIt == m_resources.end())
    {
        return nullptr;
    }

    return resourceIt->second.m_resource.lock();
}

bool ego::ResourceController::removeResource(Resource* _resource)
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

bool ego::ResourceController::isChildResourcesLoaded(const FileName& _path) const
{
    return isChildResourcesLoaded(getResource(_path));
}

bool ego::ResourceController::isChildResourcesLoaded(FileNameID _id) const
{
    return isChildResourcesLoaded(getResource(_id));
}

bool ego::ResourceController::isChildResourcesLoaded(const ResourcePointer& _resource) const
{
    std::unordered_set<const Resource*> checkedResources;
    checkedResources.insert(_resource.get());

    return isChildResourcesLoaded(_resource, checkedResources);
}

const ego::JobController& ego::ResourceController::getJobController() const
{
    EGO_ASSERT(m_jobController);
    return *m_jobController;
}

ego::JobController& ego::ResourceController::getJobController()
{
    EGO_ASSERT(m_jobController);
    return *m_jobController;
}

ego::ResourcePointer ego::ResourceController::getOrCreateResource(
    ResourceType _type,
    const FileName& _path,
    FileNameID _id,
    const ResourceFactory& _factory,
    bool& _needLoading
)
{
    _needLoading = false;

    if (_type == InvalidResourceType || !_path || _id == InvalidFileNameID)
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

bool ego::ResourceController::loadResourceContent(const FileName& _path, FileContent& _content) const
{
    FileSystemCollection fileSystems;

    {
        std::lock_guard locker(m_mutex);
        fileSystems = m_fileSystems;
    }

    for (const FileSystemPointer& fileSystem : fileSystems)
    {
        if (fileSystem && fileSystem->readFile(_path, _content))
        {
            return true;
        }
    }

    _content.clear();
    return false;
}

ego::ResourceProviderPointer ego::ResourceController::getResourceProvider(const FileName& _path) const
{
    const std::string extension = GetProviderExtension(_path);
    if (extension.empty())
    {
        return nullptr;
    }

    std::lock_guard locker(m_mutex);

    const auto providerIt = m_resourceProviders.find(extension);
    return providerIt != m_resourceProviders.end() ? providerIt->second : nullptr;
}

bool ego::ResourceController::loadResourceData(
    const ResourcePointer& _resource,
    const FileName& _path,
    bool _asyncChildLoading
)
{
    if (!_resource)
    {
        return false;
    }

    ResourceLoadingContext loadingContext(*this, *_resource, _asyncChildLoading);
    FileContent content;

    const ResourceProviderPointer provider = getResourceProvider(_path);
    if (provider)
    {
        if (!provider->provideContent(*_resource, _path, loadingContext, content))
        {
            Resource::ResourceAccessor::SetState(_resource.get(), ResourceState::Failed);
            return false;
        }
    }
    else if (!loadResourceContent(_path, content))
    {
        Resource::ResourceAccessor::SetState(_resource.get(), ResourceState::Failed);
        return false;
    }

    if (!_resource->load(_path, std::move(content), loadingContext))
    {
        return false;
    }

    return true;
}

bool ego::ResourceController::isChildResourcesLoaded(
    const ResourcePointer& _resource,
    std::unordered_set<const Resource*>& _checkedResources
) const
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

void ego::ResourceController::collectLoadingJobs(std::vector<JobReference>& _jobs) const
{
    std::lock_guard locker(m_mutex);

    for (const auto& resourceEntry : m_resources)
    {
        const JobReference loadingJob = resourceEntry.second.m_loadingJob.lock();
        if (loadingJob && !loadingJob->isFinished())
        {
            _jobs.push_back(loadingJob);
        }
    }
}

void ego::ResourceController::waitLoadingJobs(const std::vector<JobReference>& _jobs)
{
    for (const JobReference& job : _jobs)
    {
        if (job && !job->isFinished())
        {
            m_jobController->waitAndExecute(job);
        }
    }
}

void ego::ResourceController::waitAllLoadingJobs()
{
    while (true)
    {
        std::vector<JobReference> loadingJobs;
        collectLoadingJobs(loadingJobs);

        if (loadingJobs.empty())
        {
            return;
        }

        waitLoadingJobs(loadingJobs);
    }
}
