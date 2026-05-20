#include "Resource.h"

#include "EgoCore/Assert/AssertCore.h"

#include <utility>

ego::Resource::~Resource() = default;

void ego::ResourceDeleter::operator()(Resource* _resource) const
{
    if (!_resource)
    {
        return;
    }

    Resource::ResourceAccessor::Unload(_resource);
    delete _resource;
}

void ego::Resource::ResourceAccessor::PrepareLoading(Resource* _resource, const FileName& _path)
{
    EGO_ASSERT(_resource);
    _resource->prepareLoading(_path);
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

void ego::Resource::ResourceAccessor::AddChildDependency(
    Resource* _resource,
    const ResourcePointer& _childResource
)
{
    EGO_ASSERT(_resource);
    _resource->addChildDependency(_childResource);
}

void ego::Resource::ResourceAccessor::GetChildDependencies(
    const Resource* _resource,
    ChildDependencyCollection& _dependencies
)
{
    EGO_ASSERT(_resource);
    _resource->getChildDependencies(_dependencies);
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

bool ego::Resource::isLoading() const
{
    return getState() == ResourceState::Loading;
}

bool ego::Resource::isLoaded() const
{
    return getState() == ResourceState::Loaded;
}

bool ego::Resource::isFailed() const
{
    return getState() == ResourceState::Failed;
}

bool ego::Resource::load(
    const FileName& _path,
    FileContent&& _content,
    ResourceLoadingContext& _loadingContext
)
{
    prepareLoading(_path);

    const bool result = onLoad(std::move(_content), _loadingContext);
    setState(result ? ResourceState::Loaded : ResourceState::Failed);

    return result;
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
    m_childDependencies.clear();
    setState(ResourceState::Undefined);
}

void ego::Resource::onUnload()
{}

void ego::Resource::prepareLoading(const FileName& _path)
{
    std::lock_guard locker(m_mutex);
    m_path = _path;
    m_childDependencies.clear();
    setState(ResourceState::Loading);
}

void ego::Resource::setState(ResourceState _state)
{
    m_state.store(_state, std::memory_order_release);
}

void ego::Resource::addChildDependency(const ResourcePointer& _resource)
{
    if (!_resource)
    {
        return;
    }

    std::lock_guard locker(m_mutex);
    m_childDependencies.push_back(_resource);
}

void ego::Resource::clearChildDependencies()
{
    std::lock_guard locker(m_mutex);
    m_childDependencies.clear();
}

void ego::Resource::getChildDependencies(ChildDependencyCollection& _dependencies) const
{
    std::lock_guard locker(m_mutex);
    _dependencies = m_childDependencies;
}
