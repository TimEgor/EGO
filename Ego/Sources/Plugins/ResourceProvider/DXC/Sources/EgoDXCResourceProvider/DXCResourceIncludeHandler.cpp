#include "DXCResourceIncludeHandler.h"

#include "DXCResourceUtils.h"

#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/String/StringConverter.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>

ego::resources::dxc::DXCResourceIncludeHandler::DXCResourceIncludeHandler(
    IDxcUtils* _utils,
    ResourceLoadingContext& _loadingContext,
    const FileName& _sourcePath
)
    : m_utils(_utils)
    , m_loadingContext(&_loadingContext)
{
    AddIncludeDirectory(file_name_utils::GetFileDirPath(_sourcePath));
}

HRESULT STDMETHODCALLTYPE ego::resources::dxc::DXCResourceIncludeHandler::QueryInterface(REFIID _riid, void** _object)
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

ULONG STDMETHODCALLTYPE ego::resources::dxc::DXCResourceIncludeHandler::AddRef()
{
    return ++m_refCount;
}

ULONG STDMETHODCALLTYPE ego::resources::dxc::DXCResourceIncludeHandler::Release()
{
    const ULONG refCount = --m_refCount;
    if (refCount == 0)
    {
        delete this;
    }

    return refCount;
}

HRESULT STDMETHODCALLTYPE ego::resources::dxc::DXCResourceIncludeHandler::LoadSource(
    LPCWSTR _filename,
    IDxcBlob** _includeSource
)
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

    FileName loadedPath;
    FileContent includeContent;
    if (!LoadIncludeContent(ToFileName(_filename), loadedPath, includeContent))
    {
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    AddIncludeDirectory(file_name_utils::GetFileDirPath(loadedPath));
    return CreateBlob(includeContent, _includeSource);
}

ego::FileName ego::resources::dxc::DXCResourceIncludeHandler::ToFileName(LPCWSTR _filename)
{
    return FileName(ConvertWStringToString(std::wstring(_filename)));
}

void ego::resources::dxc::DXCResourceIncludeHandler::AddIncludeDirectory(const FileName& _directory)
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

bool ego::resources::dxc::DXCResourceIncludeHandler::TryLoadInclude(
    const FileName& _path,
    FileName& _loadedPath,
    FileContent& _content
) const
{
    if (TryLoadContent(*m_loadingContext, _path, _content))
    {
        _loadedPath = _path;
        return true;
    }

    return false;
}

bool ego::resources::dxc::DXCResourceIncludeHandler::LoadIncludeContent(
    const FileName& _includePath,
    FileName& _loadedPath,
    FileContent& _content
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
        const FileName includePath = *directoryIt + "/" + _includePath;
        if (TryLoadInclude(includePath, _loadedPath, _content))
        {
            return true;
        }
    }

    _content.clear();
    return false;
}

HRESULT ego::resources::dxc::DXCResourceIncludeHandler::CreateBlob(
    const FileContent& _content,
    IDxcBlob** _includeSource
) const
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
