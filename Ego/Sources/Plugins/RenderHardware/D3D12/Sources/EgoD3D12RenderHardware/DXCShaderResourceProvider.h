#pragma once

#include "EgoEngine/Graphic/RenderHardware/Resources/ShaderResourceProvider.h"

namespace ego::gpu::d3d12
{
    class DXCShaderResourceProvider final : public ShaderResourceProvider
    {
    public:
        DXCShaderResourceProvider() = default;

    protected:
        virtual bool provideShaderContent(
            const FileName& _path,
            ShaderStage _stage,
            ResourceLoadingContext& _loadingContext,
            FileContent& _content
        ) override;
    };

    EGO_POINTER(DXCShaderResourceProvider);
}
