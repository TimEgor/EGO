#include "DXCResourceIncludeHandler.h"

#include <algorithm>
#include <cstdint>
#include <limits>

#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/String/StringConverter.h"

#include "EgoRuntime/Resource/ResourceLoadingContext.h"

namespace ego::resources::dxc
{
    DXCResourceIncludeHandler::DXCResourceIncludeHandler(IDxcUtils* _utils, ResourceLoadingContext& _loadingContext, const FileName& _sourcePath)
        : m_utils(_utils),
          m_loadingContext(_loadingContext)
    {
        addIncludeDirectory(file_name_utils::GetFileDirPath(_sourcePath));
    }

    HRESULT STDMETHODCALLTYPE DXCResourceIncludeHandler::LoadSource(LPCWSTR _filename, IDxcBlob** _includeSource)
    {
        if (!_includeSource)
        {
            return E_POINTER;
        }

        *_includeSource = nullptr;
        if (!_filename || !m_utils)
        {
            return E_INVALIDARG;
        }

        FileName loadedPath;
        FileContent includeContent;
        if (!loadIncludeContent(toFileName(_filename), loadedPath, includeContent))
        {
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }

        addIncludeDirectory(file_name_utils::GetFileDirPath(loadedPath));
        return createBlob(includeContent, _includeSource);
    }

    FileName DXCResourceIncludeHandler::toFileName(LPCWSTR _filename) const
    {
        return FileName(ConvertWStringToString(std::wstring(_filename)));
    }

    std::string DXCResourceIncludeHandler::normalizeIncludePath(const FileName& _path) const
    {
        std::string path = _path.c_str();
        std::replace(path.begin(), path.end(), '\\', '/');

        while (path.size() >= 2 && path[0] == '.' && path[1] == '/')
        {
            path.erase(0, 2);
        }

        return path;
    }

    bool DXCResourceIncludeHandler::isRootedPath(std::string_view _path) const
    {
        return (!_path.empty() && (_path[0] == '/' || _path[0] == '\\')) || (_path.size() > 1 && _path[1] == ':');
    }

    bool DXCResourceIncludeHandler::loadContent(const FileName& _path, FileContent& _content) const
    {
        _content.clear();
        return _path && m_loadingContext.loadContent(_path, _content);
    }

    FileName DXCResourceIncludeHandler::stripIncludeDirectoryPrefix(const FileName& _path, const FileName& _directory) const
    {
        const std::string path = normalizeIncludePath(_path);
        const std::string directory = normalizeIncludePath(_directory);

        if (directory.empty() || path.size() <= directory.size() || path.compare(0, directory.size(), directory) != 0 || path[directory.size()] != '/')
        {
            return FileName();
        }

        return FileName(path.substr(directory.size() + 1));
    }

    void DXCResourceIncludeHandler::addCandidate(std::vector<FileName>& _candidates, const FileName& _candidate) const
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

    void DXCResourceIncludeHandler::addIncludeDirectory(const FileName& _directory)
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

    bool DXCResourceIncludeHandler::loadIncludeContent(const FileName& _includePath, FileName& _loadedPath, FileContent& _content) const
    {
        if (!_includePath)
        {
            _content.clear();
            return false;
        }

        std::vector<FileName> candidates;
        addCandidate(candidates, _includePath);

        for (auto directoryIt = m_includeDirectories.rbegin(); directoryIt != m_includeDirectories.rend(); ++directoryIt)
        {
            addCandidate(candidates, stripIncludeDirectoryPrefix(_includePath, *directoryIt));
        }

        if (!isRootedPath(_includePath.c_str()))
        {
            for (auto directoryIt = m_includeDirectories.rbegin(); directoryIt != m_includeDirectories.rend(); ++directoryIt)
            {
                addCandidate(candidates, *directoryIt + "/" + _includePath);
            }
        }

        for (const FileName& candidate : candidates)
        {
            if (loadContent(candidate, _content))
            {
                _loadedPath = candidate;
                return true;
            }
        }

        _content.clear();
        return false;
    }

    HRESULT DXCResourceIncludeHandler::createBlob(const FileContent& _content, IDxcBlob** _includeSource) const
    {
        if (_content.size() > (std::numeric_limits<uint32_t>::max)())
        {
            return E_OUTOFMEMORY;
        }

        static constexpr uint8_t EmptyData = 0;
        const void* data = _content.empty() ? &EmptyData : _content.data();

        Microsoft::WRL::ComPtr<IDxcBlobEncoding> includeSource;
        const HRESULT result = m_utils->CreateBlob(data, static_cast<uint32_t>(_content.size()), DXC_CP_UTF8, &includeSource);
        if (FAILED(result) || !includeSource)
        {
            return FAILED(result) ? result : E_FAIL;
        }

        *_includeSource = includeSource.Detach();
        return S_OK;
    }
} // namespace ego::resources::dxc