#include "DXCShaderResourceProvider.h"

#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoEngine/Resources/Resource/ResourceLoadingContext.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <limits>
#include <string>
#include <utility>

#include <Windows.h>
#include <Unknwn.h>
#include <ObjIdl.h>
#include <WTypes.h>
#include <dxcapi.h>
#include <wrl/client.h>

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

    bool IsRawHlslShader(const ego::FileName& _path)
    {
        return GetLowerExtension(_path) == ".hlsl";
    }

    bool IsShaderDescriptor(const ego::FileName& _path)
    {
        return GetLowerExtension(_path) == ".shader";
    }

    bool IsRootedPath(const std::string& _path)
    {
        return (!_path.empty() && (_path[0] == '/' || _path[0] == '\\')) ||
            (_path.size() > 1 && _path[1] == ':');
    }

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
        if (referencedPath.empty() || IsRootedPath(referencedPath))
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

        const std::string referencedPath = Trim(descriptorText);
        return ResolveReferencedPath(_shaderPath, referencedPath);
    }

    bool TryLoadContent(
        ego::ResourceLoadingContext& _loadingContext,
        const ego::FileName& _path,
        ego::FileContent& _content
    )
    {
        _content.clear();
        return _path && _loadingContext.loadContent(_path, _content);
    }

    const wchar_t* GetShaderEntryPoint(ego::gpu::ShaderStage _stage)
    {
        switch (_stage)
        {
        case ego::gpu::ShaderStage::Vertex:
            return L"VSMain";
        case ego::gpu::ShaderStage::Pixel:
            return L"PSMain";
        case ego::gpu::ShaderStage::Compute:
            return L"CSMain";
        default:
            break;
        }

        return nullptr;
    }

    const wchar_t* GetShaderTarget(ego::gpu::ShaderStage _stage)
    {
        switch (_stage)
        {
        case ego::gpu::ShaderStage::Vertex:
            return L"vs_6_6";
        case ego::gpu::ShaderStage::Pixel:
            return L"ps_6_6";
        case ego::gpu::ShaderStage::Compute:
            return L"cs_6_6";
        default:
            break;
        }

        return nullptr;
    }

    bool CompileHlslContent(
        const ego::FileContent& _content,
        ego::gpu::ShaderStage _stage,
        ego::FileContent& _compiledContent
    )
    {
        const wchar_t* entryPoint = GetShaderEntryPoint(_stage);
        const wchar_t* target = GetShaderTarget(_stage);
        if (_content.empty() || !entryPoint || !target)
        {
            _compiledContent.clear();
            return false;
        }

        Microsoft::WRL::ComPtr<IDxcUtils> utils;
        Microsoft::WRL::ComPtr<IDxcCompiler3> compiler;

        const HRESULT utilsResult = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
        if (FAILED(utilsResult) || !utils)
        {
            _compiledContent.clear();
            return false;
        }

        const HRESULT compilerResult = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
        if (FAILED(compilerResult) || !compiler)
        {
            _compiledContent.clear();
            return false;
        }

        Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
        if (FAILED(utils->CreateDefaultIncludeHandler(&includeHandler)))
        {
            _compiledContent.clear();
            return false;
        }

        const wchar_t* arguments[] =
        {
            L"-E",
            entryPoint,
            L"-T",
            target,
            L"-HV",
            L"2021",
            DXC_ARG_WARNINGS_ARE_ERRORS
        };

        DxcBuffer sourceBuffer = {};
        sourceBuffer.Ptr = _content.data();
        sourceBuffer.Size = _content.size();
        sourceBuffer.Encoding = DXC_CP_UTF8;

        Microsoft::WRL::ComPtr<IDxcResult> compileResult;
        const HRESULT compileCallResult = compiler->Compile(
            &sourceBuffer,
            arguments,
            static_cast<uint32_t>(sizeof(arguments) / sizeof(arguments[0])),
            includeHandler.Get(),
            IID_PPV_ARGS(&compileResult)
        );
        if (FAILED(compileCallResult) || !compileResult)
        {
            _compiledContent.clear();
            return false;
        }

        HRESULT compileStatus = S_OK;
        if (FAILED(compileResult->GetStatus(&compileStatus)) || FAILED(compileStatus))
        {
            _compiledContent.clear();
            return false;
        }

        Microsoft::WRL::ComPtr<IDxcBlob> compiledShader;
        const HRESULT objectResult = compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiledShader), nullptr);
        if (FAILED(objectResult) || !compiledShader)
        {
            _compiledContent.clear();
            return false;
        }

        const size_t compiledSize = compiledShader->GetBufferSize();
        if (compiledSize == 0 || compiledSize > (std::numeric_limits<uint32_t>::max)())
        {
            _compiledContent.clear();
            return false;
        }

        _compiledContent.resize(compiledSize);
        std::memcpy(_compiledContent.data(), compiledShader->GetBufferPointer(), compiledSize);

        return true;
    }
}

bool ego::gpu::d3d12::DXCShaderResourceProvider::provideShaderContent(
    const FileName& _path,
    ShaderStage _stage,
    ResourceLoadingContext& _loadingContext,
    FileContent& _content
)
{
    FileName sourcePath = _path;
    FileContent sourceContent;

    if (IsShaderDescriptor(_path))
    {
        FileContent descriptorContent;
        if (TryLoadContent(_loadingContext, _path, descriptorContent))
        {
            sourcePath = ReadShaderDescriptorPath(_path, descriptorContent);
            if (!TryLoadContent(_loadingContext, sourcePath, sourceContent))
            {
                _content.clear();
                return false;
            }
        }
        else
        {
            const FileName pathWithoutExtension = file_name_utils::RemoveExtension(_path);
            const FileName compiledShaderPath = pathWithoutExtension + ".dxc";
            const FileName rawShaderPath = pathWithoutExtension + ".hlsl";

            if (TryLoadContent(_loadingContext, compiledShaderPath, sourceContent))
            {
                sourcePath = compiledShaderPath;
            }
            else if (TryLoadContent(_loadingContext, rawShaderPath, sourceContent))
            {
                sourcePath = rawShaderPath;
            }
            else
            {
                _content.clear();
                return false;
            }
        }
    }
    else if (!TryLoadContent(_loadingContext, sourcePath, sourceContent))
    {
        _content.clear();
        return false;
    }

    if (!IsRawHlslShader(sourcePath))
    {
        _content = std::move(sourceContent);
        return !_content.empty();
    }

    return CompileHlslContent(sourceContent, _stage, _content);
}
