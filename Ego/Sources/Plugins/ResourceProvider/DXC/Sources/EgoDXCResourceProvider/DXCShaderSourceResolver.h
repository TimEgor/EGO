#pragma once

#include <string>
#include <string_view>

#include "EgoCore/FileName/FileName.h"

#include "EgoEngine/Platform/FileSystem/FileSystem.h"

namespace ego
{
    class ResourceLoadingContext;
}

namespace ego::resources::dxc
{
    class DXCShaderSourceResolver final
    {
    public:
        explicit DXCShaderSourceResolver(ResourceLoadingContext& _loadingContext);

        bool isRawHlslShader(const FileName& _path) const;

        bool resolveShaderSourceContent(const FileName& _path, FileName& _sourcePath, FileContent& _sourceContent, std::string& _loadingError) const;

        bool loadContent(const FileName& _path, FileContent& _content) const;

    private:
        std::string getLowerExtension(const FileName& _path) const;
        bool isShaderDescriptor(const FileName& _path) const;
        bool isRootedPath(std::string_view _path) const;
        std::string trim(const std::string& _value) const;
        FileName resolveReferencedPath(const FileName& _shaderPath, const FileName& _referencedPath) const;

        FileName readShaderDescriptorPath(const FileName& _shaderPath, const FileContent& _descriptorContent) const;

        bool resolveShaderDescriptorContent(const FileName& _path, FileName& _sourcePath, FileContent& _sourceContent, std::string& _loadingError) const;

        ResourceLoadingContext& m_loadingContext;
    };
} // namespace ego::resources::dxc
