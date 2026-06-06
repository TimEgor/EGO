#include "DXCShaderSourceResolver.h"

#include "DXCResourceUtils.h"

#include "EgoCore/FileName/FileNameUtils.h"

#include <string>

namespace
{
    std::string Trim(const std::string& _value)
    {
        const size_t begin = _value.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos)
        {
            return std::string();
        }

        const size_t end = _value.find_last_not_of(" \t\r\n");
        return _value.substr(begin, end - begin + 1);
    }

    ego::FileName ResolveReferencedPath(const ego::FileName& _shaderPath, const ego::FileName& _referencedPath)
    {
        const std::string referencedPath = _referencedPath.c_str();
        if (referencedPath.empty() || ego::resources::dxc::IsRootedPath(referencedPath))
        {
            return _referencedPath;
        }

        const ego::FileName dirPath = ego::file_name_utils::GetFileDirPath(_shaderPath);
        return dirPath ? dirPath + "/" + _referencedPath : _referencedPath;
    }

    ego::FileName ReadShaderDescriptorPath(
        const ego::FileName& _shaderPath,
        const ego::FileContent& _descriptorContent
    )
    {
        if (_descriptorContent.empty())
        {
            return ego::FileName();
        }

        const std::string descriptorText(
            reinterpret_cast<const char*>(_descriptorContent.data()),
            _descriptorContent.size()
        );

        return ResolveReferencedPath(_shaderPath, ego::FileName(Trim(descriptorText)));
    }

    bool ResolveShaderDescriptorContent(
        const ego::FileName& _path,
        ego::ResourceLoadingContext& _loadingContext,
        ego::FileName& _sourcePath,
        ego::FileContent& _sourceContent,
        std::string& _loadingError
    )
    {
        ego::FileContent descriptorContent;
        if (ego::resources::dxc::TryLoadContent(_loadingContext, _path, descriptorContent))
        {
            _sourcePath = ReadShaderDescriptorPath(_path, descriptorContent);
            if (ego::resources::dxc::TryLoadContent(_loadingContext, _sourcePath, _sourceContent))
            {
                return true;
            }

            _loadingError = std::string("Failed to load shader source referenced by descriptor: ") +
                _sourcePath.c_str();
            return false;
        }

        const ego::FileName pathWithoutExtension = ego::file_name_utils::RemoveExtension(_path);
        const ego::FileName compiledShaderPath = pathWithoutExtension + ".dxc";
        const ego::FileName rawShaderPath = pathWithoutExtension + ".hlsl";

        if (ego::resources::dxc::TryLoadContent(_loadingContext, compiledShaderPath, _sourceContent))
        {
            _sourcePath = compiledShaderPath;
            return true;
        }

        if (ego::resources::dxc::TryLoadContent(_loadingContext, rawShaderPath, _sourceContent))
        {
            _sourcePath = rawShaderPath;
            return true;
        }

        _sourceContent.clear();
        _loadingError = std::string("Failed to load shader descriptor or fallback shader files: ") +
            compiledShaderPath.c_str() + ", " + rawShaderPath.c_str();
        return false;
    }
}

bool ego::resources::dxc::ResolveShaderSourceContent(
    const FileName& _path,
    ResourceLoadingContext& _loadingContext,
    FileName& _sourcePath,
    FileContent& _sourceContent,
    std::string& _loadingError
)
{
    _sourcePath = _path;
    _sourceContent.clear();
    _loadingError.clear();

    if (IsShaderDescriptor(_path))
    {
        return ResolveShaderDescriptorContent(_path, _loadingContext, _sourcePath, _sourceContent, _loadingError);
    }

    if (TryLoadContent(_loadingContext, _sourcePath, _sourceContent))
    {
        return true;
    }

    _loadingError = std::string("Failed to load shader source: ") + _sourcePath.c_str();
    return false;
}
