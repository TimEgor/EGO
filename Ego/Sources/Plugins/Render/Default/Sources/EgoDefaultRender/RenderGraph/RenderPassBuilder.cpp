#include "RenderPassBuilder.h"

ego::render::RenderPassBuilder::RenderPassBuilder(const char* _passName)
    : m_passName(_passName)
{
}

const char* ego::render::RenderPassBuilder::getPassName() const
{
    return m_passName;
}

const std::vector<ego::render::RenderGraphResourceUsage>& ego::render::RenderPassBuilder::getResourceUsages() const
{
    return m_resourceUsages;
}

void ego::render::RenderPassBuilder::readTexture(const char* _name, gpu::GraphicResourceState _state, gpu::GraphicResourceViewType _viewType)
{
    addResourceUsage(_name, RenderGraphResourceKind::Texture, RenderGraphResourceAccess::Read, _state, _viewType);
}

void ego::render::RenderPassBuilder::writeTexture(const char* _name, gpu::GraphicResourceState _state, gpu::GraphicResourceViewType _viewType)
{
    addResourceUsage(_name, RenderGraphResourceKind::Texture, RenderGraphResourceAccess::Write, _state, _viewType);
}

void ego::render::RenderPassBuilder::readBuffer(const char* _name, gpu::GraphicResourceState _state, gpu::GraphicResourceViewType _viewType)
{
    addResourceUsage(_name, RenderGraphResourceKind::Buffer, RenderGraphResourceAccess::Read, _state, _viewType);
}

void ego::render::RenderPassBuilder::readAccelerationStructure(const char* _name)
{
    addResourceUsage(
        _name,
        RenderGraphResourceKind::AccelerationStructure,
        RenderGraphResourceAccess::Read,
        gpu::GraphicResourceState::RayTracingAccelerationStructure,
        gpu::GraphicResourceViewType::ShaderResource);
}

void ego::render::RenderPassBuilder::addResourceUsage(
    const char* _name,
    RenderGraphResourceKind _kind,
    RenderGraphResourceAccess _access,
    gpu::GraphicResourceState _state,
    gpu::GraphicResourceViewType _viewType)
{
    RenderGraphResourceUsage usage;
    usage.m_name = _name;
    usage.m_kind = _kind;
    usage.m_access = _access;
    usage.m_state = _state;
    usage.m_viewType = _viewType;
    m_resourceUsages.push_back(usage);
}
