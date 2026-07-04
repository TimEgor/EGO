#pragma once

#include "EgoCore/Platform/FileSystem/FileSystem.h"

#include "ShaderInterface.h"

namespace ego::gpu
{
    FileContent PackShaderContent(const FileContent& _shaderCode, const ShaderInterface& _shaderInterface);
    bool IsShaderPackageContent(const FileContent& _content);
    bool TryUnpackShaderContent(const FileContent& _content, FileContent& _shaderCode, ShaderInterface& _shaderInterface);
} // namespace ego::gpu
