#include "ResourceController.h"

#include <string>
#include <utility>
#include <vector>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Log/Log.h"
#include "EgoCore/UtilsMacros.h"

#include "ResourceLoadingContext.h"

ego::ResourceController::~ResourceController()
{
    release();
}

bool ego::ResourceController::ResourceControllerAccessor::RemoveResource(ResourceController& _controller, Resource* _resource)
{
    return _controller.removeResource(_resource);
}

bool ego::ResourceController::ResourceControllerAccessor::LoadResourceContent(const ResourceController& _controller, const FileName& _path, FileContent& _content)
{
    return _controller.loadResourceContent(_path, _content);
}

bool ego::ResourceController::ResourceControllerAccessor::AddDependency(ResourceController& _controller, Resource& _resource, const ResourcePointer& _dependency)
{
    return _controller.addDependency(_resource, _dependency);
}

bool ego::ResourceController::init(uint32_t _threadCount, const char* _jobThreadName)
{
    if (m_isInitialized)
    {
        return true;
    }

    uint32_t threadCount = _threadCount;
    if (threadCount == 0)
    {
        threadCount = JobController::GetHardwareThreadCount();
    }

    if (threadCount == 0)
    {
        threadCount = 1;
    }

    m_jobController = MakePointer<JobController>();
    EGO_CHECK_INITIALIZATION(m_jobController && m_jobController->init(threadCount, _jobThreadName));

    m_isInitialized = true;
    return true;
}

void ego::ResourceController::release()
{
    if (!m_isInitialized)
    {
        return;
    }

    waitAllLoading();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_jobController);

    m_resourceRegistry.clear();
    m_resourceSources.clear();
    m_dependencyGraph.clear();
    m_isInitialized = false;
}

bool ego::ResourceController::isInitialized() const
{
    return m_isInitialized;
}

void ego::ResourceController::addFileSystem(const FileSystemPointer& _fileSystem)
{
    m_resourceSources.addFileSystem(_fileSystem);
}

bool ego::ResourceController::removeFileSystem(const FileSystemPointer& _fileSystem)
{
    return m_resourceSources.removeFileSystem(_fileSystem);
}

void ego::ResourceController::clearFileSystems()
{
    m_resourceSources.clearFileSystems();
}

bool ego::ResourceController::addResourceProvider(const FileName& _extension, const ResourceProviderPointer& _provider)
{
    return m_resourceSources.addResourceProvider(_extension, _provider);
}

bool ego::ResourceController::removeResourceProvider(const FileName& _extension, const ResourceProviderPointer& _provider)
{
    return m_resourceSources.removeResourceProvider(_extension, _provider);
}

void ego::ResourceController::clearResourceProviders()
{
    m_resourceSources.clearResourceProviders();
}

ego::ResourcePointer ego::ResourceController::getResource(const FileName& _path) const
{
    return getResource(_path.hash());
}

ego::ResourcePointer ego::ResourceController::getResource(FileNameID _id) const
{
    return m_resourceRegistry.getResource(_id);
}

bool ego::ResourceController::removeResource(Resource* _resource)
{
    return m_resourceRegistry.removeResource(_resource);
}

void ego::ResourceController::waitAllLoading()
{
    if (!m_isInitialized)
    {
        return;
    }

    waitAllLoadingJobs();
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

bool ego::ResourceController::loadResourceContent(const FileName& _path, FileContent& _content) const
{
    return m_resourceSources.loadContent(_path, _content);
}

ego::ResourcePointer ego::ResourceController::loadResource(ResourceType _type, const FileName& _path, const ResourceFactory& _factory)
{
    if (!m_isInitialized)
    {
        EGO_ASSERT_FAIL_MESSAGE("ResourceController isn't initialized.");
        return nullptr;
    }

    bool needLoading = false;
    const ResourceID resourceID = _path.hash();
    ResourcePointer resource = m_resourceRegistry.getOrCreateResource(_type, _path, resourceID, _factory, needLoading);

    if (!resource)
    {
        return nullptr;
    }

    if (!needLoading)
    {
        return waitResourceLoading(resource) ? resource : nullptr;
    }

    if (!loadResourceData(resource, _path, false))
    {
        return nullptr;
    }

    return waitResourceLoading(resource) ? resource : nullptr;
}

ego::ResourceLoadingOperationPointer ego::ResourceController::loadResourceAsync(ResourceType _type, const FileName& _path, const ResourceFactory& _factory)
{
    if (!m_isInitialized)
    {
        EGO_ASSERT_FAIL_MESSAGE("ResourceController isn't initialized.");
        return nullptr;
    }

    bool needLoading = false;
    const ResourceID resourceID = _path.hash();
    ResourcePointer resource = m_resourceRegistry.getOrCreateResource(_type, _path, resourceID, _factory, needLoading);

    if (!resource)
    {
        return nullptr;
    }

    if (needLoading)
    {
        ResourceWeakPointer resourceWeakPointer = resource;

        const JobPointer loadingJob = CreateLambdaJob(
            [this, resourceWeakPointer, path = _path]()
            {
                ResourcePointer resource = resourceWeakPointer.lock();
                if (resource)
                {
                    loadResourceData(resource, path, true);
                }
            },
            "LoadResource");

        m_resourceRegistry.setLoadingJob(resource, loadingJob);
        m_jobController->addJob(loadingJob);
    }

    return MakePointer<ResourceLoadingOperation>(weakFromThis(), resource);
}

bool ego::ResourceController::readResourceContent(const ResourcePointer& _resource, const FileName& _path, ResourceLoadingContext& _loadingContext, FileContent& _content)
{
    const ResourceProviderPointer provider = m_resourceSources.getResourceProvider(_path);
    if (provider)
    {
        std::string loadingError;
        if (provider->provideContent(*_resource, _path, _loadingContext, _content, loadingError))
        {
            return true;
        }

        if (loadingError.empty())
        {
            loadingError = std::string("Resource provider failed to load content: ") + _path.c_str();
        }

        finishResourceFailed(_resource, std::move(loadingError));
        return false;
    }

    if (loadResourceContent(_path, _content))
    {
        return true;
    }

    finishResourceFailed(_resource, std::string("Failed to load resource content: ") + _path.c_str());
    return false;
}

bool ego::ResourceController::loadResourceData(const ResourcePointer& _resource, const FileName& _path, bool _isAsyncLoading)
{
    if (!_resource)
    {
        return false;
    }

    ResourceLoadingContext loadingContext(*this, *_resource, _isAsyncLoading);
    FileContent content;

    if (!readResourceContent(_resource, _path, loadingContext, content))
    {
        return false;
    }

    const bool isLoadSuccessful = _resource->load(_path, std::move(content), loadingContext);
    return completeResourceLoad(_resource, isLoadSuccessful, loadingContext);
}

bool ego::ResourceController::completeResourceLoad(const ResourcePointer& _resource, bool _isLoadSuccessful, ResourceLoadingContext& _loadingContext)
{
    if (!_resource)
    {
        return false;
    }

    if (!_isLoadSuccessful)
    {
        finishResourceFailed(_resource);
        return false;
    }

    if (!_loadingContext.hasAsyncDependencies())
    {
        finishResourceLoaded(_resource);
        return true;
    }

    if (beginResourceDependenciesLoading(*_resource, _loadingContext.takeDependencies()))
    {
        return true;
    }

    finishResourceFailed(_resource, std::string("Failed to begin resource dependencies loading: ") + _resource->getPath().c_str());
    return false;
}

void ego::ResourceController::finishResourceLoaded(const ResourcePointer& _resource)
{
    if (!_resource)
    {
        return;
    }

    Resource::ResourceAccessor::SetState(_resource.get(), ResourceState::Loaded);
    EGO_LOG_MESSAGE(std::string("Resource loaded successfully: ") + _resource->getPath().c_str());
    notifyResourceLoadingFinished(_resource);
}

void ego::ResourceController::finishResourceFailed(const ResourcePointer& _resource, std::string _loadingError)
{
    if (!_resource)
    {
        return;
    }

    if (!_loadingError.empty())
    {
        Resource::ResourceAccessor::SetLoadingError(_resource.get(), std::move(_loadingError));
    }

    const std::string loadingError = _resource->getLoadingError();
    if (!loadingError.empty())
    {
        EGO_LOG_ERROR(loadingError);
    }

    Resource::ResourceAccessor::SetState(_resource.get(), ResourceState::Failed);
    notifyResourceLoadingFinished(_resource);
}

bool ego::ResourceController::addDependency(Resource& _resource, const ResourcePointer& _dependency)
{
    return m_dependencyGraph.addDependency(_resource, _dependency);
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
    return m_dependencyGraph.isChildResourcesLoaded(_resource);
}

bool ego::ResourceController::waitLoading(const ResourcePointer& _resource)
{
    return waitResourceLoading(_resource);
}

bool ego::ResourceController::waitLoading(const FileName& _path)
{
    return waitLoading(getResource(_path));
}

bool ego::ResourceController::waitLoading(FileNameID _id)
{
    return waitLoading(getResource(_id));
}

bool ego::ResourceController::beginResourceDependenciesLoading(Resource& _resource, Resource::DependencyCollection&& _dependencies)
{
    ResourcePointer resource = m_resourceRegistry.getRegisteredResource(_resource);
    if (!resource)
    {
        return false;
    }

    const ResourceDependencyGraph::PendingLoadingPointer pendingLoading = m_dependencyGraph.createPendingLoading(_resource, resource, std::move(_dependencies));
    if (!pendingLoading)
    {
        return false;
    }

    Resource::ResourceAccessor::SetState(&_resource, ResourceState::LoadingDependencies);
    const uint32_t remainingDependencyCount = m_dependencyGraph.registerPendingDependencyWaiters(pendingLoading);

    if (remainingDependencyCount == 0)
    {
        schedulePendingLoadingCompletion(pendingLoading);
    }

    return true;
}

void ego::ResourceController::notifyResourceLoadingFinished(const ResourcePointer& _resource)
{
    for (const ResourceDependencyGraph::PendingLoadingPointer& pendingLoading : m_dependencyGraph.notifyResourceLoadingFinished(_resource))
    {
        schedulePendingLoadingCompletion(pendingLoading);
    }
}

void ego::ResourceController::schedulePendingLoadingCompletion(const ResourceDependencyGraph::PendingLoadingPointer& _pendingLoading)
{
    if (!_pendingLoading || _pendingLoading->m_isCompletionScheduled.exchange(true))
    {
        return;
    }

    const ResourcePointer resource = m_dependencyGraph.getPendingResource(_pendingLoading);
    if (!resource)
    {
        return;
    }

    const JobPointer completionJob = CreateLambdaJob(
        [this, _pendingLoading]()
        {
            completePendingLoading(_pendingLoading);
        },
        "CompleteResourceLoading");

    m_jobController->addJob(completionJob);
    m_resourceRegistry.setLoadingJob(resource, completionJob);
}

void ego::ResourceController::completePendingLoading(const ResourceDependencyGraph::PendingLoadingPointer& _pendingLoading)
{
    ResourceDependencyGraph::PendingLoadingCompletionResult result = m_dependencyGraph.completePendingLoading(_pendingLoading);
    if (!result.m_isCompleted)
    {
        return;
    }

    if (!result.m_areDependenciesLoaded)
    {
        finishResourceFailed(result.m_resource, std::move(result.m_loadingError));
        return;
    }

    if (Resource::ResourceAccessor::OnDependenciesLoaded(result.m_resource.get()))
    {
        finishResourceLoaded(result.m_resource);
        return;
    }

    std::string loadingError = result.m_resource->getLoadingError();
    if (loadingError.empty())
    {
        loadingError = std::string("Failed to complete resource loading: ") + result.m_resource->getPath().c_str();
    }

    finishResourceFailed(result.m_resource, std::move(loadingError));
}

bool ego::ResourceController::waitResourceLoading(const ResourcePointer& _resource)
{
    while (_resource && _resource->isLoading())
    {
        std::vector<JobPointer> loadingJobs;
        m_dependencyGraph.collectResourceLoadingJobs(
            _resource,
            [this](const ResourcePointer& _loadingResource)
            {
                return m_resourceRegistry.getLoadingJob(_loadingResource);
            },
            loadingJobs);

        if (loadingJobs.empty())
        {
            break;
        }

        waitLoadingJobs(loadingJobs);
    }

    return _resource && _resource->isLoaded();
}

void ego::ResourceController::waitLoadingJobs(const std::vector<JobPointer>& _jobs)
{
    for (const JobPointer& job : _jobs)
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
        std::vector<JobPointer> loadingJobs;
        m_resourceRegistry.collectLoadingJobs(loadingJobs);

        if (loadingJobs.empty())
        {
            return;
        }

        waitLoadingJobs(loadingJobs);
    }
}
