#include "DXCShaderResourceProvider.h"

#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/String/StringConverter.h"

#include "EgoEngine/Graphic/RenderHardware/Resources/ShaderResource.h"
#include "EgoEngine/Resources/Resource/ResourceLoadingContext.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <utility>
#include <vector>

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

    class ResourceIncludeHandler final : public IDxcIncludeHandler
    {
    public:
        ResourceIncludeHandler(
            IDxcUtils* _utils,
            ego::ResourceLoadingContext& _loadingContext,
            const ego::FileName& _sourcePath
        )
            : m_utils(_utils)
            , m_loadingContext(&_loadingContext)
        {
            AddIncludeDirectory(ego::file_name_utils::GetFileDirPath(_sourcePath));
        }

        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID _riid, void** _object) override
        {
            if (!_object)
            {
                return E_POINTER;
            }

            *_object = nullptr;
            if (_riid == __uuidof(IUnknown) || _riid == __uuidof(IDxcIncludeHandler))
            {
                *_object = static_cast<IDxcIncludeHandler*>(this);
                AddRef();
                return S_OK;
            }

            return E_NOINTERFACE;
        }

        virtual ULONG STDMETHODCALLTYPE AddRef() override
        {
            return ++m_refCount;
        }

        virtual ULONG STDMETHODCALLTYPE Release() override
        {
            const ULONG refCount = --m_refCount;
            if (refCount == 0)
            {
                delete this;
            }

            return refCount;
        }

        virtual HRESULT STDMETHODCALLTYPE LoadSource(
            LPCWSTR _filename,
            IDxcBlob** _includeSource
        ) override
        {
            if (!_includeSource)
            {
                return E_POINTER;
            }

            *_includeSource = nullptr;
            if (!_filename || !m_utils || !m_loadingContext)
            {
                return E_INVALIDARG;
            }

            ego::FileName loadedPath;
            ego::FileContent includeContent;
            if (!LoadIncludeContent(ToFileName(_filename), loadedPath, includeContent))
            {
                return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            }

            AddIncludeDirectory(ego::file_name_utils::GetFileDirPath(loadedPath));
            return CreateBlob(includeContent, _includeSource);
        }

    private:
        static ego::FileName ToFileName(LPCWSTR _filename)
        {
            return ego::FileName(ego::ConvertWStringToString(std::wstring(_filename)));
        }

        void AddIncludeDirectory(const ego::FileName& _directory)
        {
            if (!_directory)
            {
                return;
            }

            const auto foundIt = std::find(m_includeDirectories.begin(), m_includeDirectories.end(), _directory);
            if (foundIt == m_includeDirectories.end())
            {
                m_includeDirectories.push_back(_directory);
            }
        }

        bool TryLoadInclude(
            const ego::FileName& _path,
            ego::FileName& _loadedPath,
            ego::FileContent& _content
        ) const
        {
            if (TryLoadContent(*m_loadingContext, _path, _content))
            {
                _loadedPath = _path;
                return true;
            }

            return false;
        }

        bool LoadIncludeContent(
            const ego::FileName& _includePath,
            ego::FileName& _loadedPath,
            ego::FileContent& _content
        ) const
        {
            if (!_includePath)
            {
                _content.clear();
                return false;
            }

            if (TryLoadInclude(_includePath, _loadedPath, _content))
            {
                return true;
            }

            if (IsRootedPath(_includePath.c_str()))
            {
                _content.clear();
                return false;
            }

            for (auto directoryIt = m_includeDirectories.rbegin();
                directoryIt != m_includeDirectories.rend();
                ++directoryIt)
            {
                const ego::FileName includePath = *directoryIt + "/" + _includePath;
                if (TryLoadInclude(includePath, _loadedPath, _content))
                {
                    return true;
                }
            }

            _content.clear();
            return false;
        }

        HRESULT CreateBlob(const ego::FileContent& _content, IDxcBlob** _includeSource) const
        {
            if (_content.size() > (std::numeric_limits<uint32_t>::max)())
            {
                return E_OUTOFMEMORY;
            }

            static constexpr uint8_t EmptyData = 0;
            const void* data = _content.empty() ? &EmptyData : _content.data();

            Microsoft::WRL::ComPtr<IDxcBlobEncoding> includeSource;
            const HRESULT result = m_utils->CreateBlob(
                data,
                static_cast<uint32_t>(_content.size()),
                DXC_CP_UTF8,
                &includeSource
            );
            if (FAILED(result) || !includeSource)
            {
                return FAILED(result) ? result : E_FAIL;
            }

            *_includeSource = includeSource.Detach();
            return S_OK;
        }

        std::atomic<ULONG> m_refCount = 1;
        Microsoft::WRL::ComPtr<IDxcUtils> m_utils;
        ego::ResourceLoadingContext* m_loadingContext = nullptr;
        std::vector<ego::FileName> m_includeDirectories;
    };

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
        const ego::FileName& _sourcePath,
        ego::gpu::ShaderStage _stage,
        ego::ResourceLoadingContext& _loadingContext,
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
        includeHandler.Attach(new ResourceIncludeHandler(utils.Get(), _loadingContext, _sourcePath));
        if (!includeHandler)
        {
            _compiledContent.clear();
            return false;
        }

        LPCWSTR arguments[] =
        {
            L"-HV",
            L"2021",
            DXC_ARG_WARNINGS_ARE_ERRORS
        };

        const std::wstring sourcePath = ego::ConvertStringToWString(_sourcePath.c_str());
        Microsoft::WRL::ComPtr<IDxcCompilerArgs> compilerArguments;
        const HRESULT argumentsResult = utils->BuildArguments(
            sourcePath.c_str(),
            entryPoint,
            target,
            arguments,
            static_cast<uint32_t>(sizeof(arguments) / sizeof(arguments[0])),
            nullptr,
            0,
            &compilerArguments
        );
        if (FAILED(argumentsResult) || !compilerArguments)
        {
            _compiledContent.clear();
            return false;
        }

        DxcBuffer sourceBuffer = {};
        sourceBuffer.Ptr = _content.data();
        sourceBuffer.Size = _content.size();
        sourceBuffer.Encoding = DXC_CP_UTF8;

        Microsoft::WRL::ComPtr<IDxcResult> compileResult;
        const HRESULT compileCallResult = compiler->Compile(
            &sourceBuffer,
            compilerArguments->GetArguments(),
            compilerArguments->GetCount(),
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

bool ego::resources::dxc::DXCShaderResourceProvider::provideContent(
    const Resource& _resource,
    const FileName& _path,
    ResourceLoadingContext& _loadingContext,
    FileContent& _content
)
{
    if (!ego::rtti::IsObjectBasedOn<gpu::ShaderResource>(_resource))
    {
        _content.clear();
        return false;
    }

    const gpu::ShaderResource& shaderResource = static_cast<const gpu::ShaderResource&>(_resource);
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

    return CompileHlslContent(sourceContent, sourcePath, shaderResource.getShaderStage(), _loadingContext, _content);
}
