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

        const GraphicResourceWeakPointer& getResource() const;
        virtual uint32_t getBindlessIndex() const;

        virtual GraphicResourceViewType getViewType() const = 0;

    private:
        GraphicResourceWeakPointer m_resource;
    };

    EGO_POINTER(ResourceView);
}
