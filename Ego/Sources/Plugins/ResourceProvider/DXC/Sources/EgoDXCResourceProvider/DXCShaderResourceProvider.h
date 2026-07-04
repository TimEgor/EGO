#pragma once

#include <string>

#include "EgoRuntime/Resource/ResourceProvider.h"

namespace ego::resources::dxc
{
    class DXCShaderResourceProvider final : public ResourceProvider
    {
    public:
        DXCShaderResourceProvider() = default;

    protected:
        bool onProvideContent(const Resource& _resource, const FileName& _path, ResourceLoadingContext& _loadingContext, FileContent& _content, std::string& _loadingError)
            override;
    };

    EGO_POINTER(DXCShaderResourceProvider);
} // namespace ego::resources::dxc
