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

bool ego::ResourceLoadingContext::loadContent(const FileName& _path, FileContent& _content) const
{
    return ResourceController::ResourceControllerAccessor::LoadResourceContent(&m_controller, _path, _content);
}
