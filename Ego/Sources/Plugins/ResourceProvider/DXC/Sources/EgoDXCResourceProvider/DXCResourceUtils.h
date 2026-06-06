#pragma once

#include "EgoCore/FileName/FileName.h"

#include "EgoEngine/Platform/FileSystem/FileSystem.h"

#include <string_view>

namespace ego
{
    class ResourceLoadingContext;
}

namespace ego::resources::dxc
{
    bool IsRawHlslShader(const FileName& _path);
    bool IsShaderDescriptor(const FileName& _path);
    bool IsRootedPath(std::string_view _path);
    bool TryLoadContent(ResourceLoadingContext& _loadingContext, const FileName& _path, FileContent& _content);
}
