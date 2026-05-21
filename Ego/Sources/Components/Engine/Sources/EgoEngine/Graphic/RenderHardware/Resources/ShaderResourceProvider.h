#pragma once

#include "ShaderResource.h"

#include "EgoEngine/Resources/Resource/ResourceProvider.h"

namespace ego::gpu
{
    class ShaderResourceProvider : public ResourceProvider
    {
    public:
        ShaderResourceProvider() = default;

        virtual bool provideContent(
            const Resource& _resource,
            const FileName& _path,
            ResourceLoadingContext& _loadingContext,
            FileContent& _content
        ) override;

    protected:
        virtual bool provideShaderContent(
            const FileName& _path,
            ShaderStage _stage,
            ResourceLoadingContext& _loadingContext,
            FileContent& _content
        );
    };

    EGO_POINTER(ShaderResourceProvider);
}
