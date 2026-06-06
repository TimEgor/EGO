#pragma once

#include "EgoCore/FileName/FileName.h"

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Shader.h"
#include "EgoEngine/Platform/FileSystem/FileSystem.h"

#include <string>

namespace ego
{
    class ResourceLoadingContext;
}

namespace ego::resources::dxc
{
    bool CompileHlslContent(
        const FileContent& _content,
        const FileName& _sourcePath,
        gpu::ShaderStage _stage,
        ResourceLoadingContext& _loadingContext,
        FileContent& _compiledContent,
        std::string& _loadingError
    );
}
