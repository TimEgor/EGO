#pragma once

#include "Resource.h"

#include <mutex>
#include <string>

namespace ego
{
    class ResourceProvider
    {
    public:
        ResourceProvider() = default;
        virtual ~ResourceProvider() = default;

        bool provideContent(
            const Resource& _resource,
            const FileName& _path,
            ResourceLoadingContext& _loadingContext,
            FileContent& _content,
            std::string& _loadingError
        );

        std::string getLoadingError() const;

    protected:
        virtual bool onProvideContent(
            const Resource& _resource,
            const FileName& _path,
            ResourceLoadingContext& _loadingContext,
            FileContent& _content
        ) = 0;

        void clearLoadingError();
        void setLoadingError(std::string _loadingError);

    private:
        mutable std::recursive_mutex m_mutex;
        std::string m_loadingError;
    };

    EGO_POINTER(ResourceProvider);
}
