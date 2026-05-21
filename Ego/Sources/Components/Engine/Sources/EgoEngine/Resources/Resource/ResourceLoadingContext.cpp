#include "ResourceLoadingContext.h"

#include "ResourceController.h"

ego::ResourceLoadingContext::ResourceLoadingContext(
    ResourceController& _controller,
    Resource& _ownerResource,
    bool _asyncLoading
)
    : m_controller(_controller)
    , m_ownerResource(_ownerResource)
    , m_asyncLoading(_asyncLoading)
{}

ego::ResourcePointer ego::ResourceLoadingContext::loadResource(
    ResourceType _type,
    const FileName& _path,
    const ResourceFactory& _factory
)
{
    ResourcePointer resource = m_asyncLoading
        ? m_controller.loadResourceAsync(_type, _path, _factory)
        : m_controller.loadResource(_type, _path, _factory);

    if (resource)
    {
        Resource::ResourceAccessor::AddChildDependency(&m_ownerResource, resource);
    }

    return resource;
}

bool ego::ResourceLoadingContext::loadContent(const FileName& _path, FileContent& _content) const
{
    return m_controller.loadResourceContent(_path, _content);
}
