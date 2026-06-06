#include "ResourceLoadingContext.h"

#include "ResourceController.h"

ego::ResourceLoadingContext::ResourceLoadingContext(
    ResourceController& _controller,
    Resource& _ownerResource
)
    : m_controller(_controller)
    , m_ownerResource(_ownerResource)
{}

bool ego::ResourceLoadingContext::loadContent(const FileName& _path, FileContent& _content) const
{
    return ResourceController::ResourceControllerAccessor::LoadResourceContent(m_controller, _path, _content);
}
