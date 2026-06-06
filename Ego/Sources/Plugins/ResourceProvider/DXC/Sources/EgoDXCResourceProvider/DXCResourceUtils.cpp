#include "DXCResourceUtils.h"

#include "EgoCore/FileName/FileNameUtils.h"

#include "EgoEngine/Resources/Resource/ResourceLoadingContext.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace
{
    std::string GetLowerExtension(const ego::FileName& _path)
    {
        std::string extension = ego::file_name_utils::GetFileExtension(_path).c_str();
        std::transform(
            extension.begin(),
            extension.end(),
            extension.begin(),
            [](unsigned char _ch)
            {
                return static_cast<char>(std::tolower(_ch));
            }
        );
        return extension;
    }
}

bool ego::resources::dxc::IsRawHlslShader(const FileName& _path)
{
    return GetLowerExtension(_path) == ".hlsl";
}

bool ego::resources::dxc::IsShaderDescriptor(const FileName& _path)
{
    return GetLowerExtension(_path) == ".shader";
}

bool ego::resources::dxc::IsRootedPath(std::string_view _path)
{
    return (!_path.empty() && (_path[0] == '/' || _path[0] == '\\')) ||
        (_path.size() > 1 && _path[1] == ':');
}

bool ego::resources::dxc::TryLoadContent(
    ResourceLoadingContext& _loadingContext,
    const FileName& _path,
    FileContent& _content
)
{
    _content.clear();
    return _path && _loadingContext.loadContent(_path, _content);
}
