#pragma once

#include "Resource.h"

namespace ego
{
    class ResourceLoadingContext final
    {
    public:
        ResourceLoadingContext(ResourceController& _controller, Resource& _ownerResource, bool _asyncLoading);

        template <typename TResource>
        SharedPointer<TResource> load(const FileName& _path)
        {
            static_assert(std::is_base_of_v<Resource, TResource>);

            return StaticPointerCast<TResource>(
                loadResource(
                    EGO_RESOURCE_TYPE(TResource),
                    _path,
                    []() { return CreateResource<TResource>(); }
                )
            );
        }

        ResourcePointer loadResource(
            ResourceType _type,
            const FileName& _path,
            const ResourceFactory& _factory
        );

    private:
        ResourceController& m_controller;
        Resource& m_ownerResource;
        bool m_asyncLoading = false;
    };
}
