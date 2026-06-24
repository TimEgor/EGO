#include <string>
#include <utility>

#include "DXCShaderResourceProvider.h"

#include "DXCShaderCompiler.h"
#include "DXCShaderSourceResolver.h"

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/ShaderResource.h"
#include "EgoEngine/Resources/Resource/ResourceLoadingContext.h"

namespace ego::resources::dxc
{
    bool DXCShaderResourceProvider::onProvideContent(
        const Resource& _resource,
        const FileName& _path,
        ResourceLoadingContext& _loadingContext,
        FileContent& _content,
        std::string& _loadingError)
    {
        DXCShaderSourceResolver sourceResolver(_loadingContext);
        if (!ego::rtti::IsObjectBasedOn<gpu::ShaderResource>(_resource))
        {
            if (sourceResolver.loadContent(_path, _content))
            {
                return true;
            }

            _loadingError = std::string("Failed to load resource content: ") + _path.c_str();
            return false;
        }

        const auto& shaderResource = static_cast<const gpu::ShaderResource&>(_resource);
        FileName sourcePath;
        FileContent sourceContent;
        if (!sourceResolver.resolveShaderSourceContent(_path, sourcePath, sourceContent, _loadingError))
        {
            _content.clear();
            return false;
        }

        if (!sourceResolver.isRawHlslShader(sourcePath))
        {
            _content = std::move(sourceContent);
            if (!_content.empty())
            {
                return true;
            }

            _loadingError = std::string("Shader bytecode is empty: ") + sourcePath.c_str();
            return false;
        }

        const DXCShaderCompiler compiler(_loadingContext);
        return compiler.compileHlslContent(sourceContent, sourcePath, shaderResource.getShaderStage(), _content, _loadingError);
    }
} // namespace ego::resources::dxc
