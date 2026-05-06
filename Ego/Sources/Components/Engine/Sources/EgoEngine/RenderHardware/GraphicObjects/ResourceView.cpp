#include "ResourceView.h"

ego::gpu::ResourceView::ResourceView(const GraphicResourcePointer& _resource)
    : m_resource(_resource)
{}

const ego::gpu::GraphicResourceWeakPointer& ego::gpu::ResourceView::getResource() const
{
    return m_resource;
}