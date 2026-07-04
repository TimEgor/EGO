#pragma once

#include <string>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Platform/FileSystem/FileSystem.h"

#include "EgoGraphicHardware/GraphicObjects/Shader.h"

struct IDxcResult;

namespace ego
{
    class ResourceLoadingContext;
}

namespace ego::resources::dxc
{
    class DXCShaderCompiler final
    {
    public:
        explicit DXCShaderCompiler(ResourceLoadingContext& _loadingContext);

        bool compileHlslContent(const FileContent& _content, const FileName& _sourcePath, gpu::ShaderStage _stage, FileContent& _compiledContent, std::string& _loadingError) const;

    private:
        const wchar_t* getShaderEntryPoint(gpu::ShaderStage _stage) const;
        const wchar_t* getShaderTarget(gpu::ShaderStage _stage) const;
        std::string getHResultText(long _result) const;
        std::string readCompileError(IDxcResult& _compileResult) const;

        ResourceLoadingContext& m_loadingContext;
    };
} // namespace ego::resources::dxc
