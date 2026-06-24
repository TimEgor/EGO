#include "Resource.h"

#include "EgoEngine/Engine.h"
#include "ResourceController.h"

#include "EgoCore/Assert/AssertCore.h"

#include <algorithm>
#include <utility>

ego::Resource::~Resource() = default;

void ego::ResourceDeleter::operator()(Resource* _resource) const
{
    if (!_resource)
    {
        return;
    }

    ResourceController& resourceController = engine::GetEngine().getResourceController();
    ResourceController::ResourceControllerAccessor::RemoveResource(resourceController, _resource);
    Resource::ResourceAccessor::Unload(_resource);
    delete _resource;
}

void ego::Resource::ResourceAccessor::PrepareLoading(Resource* _resource, const FileName& _path)
{
    EGO_ASSERT(_resource);
    _resource->prepareLoading(_path);
}

void ego::Resource::ResourceAccessor::SetLoadingError(Resource* _resource, std::string _loadingError)
{
    EGO_ASSERT(_resource);
    _resource->setLoadingError(std::move(_loadingError));
}

void ego::Resource::ResourceAccessor::SetState(Resource* _resource, ResourceState _state)
{
    EGO_ASSERT(_resource);
    _resource->setState(_state);
}

void ego::Resource::ResourceAccessor::Unload(Resource* _resource)
{
    EGO_ASSERT(_resource);
    _resource->unload();
}

void ego::Resource::ResourceAccessor::AddDependency(Resource* _resource, const ResourcePointer& _dependency)
{
    EGO_ASSERT(_resource);
    _resource->addDependency(_dependency);
}

void ego::Resource::ResourceAccessor::GetDependencies(const Resource* _resource, DependencyCollection& _dependencies)
{
    EGO_ASSERT(_resource);
    _resource->getDependencies(_dependencies);
}

bool ego::Resource::ResourceAccessor::OnDependenciesLoaded(Resource* _resource)
{
    EGO_ASSERT(_resource);
    return _resource->onDependenciesLoaded();
}

ego::FileName ego::Resource::getPath() const
{
    std::lock_guard locker(m_mutex);
    return m_path;
}

ego::ResourceState ego::Resource::getState() const
{
    return m_state.load(std::memory_order_acquire);
}

std::string ego::Resource::getLoadingError() const
{
    std::lock_guard locker(m_mutex);
    return m_loadingError;
}

bool ego::Resource::isLoading() const
{
    const ResourceState state = getState();
    return state == ResourceState::Loading || state == ResourceState::LoadingDependencies;
}

bool ego::Resource::isLoadingDependencies() const
{
    return getState() == ResourceState::LoadingDependencies;
}

bool ego::Resource::isLoaded() const
{
    return getState() == ResourceState::Loaded;
}

bool ego::Resource::isFailed() const
{
    return getState() == ResourceState::Failed;
}

bool ego::Resource::load(const FileName& _path, FileContent&& _content, ResourceLoadingContext& _loadingContext)
{
    prepareLoading(_path);

    if (!onLoad(std::move(_content), _loadingContext))
    {
        if (getLoadingError().empty())
        {
            setLoadingError(std::string("Failed to load resource: ") + _path.c_str());
        }

        setState(ResourceState::Failed);
        return false;
    }

    return true;
}

void ego::Resource::unload()
{
    if (getState() == ResourceState::Undefined)
    {
        return;
    }

    onUnload();

    std::lock_guard locker(m_mutex);
    m_path.clear();
    m_loadingError.clear();
    m_dependencies.clear();
    m_dependenciesLoadedCallback = nullptr;
    setState(ResourceState::Undefined);
}

void ego::Resource::onUnload() {}

bool ego::Resource::onDependenciesLoaded()
{
    DependenciesLoadedCallback dependenciesLoadedCallback = takeDependenciesLoadedCallback();
    if (dependenciesLoadedCallback)
    {
        return dependenciesLoadedCallback();
    }

    const std::string loadingError = "Resource dependencies have been loaded, but dependency completion isn't implemented.";

    setLoadingError(loadingError);
    EGO_ASSERT_FAIL_MESSAGE(loadingError.c_str());
    return false;
}

void ego::Resource::prepareLoading(const FileName& _path)
{
    std::lock_guard locker(m_mutex);
    m_path = _path;
    m_loadingError.clear();
    m_dependencies.clear();
    m_dependenciesLoadedCallback = nullptr;
    setState(ResourceState::Loading);
}

void ego::Resource::setLoadingError(std::string _loadingError)
{
    std::lock_guard locker(m_mutex);
    m_loadingError = std::move(_loadingError);
}

void ego::Resource::setDependenciesLoadedCallback(DependenciesLoadedCallback&& _callback)
{
    std::lock_guard locker(m_mutex);
    m_dependenciesLoadedCallback = std::move(_callback);
}

void ego::Resource::setState(ResourceState _state)
{
    m_state.store(_state, std::memory_order_release);
}

void ego::Resource::addDependency(const ResourcePointer& _resource)
{
    if (!_resource)
    {
        return;
    }

    std::lock_guard locker(m_mutex);
    const auto foundIt = std::find_if(
        m_dependencies.begin(),
        m_dependencies.end(),
        [&_resource](const ResourcePointer& _dependency)
        {
            return _dependency.get() == _resource.get();
        });
    if (foundIt != m_dependencies.end())
    {
        return;
    }

    m_dependencies.push_back(_resource);
}

void ego::Resource::clearDependencies()
{
    std::lock_guard locker(m_mutex);
    m_dependencies.clear();
}

void ego::Resource::getDependencies(DependencyCollection& _dependencies) const
{
    std::lock_guard locker(m_mutex);
    _dependencies = m_dependencies;
}

ego::Resource::DependenciesLoadedCallback ego::Resource::takeDependenciesLoadedCallback()
{
    std::lock_guard locker(m_mutex);
    DependenciesLoadedCallback dependenciesLoadedCallback = std::move(m_dependenciesLoadedCallback);
    m_dependenciesLoadedCallback = nullptr;
    return dependenciesLoadedCallback;
}
