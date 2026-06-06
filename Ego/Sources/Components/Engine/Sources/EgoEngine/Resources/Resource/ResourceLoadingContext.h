#pragma once

#include "ResourceController.h"

namespace ego
{
    class ResourceLoadingContext final
    {
        friend class ResourceController;

    public:
        ResourceLoadingContext(ResourceController& _controller, Resource& _ownerResource, bool _isAsyncLoading);

        template <typename TResource>
        SharedPointer<TResource> loadDependency(const FileName& _path)
        {
            static_assert(std::is_base_of_v<Resource, TResource>);

            SharedPointer<TResource> resource = m_controller.load<TResource>(_path);

            if (resource && !addDependency(resource))
            {
                return nullptr;
            }

            return resource;
        }

        template <typename TResource>
        SharedPointer<TResource> loadAsyncDependency(const FileName& _path)
        {
            static_assert(std::is_base_of_v<Resource, TResource>);

            ResourceLoadingOperationPointer operation = m_controller.loadAsync<TResource>(_path);
            SharedPointer<TResource> resource = operation ? operation->getResource<TResource>() : nullptr;

            if (resource && !addDependency(resource))
            {
                return nullptr;
            }

            if (resource)
            {
                m_hasAsyncDependencies = true;
            }

            return resource;
        }

        bool addDependency(const ResourcePointer& _dependency);

        bool isAsyncLoading() const;
        bool loadContent(const FileName& _path, FileContent& _content) const;

    private:
        Resource::DependencyCollection takeDependencies();
        bool hasAsyncDependencies() const;

        ResourceController& m_controller;
        Resource& m_ownerResource;
        Resource::DependencyCollection m_dependencies;
        bool m_isAsyncLoading = false;
        bool m_hasAsyncDependencies = false;
    };
}
