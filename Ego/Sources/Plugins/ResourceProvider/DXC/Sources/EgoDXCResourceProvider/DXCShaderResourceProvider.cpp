#include <string>
#include <utility>

#include "DXCShaderResourceProvider.h"

#include "DXCResourceUtils.h"
#include "DXCShaderCompiler.h"
#include "DXCShaderSourceResolver.h"

#include "EgoEngine/Graphic/RenderHardware/Resources/ShaderResource.h"
#include "EgoEngine/Resources/Resource/ResourceLoadingContext.h"

bool ego::resources::dxc::DXCShaderResourceProvider::onProvideContent(
    const Resource& _resource,
    const FileName& _path,
    ResourceLoadingContext& _loadingContext,
    FileContent& _content,
    std::string& _loadingError
)
{
    if (!ego::rtti::IsObjectBasedOn<gpu::ShaderResource>(_resource))
    {
        if (TryLoadContent(_loadingContext, _path, _content))
        {
            return true;
        }

        _loadingError = std::string("Failed to load resource content: ") + _path.c_str();
        return false;
    }

    const gpu::ShaderResource& shaderResource = static_cast<const gpu::ShaderResource&>(_resource);
    FileName sourcePath;
    FileContent sourceContent;
    if (!ResolveShaderSourceContent(_path, _loadingContext, sourcePath, sourceContent, _loadingError))
    {
        _content.clear();
        return false;
    }

    if (!IsRawHlslShader(sourcePath))
    {
        _content = std::move(sourceContent);
        if (!_content.empty())
        {
            return true;
        }

        _loadingError = std::string("Shader bytecode is empty: ") + sourcePath.c_str();
        return false;
    }

    if (CompileHlslContent(
        sourceContent,
        sourcePath,
        shaderResource.getShaderStage(),
        _loadingContext,
        _content,
        _loadingError
    ))
    {
        return true;
    }

    return false;
}
