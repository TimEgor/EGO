#pragma once

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoEngine/Platform/FileSystem/FileSystem.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace ego
{
    class ResourceController;
    class ResourceLoadingContext;

    using ResourceType = rtti::TypeMetaInfoID;
    inline constexpr ResourceType InvalidResourceType = rtti::InvalidTypeMetaInfoID;
    using ResourceID = FileNameID;
    inline constexpr ResourceID InvalidResourceID = InvalidFileNameID;

    enum class ResourceState
    {
        Undefined,
        Loading,
        Loaded,
        Failed
    };

    class Resource;
    EGO_POINTER(Resource);
    EGO_WEAK_POINTER(Resource);

    struct ResourceDeleter final
    {
        void operator()(Resource* _resource) const;
    };

    using ResourceFactory = std::function<ResourcePointer()>;

    class Resource
    {
    public:
        using DependencyCollection = std::vector<ResourcePointer>;

        class ResourceAccessor final : public NonInstanceable
        {
            friend class ResourceController;
            friend class ResourceLoadingContext;
            friend struct ResourceDeleter;

            static void PrepareLoading(Resource* _resource, const FileName& _path);
            static void SetLoadingError(Resource* _resource, std::string _loadingError);
            static void SetState(Resource* _resource, ResourceState _state);
            static void Unload(Resource* _resource);
            static void AddDependency(Resource* _resource, const ResourcePointer& _dependency);
            static void GetDependencies(const Resource* _resource, DependencyCollection& _dependencies);
        };

        Resource() = default;
        virtual ~Resource();

        FileName getPath() const;
        ResourceState getState() const;
        std::string getLoadingError() const;

        bool isLoading() const;
        bool isLoaded() const;
        bool isFailed() const;

        bool load(const FileName& _path, FileContent&& _content, ResourceLoadingContext& _loadingContext);

        virtual ResourceType getType() const = 0;

        EGO_RTTI_VIRTUAL_BASE(Resource);

    protected:
        virtual bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) = 0;
        virtual void onUnload();

    private:
        void clearDependencies();
        void addDependency(const ResourcePointer& _resource);
        void getDependencies(DependencyCollection& _dependencies) const;

        void prepareLoading(const FileName& _path);
        void setLoadingError(std::string _loadingError);
        void setState(ResourceState _state);
        void unload();

        mutable std::mutex m_mutex;

        FileName m_path;
        std::string m_loadingError;
        DependencyCollection m_dependencies;
        std::atomic<ResourceState> m_state = ResourceState::Undefined;
    };

    template <typename TResource, typename... TArgs>
    SharedPointer<TResource> CreateResource(TArgs&&... _args)
    {
        static_assert(std::is_base_of_v<Resource, TResource>);
        return SharedPointer<TResource>(
            new TResource(std::forward<TArgs>(_args)...),
            ResourceDeleter{}
        );
    }
}

#define EGO_RESOURCE(_RESOURCE, ...)                               \
    EGO_RTTI_VIRTUAL(_RESOURCE, __VA_ARGS__)                       \
                                                                   \
    static const char* GetResourceTypeName()                       \
    {                                                              \
        return GetMetaInfoTypeName();                              \
    }                                                              \
                                                                   \
    static ego::ResourceType GetResourceType()                     \
    {                                                              \
        return GetMetaInfoID();                                    \
    }                                                              \
                                                                   \
    virtual ego::ResourceType getType() const override             \
    {                                                              \
        return GetResourceType();                                  \
    }

#define EGO_RESOURCE_TYPE(_RESOURCE) (_RESOURCE::GetResourceType())
#define EGO_RESOURCE_TYPE_NAME(_RESOURCE) (_RESOURCE::GetResourceTypeName())
