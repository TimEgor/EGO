#pragma once

#include <vector>

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/GraphicResource.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/ResourceView.h"

namespace ego::render
{
    enum class RenderGraphResourceKind
    {
        Texture,
        Buffer,
        AccelerationStructure
    };

    enum class RenderGraphResourceAccess
    {
        Read,
        Write,
        ReadWrite
    };

    struct RenderGraphResourceUsage final
    {
        const char* m_name = nullptr;
        RenderGraphResourceKind m_kind = RenderGraphResourceKind::Texture;
        RenderGraphResourceAccess m_access = RenderGraphResourceAccess::Read;
        gpu::GraphicResourceState m_state = gpu::GraphicResourceState::Undefined;
        gpu::GraphicResourceViewType m_viewType = gpu::GraphicResourceViewType::ShaderResource;
    };

    class RenderPassBuilder final
    {
    public:
        explicit RenderPassBuilder(const char* _passName);

        const char* getPassName() const;
        const std::vector<RenderGraphResourceUsage>& getResourceUsages() const;

        void readTexture(const char* _name, gpu::GraphicResourceState _state, gpu::GraphicResourceViewType _viewType = gpu::GraphicResourceViewType::ShaderResource);
        void writeTexture(const char* _name, gpu::GraphicResourceState _state, gpu::GraphicResourceViewType _viewType);
        void readBuffer(
            const char* _name,
            gpu::GraphicResourceState _state = gpu::GraphicResourceState::ShaderRead,
            gpu::GraphicResourceViewType _viewType = gpu::GraphicResourceViewType::ShaderResource);
        void readAccelerationStructure(const char* _name);

    private:
        void addResourceUsage(
            const char* _name,
            RenderGraphResourceKind _kind,
            RenderGraphResourceAccess _access,
            gpu::GraphicResourceState _state,
            gpu::GraphicResourceViewType _viewType);

        const char* m_passName = nullptr;
        std::vector<RenderGraphResourceUsage> m_resourceUsages;
    };
} // namespace ego::render
