#pragma once

#include "EgoEngine/Resources/Resource/ResourceProvider.h"

namespace ego::resources::dxc
{
    class DXCShaderResourceProvider final : public ResourceProvider
    {
    public:
        DXCShaderResourceProvider() = default;

    protected:
        virtual bool onProvideContent(
            const Resource& _resource,
            const FileName& _path,
            ResourceLoadingContext& _loadingContext,
            FileContent& _content
        ) override;
    };

    EGO_POINTER(DXCShaderResourceProvider);
}
