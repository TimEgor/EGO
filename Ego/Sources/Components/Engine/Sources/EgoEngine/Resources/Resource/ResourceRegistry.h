#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

#include "EgoCore/Job/Job.h"

#include "Resource.h"

namespace ego
{
    class ResourceRegistry final
    {
    public:
        using ResourceFactory = ego::ResourceFactory;

        ResourcePointer getResource(FileNameID _id) const;
        ResourcePointer getRegisteredResource(Resource& _resource) const;

        ResourcePointer getOrCreateResource(
            ResourceType _type,
            const FileName& _path,
            ResourceID _id,
            const ResourceFactory& _factory,
            bool& _needLoading
        );

        bool removeResource(Resource* _resource);
        void setLoadingJob(const ResourcePointer& _resource, const JobReference& _job);
        JobReference getLoadingJob(const ResourcePointer& _resource) const;
        void collectLoadingJobs(std::vector<JobReference>& _jobs) const;
        void clear();

    private:
        struct ResourceEntry final
        {
            ResourceType m_type = InvalidResourceType;
            FileName m_path;
            ResourceFactory m_factory;
            ResourceWeakPointer m_resource;
            Resource* m_resourcePtr = nullptr;
            JobWeakReference m_loadingJob;
        };

        using ResourceCollection = std::unordered_map<ResourceID, ResourceEntry>;

        mutable std::mutex m_mutex;
        ResourceCollection m_resources;
    };
}
