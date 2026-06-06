#pragma once

#include "EgoCore/FileName/FileName.h"

#include "EgoEngine/Platform/FileSystem/FileSystem.h"

#include <string>

namespace ego
{
    class ResourceLoadingContext;
}

namespace ego::resources::dxc
{
    bool ResolveShaderSourceContent(
        const FileName& _path,
        ResourceLoadingContext& _loadingContext,
        FileName& _sourcePath,
        FileContent& _sourceContent,
        std::string& _loadingError
    );
}
