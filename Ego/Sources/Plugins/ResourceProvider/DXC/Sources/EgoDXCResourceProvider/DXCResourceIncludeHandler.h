#pragma once

#include "EgoCore/FileName/FileName.h"

#include "EgoEngine/Platform/FileSystem/FileSystem.h"

#include <Windows.h>
#include <Unknwn.h>
#include <ObjIdl.h>
#include <WTypes.h>
#include <dxcapi.h>
#include <wrl/client.h>
#include <wrl/implements.h>

#include <vector>

namespace ego
{
    class ResourceLoadingContext;
}

namespace ego::resources::dxc
{
    class DXCResourceIncludeHandler final
        : public Microsoft::WRL::RuntimeClass<
            Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
            IDxcIncludeHandler>
    {
    public:
        DXCResourceIncludeHandler(
            IDxcUtils* _utils,
            ResourceLoadingContext& _loadingContext,
            const FileName& _sourcePath
        );

        virtual HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR _filename, IDxcBlob** _includeSource) override;

    private:
        void AddIncludeDirectory(const FileName& _directory);

        bool LoadIncludeContent(
            const FileName& _includePath,
            FileName& _loadedPath,
            FileContent& _content
        ) const;

        HRESULT CreateBlob(const FileContent& _content, IDxcBlob** _includeSource) const;

        Microsoft::WRL::ComPtr<IDxcUtils> m_utils;
        ResourceLoadingContext* m_loadingContext = nullptr;
        std::vector<FileName> m_includeDirectories;
    };
}
