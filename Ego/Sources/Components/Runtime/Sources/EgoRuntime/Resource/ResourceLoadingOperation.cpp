#include "ResourceController.h"

ego::ResourceLoadingOperation::ResourceLoadingOperation(const ResourceControllerWeakPointer& _controller, const ResourcePointer& _resource)
    : m_controller(_controller),
      m_resource(_resource)
{
}

ego::ResourcePointer ego::ResourceLoadingOperation::getResource() const
{
    return m_resource;
}

ego::ResourceState ego::ResourceLoadingOperation::getState() const
{
    return m_resource ? m_resource->getState() : ResourceState::Undefined;
}

bool ego::ResourceLoadingOperation::isFinished() const
{
    return !m_resource || !m_resource->isLoading();
}

bool ego::ResourceLoadingOperation::isLoaded() const
{
    return m_resource && m_resource->isLoaded();
}

bool ego::ResourceLoadingOperation::isFailed() const
{
    return !m_resource || m_resource->isFailed();
}

bool ego::ResourceLoadingOperation::waitLoading()
{
    if (!m_resource)
    {
        return false;
    }

    if (!m_resource->isLoading())
    {
        return m_resource->isLoaded();
    }

    const ResourceControllerPointer controller = m_controller.lock();
    return controller ? controller->waitLoading(m_resource) : false;
}
