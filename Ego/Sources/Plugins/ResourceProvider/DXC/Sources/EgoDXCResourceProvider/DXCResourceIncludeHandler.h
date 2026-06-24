#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "EgoCore/FileName/FileName.h"

#include "EgoEngine/Platform/FileSystem/FileSystem.h"

#include <Windows.h>
#include <Unknwn.h>
#include <ObjIdl.h>
#include <WTypes.h>
#include <dxcapi.h>
#include <wrl/client.h>
#include <wrl/implements.h>

namespace ego
{
    class ResourceLoadingContext;
}

namespace ego::resources::dxc
{
    class DXCResourceIncludeHandler final : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IDxcIncludeHandler>
    {
    public:
        DXCResourceIncludeHandler(IDxcUtils* _utils, ResourceLoadingContext& _loadingContext, const FileName& _sourcePath);

        HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR _filename, IDxcBlob** _includeSource) override;

    private:
        FileName toFileName(LPCWSTR _filename) const;
        std::string normalizeIncludePath(const FileName& _path) const;
        bool isRootedPath(std::string_view _path) const;
        bool loadContent(const FileName& _path, FileContent& _content) const;

        FileName stripIncludeDirectoryPrefix(const FileName& _path, const FileName& _directory) const;

        void addCandidate(std::vector<FileName>& _candidates, const FileName& _candidate) const;
        void addIncludeDirectory(const FileName& _directory);

        bool loadIncludeContent(const FileName& _includePath, FileName& _loadedPath, FileContent& _content) const;

        HRESULT createBlob(const FileContent& _content, IDxcBlob** _includeSource) const;

        Microsoft::WRL::ComPtr<IDxcUtils> m_utils;
        ResourceLoadingContext& m_loadingContext;
        std::vector<FileName> m_includeDirectories;
    };
} // namespace ego::resources::dxc
