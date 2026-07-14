#include "ResourceLoadingContext.h"

#include <utility>

#include "ResourceController.h"

ego::ResourceLoadingContext::ResourceLoadingContext(ResourceController& _controller, Resource& _ownerResource, bool _isAsyncLoading)
    : m_controller(_controller),
      m_ownerResource(_ownerResource),
      m_isAsyncLoading(_isAsyncLoading)
{
}

bool ego::ResourceLoadingContext::addDependency(const ResourcePointer& _dependency)
{
    if (!_dependency)
    {
        return false;
    }

    if (!ResourceController::ResourceControllerAccessor::AddDependency(m_controller, m_ownerResource, _dependency))
    {
        return false;
    }

    m_dependencies.push_back(_dependency);
    return true;
}

ego::Resource::DependencyCollection ego::ResourceLoadingContext::takeDependencies()
{
    return std::move(m_dependencies);
}

bool ego::ResourceLoadingContext::hasAsyncDependencies() const
{
    return m_hasAsyncDependencies;
}

bool ego::ResourceLoadingContext::isAsyncLoading() const
{
    return m_isAsyncLoading;
}

bool ego::ResourceLoadingContext::loadContent(const FileName& _path, FileContent& _content) const
{
    return ResourceController::ResourceControllerAccessor::LoadResourceContent(m_controller, _path, _content);
}
