#include "DXCResourceIncludeHandler.h"

#include "DXCResourceUtils.h"

#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/String/StringConverter.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace
{
    ego::FileName ToFileName(LPCWSTR _filename)
    {
        return ego::FileName(ego::ConvertWStringToString(std::wstring(_filename)));
    }

    std::string NormalizeIncludePath(const ego::FileName& _path)
    {
        std::string path = _path.c_str();
        std::replace(path.begin(), path.end(), '\\', '/');

        while (path.size() >= 2 && path[0] == '.' && path[1] == '/')
        {
            path.erase(0, 2);
        }

        return path;
    }

    ego::FileName StripIncludeDirectoryPrefix(
        const ego::FileName& _path,
        const ego::FileName& _directory
    )
    {
        const std::string path = NormalizeIncludePath(_path);
        const std::string directory = NormalizeIncludePath(_directory);

        if (directory.empty() ||
            path.size() <= directory.size() ||
            path.compare(0, directory.size(), directory) != 0 ||
            path[directory.size()] != '/')
        {
            return ego::FileName();
        }

        return ego::FileName(path.substr(directory.size() + 1));
    }

    void AddCandidate(std::vector<ego::FileName>& _candidates, const ego::FileName& _candidate)
    {
        if (!_candidate)
        {
            return;
        }

        const auto foundIt = std::find(_candidates.begin(), _candidates.end(), _candidate);
        if (foundIt == _candidates.end())
        {
            _candidates.push_back(_candidate);
        }
    }
}

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

    std::vector<FileName> candidates;
    AddCandidate(candidates, _includePath);

    for (auto directoryIt = m_includeDirectories.rbegin();
        directoryIt != m_includeDirectories.rend();
        ++directoryIt)
    {
        AddCandidate(candidates, StripIncludeDirectoryPrefix(_includePath, *directoryIt));
    }

    if (!IsRootedPath(_includePath.c_str()))
    {
        for (auto directoryIt = m_includeDirectories.rbegin();
            directoryIt != m_includeDirectories.rend();
            ++directoryIt)
        {
            AddCandidate(candidates, *directoryIt + "/" + _includePath);
        }
    }

    for (const FileName& candidate : candidates)
    {
        if (TryLoadContent(*m_loadingContext, candidate, _content))
        {
            _loadedPath = candidate;
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
