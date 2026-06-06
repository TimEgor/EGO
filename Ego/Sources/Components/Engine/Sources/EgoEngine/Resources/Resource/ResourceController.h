#pragma once

#include <functional>
#include <string>
#include <type_traits>
#include <vector>

#include "EgoCore/Job/Job.h"
#include "EgoCore/Job/JobController.h"
#include "EgoCore/Patterns/NonInstanceable.h"

#include "Resource.h"
#include "ResourceDependencyGraph.h"
#include "ResourceProvider.h"
#include "ResourceRegistry.h"
#include "ResourceSourceRegistry.h"

namespace ego
{
    class ResourceController;
    EGO_POINTER(ResourceController);
    EGO_WEAK_POINTER(ResourceController);

    class ResourceLoadingOperation final
    {
    public:
        ResourceLoadingOperation(
            const ResourceControllerWeakPointer& _controller,
            const ResourcePointer& _resource
        );

        ResourcePointer getResource() const;

        template <typename TResource>
        SharedPointer<TResource> getResource() const
        {
            static_assert(std::is_base_of_v<Resource, TResource>);

            return m_resource && m_resource->getType() == EGO_RESOURCE_TYPE(TResource)
                ? StaticPointerCast<TResource>(m_resource)
                : nullptr;
        }

        ResourceState getState() const;

        bool isFinished() const;
        bool isLoaded() const;
        bool isFailed() const;
        bool waitLoading();

    private:
        ResourceControllerWeakPointer m_controller;
        ResourcePointer m_resource;
    };

    EGO_POINTER(ResourceLoadingOperation);

    class ResourceController final : public EnableSharedFromThis<ResourceController>
    {
    public:
        using ResourceFactory = ego::ResourceFactory;

        class ResourceControllerAccessor final : public NonInstanceable
        {
            friend struct ResourceDeleter;
            friend class ResourceLoadingContext;

            static bool RemoveResource(ResourceController& _controller, Resource* _resource);
            static bool LoadResourceContent(
                const ResourceController& _controller,
                const FileName& _path,
                FileContent& _content
            );
            static bool AddDependency(
                ResourceController& _controller,
                Resource& _resource,
                const ResourcePointer& _dependency
            );
        };

        ResourceController() = default;
        ~ResourceController();

        bool init(uint32_t _threadCount = 0, const char* _jobThreadName = "EgoResourceJob");
        void release();

        bool isInitialized() const;

        void addFileSystem(const FileSystemPointer& _fileSystem);
        bool removeFileSystem(const FileSystemPointer& _fileSystem);
        void clearFileSystems();

        bool addResourceProvider(const FileName& _extension, const ResourceProviderPointer& _provider);
        bool removeResourceProvider(const FileName& _extension);
        void clearResourceProviders();

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

        template <typename TResource>
        ResourceLoadingOperationPointer loadAsync(const FileName& _path)
        {
            static_assert(std::is_base_of_v<Resource, TResource>);

            return loadResourceAsync(
                EGO_RESOURCE_TYPE(TResource),
                _path,
                []() { return CreateResource<TResource>(); }
            );
        }

        template <typename TResource>
        SharedPointer<TResource> getResource(const FileName& _path) const
        {
            static_assert(std::is_base_of_v<Resource, TResource>);

            ResourcePointer resource = getResource(_path);
            return resource && resource->getType() == EGO_RESOURCE_TYPE(TResource)
                ? StaticPointerCast<TResource>(resource)
                : nullptr;
        }

        template <typename TResource>
        SharedPointer<TResource> getResource(FileNameID _id) const
        {
            static_assert(std::is_base_of_v<Resource, TResource>);

            ResourcePointer resource = getResource(_id);
            return resource && resource->getType() == EGO_RESOURCE_TYPE(TResource)
                ? StaticPointerCast<TResource>(resource)
                : nullptr;
        }

        ResourcePointer getResource(const FileName& _path) const;
        ResourcePointer getResource(FileNameID _id) const;

        template <typename TResource>
        bool isChildResourcesLoaded(const FileName& _path) const
        {
            static_assert(std::is_base_of_v<Resource, TResource>);
            return isChildResourcesLoaded(_path);
        }

        bool isChildResourcesLoaded(const FileName& _path) const;
        bool isChildResourcesLoaded(FileNameID _id) const;
        bool isChildResourcesLoaded(const ResourcePointer& _resource) const;

        bool waitLoading(const ResourcePointer& _resource);
        bool waitLoading(const FileName& _path);
        bool waitLoading(FileNameID _id);

        const JobController& getJobController() const;
        JobController& getJobController();

    private:
        ResourcePointer loadResource(
            ResourceType _type,
            const FileName& _path,
            const ResourceFactory& _factory
        );

        ResourceLoadingOperationPointer loadResourceAsync(
            ResourceType _type,
            const FileName& _path,
            const ResourceFactory& _factory
        );

        bool removeResource(Resource* _resource);
        bool addDependency(Resource& _resource, const ResourcePointer& _dependency);

        bool loadResourceContent(const FileName& _path, FileContent& _content) const;
        bool readResourceContent(
            const ResourcePointer& _resource,
            const FileName& _path,
            ResourceLoadingContext& _loadingContext,
            FileContent& _content
        );
        bool loadResourceData(
            const ResourcePointer& _resource,
            const FileName& _path,
            bool _isAsyncLoading
        );
        bool completeResourceLoad(
            const ResourcePointer& _resource,
            bool _isLoadSuccessful,
            ResourceLoadingContext& _loadingContext
        );
        void finishResourceLoaded(const ResourcePointer& _resource);
        void finishResourceFailed(const ResourcePointer& _resource, std::string _loadingError = {});
        bool beginResourceDependenciesLoading(
            Resource& _resource,
            Resource::DependencyCollection&& _dependencies
        );
        void notifyResourceLoadingFinished(const ResourcePointer& _resource);
        void schedulePendingLoadingCompletion(
            const ResourceDependencyGraph::PendingLoadingPointer& _pendingLoading
        );
        void completePendingLoading(const ResourceDependencyGraph::PendingLoadingPointer& _pendingLoading);
        bool waitResourceLoading(const ResourcePointer& _resource);

        void waitLoadingJobs(const std::vector<JobReference>& _jobs);
        void waitAllLoadingJobs();

        ResourceRegistry m_resourceRegistry;
        ResourceSourceRegistry m_resourceSources;
        ResourceDependencyGraph m_dependencyGraph;
        JobControllerPointer m_jobController = nullptr;
        bool m_isInitialized = false;
    };
}
