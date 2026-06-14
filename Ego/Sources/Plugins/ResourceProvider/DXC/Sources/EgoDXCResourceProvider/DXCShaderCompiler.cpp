#include "DXCShaderCompiler.h"

#include "DXCResourceIncludeHandler.h"

#include "EgoCore/String/StringConverter.h"

#include <Windows.h>
#include <Unknwn.h>
#include <ObjIdl.h>
#include <WTypes.h>
#include <dxcapi.h>
#include <wrl/client.h>

#include <cstdint>
#include <cstring>
#include <limits>
#include <sstream>
#include <string>

namespace
{
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

    bool CreateDxcInstances(
        Microsoft::WRL::ComPtr<IDxcUtils>& _utils,
        Microsoft::WRL::ComPtr<IDxcCompiler3>& _compiler,
        std::string& _loadingError
    )
    {
        const HRESULT utilsResult = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
        if (FAILED(utilsResult) || !_utils)
        {
            _loadingError = "Failed to create DXC utils.";
            return false;
        }

        const HRESULT compilerResult = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
        if (SUCCEEDED(compilerResult) && _compiler)
        {
            return true;
        }

        _loadingError = "Failed to create DXC compiler.";
        return false;
    }

    bool BuildCompilerArguments(
        IDxcUtils& _utils,
        const ego::FileName& _sourcePath,
        const wchar_t* _entryPoint,
        const wchar_t* _target,
        Microsoft::WRL::ComPtr<IDxcCompilerArgs>& _compilerArguments,
        std::string& _loadingError
    )
    {
        LPCWSTR arguments[] =
        {
            L"-HV",
            L"2021",
            DXC_ARG_WARNINGS_ARE_ERRORS
        };

        const std::wstring sourcePath = ego::ConvertStringToWString(_sourcePath.c_str());
        const HRESULT argumentsResult = _utils.BuildArguments(
            sourcePath.c_str(),
            _entryPoint,
            _target,
            arguments,
            static_cast<uint32_t>(sizeof(arguments) / sizeof(arguments[0])),
            nullptr,
            0,
            &_compilerArguments
        );

        if (SUCCEEDED(argumentsResult) && _compilerArguments)
        {
            return true;
        }

        _loadingError = std::string("Failed to build DXC compiler arguments: ") + _sourcePath.c_str();
        return false;
    }

    bool CompileSourceBuffer(
        IDxcCompiler3& _compiler,
        const ego::FileContent& _content,
        IDxcCompilerArgs& _compilerArguments,
        IDxcIncludeHandler& _includeHandler,
        Microsoft::WRL::ComPtr<IDxcResult>& _compileResult,
        std::string& _loadingError
    )
    {
        DxcBuffer sourceBuffer = {};
        sourceBuffer.Ptr = _content.data();
        sourceBuffer.Size = _content.size();
        sourceBuffer.Encoding = DXC_CP_UTF8;

        const HRESULT compileCallResult = _compiler.Compile(
            &sourceBuffer,
            _compilerArguments.GetArguments(),
            _compilerArguments.GetCount(),
            &_includeHandler,
            IID_PPV_ARGS(&_compileResult)
        );

        if (SUCCEEDED(compileCallResult) && _compileResult)
        {
            return true;
        }

        _loadingError = "Failed to call DXC compiler.";
        return false;
    }

    std::string ReadCompileError(IDxcResult& _compileResult)
    {
        Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
        const HRESULT errorResult = _compileResult.GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
        if (FAILED(errorResult) || !errors || errors->GetStringLength() == 0)
        {
            return std::string();
        }

        return std::string(errors->GetStringPointer(), errors->GetStringLength());
    }

    std::string FormatHResult(HRESULT _result)
    {
        std::ostringstream stream;
        stream << "0x" << std::uppercase << std::hex << static_cast<uint32_t>(_result);
        return stream.str();
    }

    bool CheckCompileStatus(IDxcResult& _compileResult, std::string& _loadingError)
    {
        HRESULT compileStatus = S_OK;
        const HRESULT statusResult = _compileResult.GetStatus(&compileStatus);
        if (FAILED(statusResult))
        {
            _loadingError = std::string("Failed to read DXC compile status: ") + FormatHResult(statusResult);
            return false;
        }

        if (SUCCEEDED(compileStatus))
        {
            return true;
        }

        _loadingError = ReadCompileError(_compileResult);
        if (_loadingError.empty())
        {
            _loadingError = std::string("DXC compilation failed: ") + FormatHResult(compileStatus);
        }

        return false;
    }

    bool ReadCompiledShader(
        IDxcResult& _compileResult,
        ego::FileContent& _compiledContent,
        std::string& _loadingError
    )
    {
        Microsoft::WRL::ComPtr<IDxcBlob> compiledShader;
        const HRESULT objectResult = _compileResult.GetOutput(
            DXC_OUT_OBJECT,
            IID_PPV_ARGS(&compiledShader),
            nullptr
        );
        if (FAILED(objectResult) || !compiledShader)
        {
            _loadingError = "Failed to read DXC object output.";
            return false;
        }

        const size_t compiledSize = compiledShader->GetBufferSize();
        if (compiledSize == 0 || compiledSize > (std::numeric_limits<uint32_t>::max)())
        {
            _loadingError = "DXC object output is empty or too large.";
            return false;
        }

        _compiledContent.resize(compiledSize);
        std::memcpy(_compiledContent.data(), compiledShader->GetBufferPointer(), compiledSize);
        return true;
    }
}

bool ego::resources::dxc::CompileHlslContent(
    const FileContent& _content,
    const FileName& _sourcePath,
    gpu::ShaderStage _stage,
    ResourceLoadingContext& _loadingContext,
    FileContent& _compiledContent,
    std::string& _loadingError
)
{
    _loadingError.clear();

    const wchar_t* entryPoint = GetShaderEntryPoint(_stage);
    const wchar_t* target = GetShaderTarget(_stage);
    if (_content.empty() || !entryPoint || !target)
    {
        _loadingError = std::string("Invalid HLSL shader content or stage: ") + _sourcePath.c_str();
        _compiledContent.clear();
        return false;
    }

    Microsoft::WRL::ComPtr<IDxcUtils> utils;
    Microsoft::WRL::ComPtr<IDxcCompiler3> compiler;
    if (!CreateDxcInstances(utils, compiler, _loadingError))
    {
        _compiledContent.clear();
        return false;
    }

    Microsoft::WRL::ComPtr<DXCResourceIncludeHandler> includeHandler =
        Microsoft::WRL::Make<DXCResourceIncludeHandler>(utils.Get(), _loadingContext, _sourcePath);
    if (!includeHandler)
    {
        _loadingError = "Failed to create DXC include handler.";
        _compiledContent.clear();
        return false;
    }

    Microsoft::WRL::ComPtr<IDxcCompilerArgs> compilerArguments;
    if (!BuildCompilerArguments(*utils.Get(), _sourcePath, entryPoint, target, compilerArguments, _loadingError))
    {
        _compiledContent.clear();
        return false;
    }

    Microsoft::WRL::ComPtr<IDxcResult> compileResult;
    if (!CompileSourceBuffer(
            *compiler.Get(),
            _content,
            *compilerArguments.Get(),
            *includeHandler.Get(),
            compileResult,
            _loadingError
        ))
    {
        _compiledContent.clear();
        return false;
    }

    if (!CheckCompileStatus(*compileResult.Get(), _loadingError))
    {
        _compiledContent.clear();
        return false;
    }

    if (!ReadCompiledShader(*compileResult.Get(), _compiledContent, _loadingError))
    {
        _compiledContent.clear();
        return false;
    }

    return true;
}
