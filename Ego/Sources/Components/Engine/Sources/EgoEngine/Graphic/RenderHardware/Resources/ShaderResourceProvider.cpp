#include "ShaderResourceProvider.h"

#include "EgoEngine/Resources/Resource/ResourceLoadingContext.h"

bool ego::gpu::ShaderResourceProvider::provideContent(
    const Resource& _resource,
    const FileName& _path,
    ResourceLoadingContext& _loadingContext,
    FileContent& _content
)
{
    if (!ego::rtti::IsObjectBasedOn<ShaderResource>(_resource))
    {
        _content.clear();
        return false;
    }

    const ShaderResource& shaderResource = static_cast<const ShaderResource&>(_resource);
    return provideShaderContent(
        _path,
        shaderResource.getShaderStage(),
        _loadingContext,
        _content
    );
}

bool ego::gpu::ShaderResourceProvider::provideShaderContent(
    const FileName& _path,
    ShaderStage,
    ResourceLoadingContext& _loadingContext,
    FileContent& _content
)
{
    return _loadingContext.loadContent(_path, _content);
}
