#include "DXCShaderSourceResolver.h"

#include <algorithm>
#include <cctype>
#include <string>

#include "EgoCore/FileName/FileNameUtils.h"

#include "EgoResource/ResourceLoadingContext.h"

namespace ego::resources::dxc
{
    DXCShaderSourceResolver::DXCShaderSourceResolver(ResourceLoadingContext& _loadingContext)
        : m_loadingContext(_loadingContext)
    {
    }

    bool DXCShaderSourceResolver::isRawHlslShader(const FileName& _path) const
    {
        return getLowerExtension(_path) == ".hlsl";
    }

    bool DXCShaderSourceResolver::resolveShaderSourceContent(
        const FileName& _path,
        FileName& _sourcePath,
        FileContent& _sourceContent,
        std::string& _loadingError) const
    {
        _sourcePath = _path;
        _sourceContent.clear();
        _loadingError.clear();

        if (isShaderDescriptor(_path))
        {
            return resolveShaderDescriptorContent(_path, _sourcePath, _sourceContent, _loadingError);
        }

        if (loadContent(_sourcePath, _sourceContent))
        {
            return true;
        }

        _loadingError = std::string("Failed to load shader source: ") + _sourcePath.c_str();
        return false;
    }

    bool DXCShaderSourceResolver::loadContent(const FileName& _path, FileContent& _content) const
    {
        _content.clear();
        return _path && m_loadingContext.loadContent(_path, _content);
    }

    std::string DXCShaderSourceResolver::getLowerExtension(const FileName& _path) const
    {
        std::string extension(file_name_utils::GetFileExtension(_path).getView());
        std::transform(
            extension.begin(),
            extension.end(),
            extension.begin(),
            [](unsigned char _ch)
            {
                return static_cast<char>(std::tolower(_ch));
            });
        return extension;
    }

    bool DXCShaderSourceResolver::isShaderDescriptor(const FileName& _path) const
    {
        return getLowerExtension(_path) == ".shader";
    }

    bool DXCShaderSourceResolver::isRootedPath(std::string_view _path) const
    {
        return (!_path.empty() && (_path[0] == '/' || _path[0] == '\\')) || (_path.size() > 1 && _path[1] == ':');
    }

    std::string DXCShaderSourceResolver::trim(const std::string& _value) const
    {
        const size_t begin = _value.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos)
        {
            return std::string();
        }

        const size_t end = _value.find_last_not_of(" \t\r\n");
        return _value.substr(begin, end - begin + 1);
    }

    FileName DXCShaderSourceResolver::resolveReferencedPath(const FileName& _shaderPath, const FileName& _referencedPath) const
    {
        const std::string referencedPath(_referencedPath.getView());
        if (referencedPath.empty() || isRootedPath(referencedPath))
        {
            return _referencedPath;
        }

        const FileName dirPath = file_name_utils::GetFileDirPath(_shaderPath);
        return dirPath ? dirPath + "/" + _referencedPath : _referencedPath;
    }

    FileName DXCShaderSourceResolver::readShaderDescriptorPath(const FileName& _shaderPath, const FileContent& _descriptorContent) const
    {
        if (_descriptorContent.empty())
        {
            return FileName();
        }

        const std::string descriptorText(reinterpret_cast<const char*>(_descriptorContent.data()), _descriptorContent.size());

        return resolveReferencedPath(_shaderPath, FileName(trim(descriptorText)));
    }

    bool DXCShaderSourceResolver::resolveShaderDescriptorContent(
        const FileName& _path,
        FileName& _sourcePath,
        FileContent& _sourceContent,
        std::string& _loadingError) const
    {
        FileContent descriptorContent;
        if (loadContent(_path, descriptorContent))
        {
            _sourcePath = readShaderDescriptorPath(_path, descriptorContent);
            if (loadContent(_sourcePath, _sourceContent))
            {
                return true;
            }

            _loadingError = std::string("Failed to load shader source referenced by descriptor: ") + _sourcePath.c_str();
            return false;
        }

        const FileName pathWithoutExtension = file_name_utils::RemoveExtension(_path);
        const FileName compiledShaderPath = pathWithoutExtension + ".dxc";
        const FileName rawShaderPath = pathWithoutExtension + ".hlsl";

        if (loadContent(compiledShaderPath, _sourceContent))
        {
            _sourcePath = compiledShaderPath;
            return true;
        }

        if (loadContent(rawShaderPath, _sourceContent))
        {
            _sourcePath = rawShaderPath;
            return true;
        }

        _sourceContent.clear();
        _loadingError = std::string("Failed to load shader descriptor or fallback shader files: ") + compiledShaderPath.c_str() + ", " + rawShaderPath.c_str();
        return false;
    }
} // namespace ego::resources::dxc
