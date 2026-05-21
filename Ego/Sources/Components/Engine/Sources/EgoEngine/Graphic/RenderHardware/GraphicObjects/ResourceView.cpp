#include "ResourceView.h"

ego::gpu::ResourceView::ResourceView(const GraphicResourceReference& _resource)
    : m_resource(_resource)
{}

const ego::gpu::GraphicResourceReference& ego::gpu::ResourceView::getResource() const
{
    return m_resource;
}

uint32_t ego::gpu::ResourceView::getBindlessIndex() const
{
    return InvalidBindlessIndex;
}
