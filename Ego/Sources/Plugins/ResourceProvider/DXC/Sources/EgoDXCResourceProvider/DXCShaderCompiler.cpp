#include "DXCShaderCompiler.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <string>

#include "EgoCore/String/StringConverter.h"

#include "DXCResourceIncludeHandler.h"

#include <Windows.h>
#include <Unknwn.h>
#include <ObjIdl.h>
#include <WTypes.h>
#include <dxcapi.h>
#include <wrl/client.h>

namespace ego::resources::dxc
{
    DXCShaderCompiler::DXCShaderCompiler(ResourceLoadingContext& _loadingContext)
        : m_loadingContext(_loadingContext)
    {
    }

    bool DXCShaderCompiler::compileHlslContent(
        const FileContent& _content,
        const FileName& _sourcePath,
        gpu::ShaderStage _stage,
        FileContent& _compiledContent,
        std::string& _loadingError) const
    {
        _loadingError.clear();
        _compiledContent.clear();

        const wchar_t* entryPoint = getShaderEntryPoint(_stage);
        const wchar_t* target = getShaderTarget(_stage);
        if (_content.empty() || !target)
        {
            _loadingError = std::string("Invalid HLSL shader content or stage: ") + _sourcePath.c_str();
            return false;
        }

        Microsoft::WRL::ComPtr<IDxcUtils> utils;
        const HRESULT utilsResult = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
        if (FAILED(utilsResult) || !utils)
        {
            _loadingError = "Failed to create DXC utils.";
            return false;
        }

        Microsoft::WRL::ComPtr<IDxcCompiler3> compiler;
        const HRESULT compilerResult = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
        if (FAILED(compilerResult) || !compiler)
        {
            _loadingError = "Failed to create DXC compiler.";
            return false;
        }

        Microsoft::WRL::ComPtr<DXCResourceIncludeHandler> includeHandler = Microsoft::WRL::Make<DXCResourceIncludeHandler>(utils.Get(), m_loadingContext, _sourcePath);
        if (!includeHandler)
        {
            _loadingError = "Failed to create DXC include handler.";
            return false;
        }

        LPCWSTR arguments[] = {L"-HV", L"2021", DXC_ARG_WARNINGS_ARE_ERRORS};

        Microsoft::WRL::ComPtr<IDxcCompilerArgs> compilerArguments;
        const std::wstring sourcePath = ConvertStringToWString(_sourcePath.c_str());
        const HRESULT argumentsResult = utils->BuildArguments(
            sourcePath.c_str(),
            entryPoint,
            target,
            arguments,
            static_cast<uint32_t>(sizeof(arguments) / sizeof(arguments[0])),
            nullptr,
            0,
            &compilerArguments);
        if (FAILED(argumentsResult) || !compilerArguments)
        {
            _loadingError = std::string("Failed to build DXC compiler arguments: ") + _sourcePath.c_str();
            return false;
        }

        DxcBuffer sourceBuffer = {};
        sourceBuffer.Ptr = _content.data();
        sourceBuffer.Size = _content.size();
        sourceBuffer.Encoding = DXC_CP_UTF8;

        Microsoft::WRL::ComPtr<IDxcResult> compileResult;
        const HRESULT compileCallResult =
            compiler->Compile(&sourceBuffer, compilerArguments->GetArguments(), compilerArguments->GetCount(), includeHandler.Get(), IID_PPV_ARGS(&compileResult));
        if (FAILED(compileCallResult) || !compileResult)
        {
            _loadingError = "Failed to call DXC compiler.";
            return false;
        }

        HRESULT compileStatus = S_OK;
        const HRESULT statusResult = compileResult->GetStatus(&compileStatus);
        if (FAILED(statusResult))
        {
            _loadingError = std::string("Failed to read DXC compile status: ") + getHResultText(statusResult);
            return false;
        }

        if (FAILED(compileStatus))
        {
            _loadingError = readCompileError(*compileResult.Get());
            if (_loadingError.empty())
            {
                _loadingError = std::string("DXC compilation failed: ") + getHResultText(compileStatus);
            }

            return false;
        }

        Microsoft::WRL::ComPtr<IDxcBlob> compiledShader;
        const HRESULT objectResult = compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiledShader), nullptr);
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

    const wchar_t* DXCShaderCompiler::getShaderEntryPoint(gpu::ShaderStage _stage) const
    {
        switch (_stage)
        {
        case gpu::ShaderStage::Vertex:
            return L"VSMain";
        case gpu::ShaderStage::Pixel:
            return L"PSMain";
        case gpu::ShaderStage::Compute:
            return L"CSMain";
        case gpu::ShaderStage::RayGeneration:
        case gpu::ShaderStage::Miss:
        case gpu::ShaderStage::ClosestHit:
            return nullptr;
        default:
            break;
        }

        return nullptr;
    }

    const wchar_t* DXCShaderCompiler::getShaderTarget(gpu::ShaderStage _stage) const
    {
        switch (_stage)
        {
        case gpu::ShaderStage::Vertex:
            return L"vs_6_6";
        case gpu::ShaderStage::Pixel:
            return L"ps_6_6";
        case gpu::ShaderStage::Compute:
            return L"cs_6_6";
        case gpu::ShaderStage::RayGeneration:
        case gpu::ShaderStage::Miss:
        case gpu::ShaderStage::ClosestHit:
            return L"lib_6_6";
        default:
            break;
        }

        return nullptr;
    }

    std::string DXCShaderCompiler::getHResultText(long _result) const
    {
        return "HRESULT " + std::to_string(_result);
    }

    std::string DXCShaderCompiler::readCompileError(IDxcResult& _compileResult) const
    {
        Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
        const HRESULT errorResult = _compileResult.GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
        if (FAILED(errorResult) || !errors || errors->GetStringLength() == 0)
        {
            return std::string();
        }

        return std::string(errors->GetStringPointer(), errors->GetStringLength());
    }
} // namespace ego::resources::dxc
