#pragma once

#include "Resource.h"
#include "ResourceProvider.h"

#include "EgoCore/Job/Job.h"
#include "EgoCore/Job/JobController.h"

#include <functional>
#include <mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ego
{
    class ResourceController final : public EnableSharedFromThis<ResourceController>
    {
        friend class ResourceLoadingContext;

    public:
        using ResourceFactory = ego::ResourceFactory;

        ResourceController() = default;
        ~ResourceController();

        bool init(uint32_t _threadCount = 0, const char* _jobThreadName = "ResourceJob");
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
        SharedPointer<TResource> loadAsync(const FileName& _path, JobReference* _job = nullptr)
        {
            static_assert(std::is_base_of_v<Resource, TResource>);

            return StaticPointerCast<TResource>(
                loadResourceAsync(
                    EGO_RESOURCE_TYPE(TResource),
                    _path,
                    []() { return CreateResource<TResource>(); },
                    _job
                )
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

        const JobController& getJobController() const;
        JobController& getJobController();

    private:
        struct ResourceEntry final
        {
            ResourceType m_type = InvalidResourceType;
            FileName m_path;
            ResourceFactory m_factory;
            ResourceWeakPointer m_resource;
            JobWeakReference m_loadingJob;
        };

        using FileSystemCollection = std::vector<FileSystemPointer>;
        using ResourceCollection = std::unordered_map<ResourceID, ResourceEntry>;
        using ResourceProviderCollection = std::unordered_map<std::string, ResourceProviderPointer>;

        ResourcePointer loadResource(
            ResourceType _type,
            const FileName& _path,
            const ResourceFactory& _factory
        );

        ResourcePointer loadResourceAsync(
            ResourceType _type,
            const FileName& _path,
            const ResourceFactory& _factory,
            JobReference* _job = nullptr
        );

        ResourcePointer getOrCreateResource(
            ResourceType _type,
            const FileName& _path,
            ResourceID _id,
            const ResourceFactory& _factory,
            bool& _needLoading
        );

        bool loadResourceContent(const FileName& _path, FileContent& _content) const;
        ResourceProviderPointer getResourceProvider(const FileName& _path) const;
        bool loadResourceData(
            const ResourcePointer& _resource,
            const FileName& _path,
            bool _asyncChildLoading
        );
        bool isChildResourcesLoaded(
            const ResourcePointer& _resource,
            std::unordered_set<const Resource*>& _checkedResources
        ) const;

        void collectLoadingJobs(std::vector<JobReference>& _jobs) const;
        void waitLoadingJobs(const std::vector<JobReference>& _jobs);
        void waitAllLoadingJobs();

        mutable std::recursive_mutex m_mutex;
        FileSystemCollection m_fileSystems;
        ResourceCollection m_resources;
        ResourceProviderCollection m_resourceProviders;
        JobControllerPointer m_jobController = nullptr;
        bool m_isInitialized = false;
    };

    EGO_POINTER(ResourceController);
    EGO_WEAK_POINTER(ResourceController);
}
