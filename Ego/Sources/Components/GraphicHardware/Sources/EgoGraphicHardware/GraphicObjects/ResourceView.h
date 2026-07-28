#pragma once

#include "GraphicResource.h"

namespace ego::gpu
{
    enum class GraphicResourceViewType
    {
        ConstantBuffer,
        ShaderResource,
        UnorderedAccess,
        RenderTarget,
        DepthStencil
    };

    class ResourceView : public GraphicObject
    {
    public:
        ResourceView(const GraphicResourcePointer& _resource);

        const GraphicResourcePointer& getResource() const;
        virtual uint32_t getBindlessIndex() const;

        virtual GraphicResourceViewType getViewType() const = 0;

    private:
        GraphicResourcePointer m_resource;
    };

    EGO_INTRUSIVE_POINTER(ResourceView);
} // namespace ego::gpu
