#pragma once

#include "Resource.h"

#include <string>

namespace ego
{
    class ResourceProvider
    {
    public:
        ResourceProvider() = default;
        virtual ~ResourceProvider() = default;

        bool provideContent(const Resource& _resource, const FileName& _path, ResourceLoadingContext& _loadingContext, FileContent& _content, std::string& _loadingError);

    protected:
        virtual bool onProvideContent(
            const Resource& _resource,
            const FileName& _path,
            ResourceLoadingContext& _loadingContext,
            FileContent& _content,
            std::string& _loadingError) = 0;
    };

    EGO_POINTER(ResourceProvider);
} // namespace ego
