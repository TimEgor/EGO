#pragma once

#include "ResourceController.h"

namespace ego
{
    class ResourceLoadingContext final
    {
    public:
        ResourceLoadingContext(ResourceController& _controller, Resource& _ownerResource, bool _asyncLoading);

        template <typename TResource>
        SharedPointer<TResource> loadResource(const FileName& _path)
        {
            static_assert(std::is_base_of_v<Resource, TResource>);

            SharedPointer<TResource> resource = m_asyncLoading
                ? m_controller.loadAsync<TResource>(_path)
                : m_controller.load<TResource>(_path);

            if (resource)
            {
                Resource::ResourceAccessor::AddDependency(&m_ownerResource, resource);
            }

            return resource;
        }

        bool loadContent(const FileName& _path, FileContent& _content) const;

    private:
        ResourceController& m_controller;
        Resource& m_ownerResource;
        bool m_asyncLoading = false;
    };
}
