#pragma once

#include "Resource.h"

namespace ego
{
    class ResourceProvider
    {
    public:
        ResourceProvider() = default;
        virtual ~ResourceProvider() = default;

        virtual bool provideContent(
            const Resource& _resource,
            const FileName& _path,
            ResourceLoadingContext& _loadingContext,
            FileContent& _content
        ) = 0;
    };

    EGO_POINTER(ResourceProvider);
}
