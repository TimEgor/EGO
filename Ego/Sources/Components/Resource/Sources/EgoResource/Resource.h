#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

namespace ego
{
    class ResourceController;
    class ResourceDependencyGraph;
    class ResourceLoadingContext;
    class ResourceRegistry;

    using ResourceType = rtti::TypeMetaInfoID;
    inline constexpr ResourceType InvalidResourceType = rtti::InvalidTypeMetaInfoID;
    using ResourceID = FileNameID;
    inline constexpr ResourceID InvalidResourceID = InvalidFileNameID;

    enum class ResourceState
    {
        Undefined,
        Loading,
        LoadingDependencies,
        Loaded,
        Failed
    };

    class Resource;
    EGO_POINTER(Resource);
    EGO_WEAK_POINTER(Resource);

    struct ResourceDeleter final
    {
        ResourceDeleter() = default;
        explicit ResourceDeleter(const WeakPointer<ResourceController>& _controller);

        void operator()(Resource* _resource) const;

    private:
        WeakPointer<ResourceController> m_controller;
    };

    using ResourceFactory = std::function<ResourcePointer()>;

    class Resource
    {
    public:
        using DependencyCollection = std::vector<ResourcePointer>;

        class ResourceAccessor final : public NonInstanceable
        {
            friend class ResourceController;
            friend class ResourceDependencyGraph;
            friend class ResourceLoadingContext;
            friend class ResourceRegistry;
            friend struct ResourceDeleter;

            static void PrepareLoading(Resource* _resource, const FileName& _path);
            static void SetLoadingError(Resource* _resource, std::string _loadingError);
            static void SetState(Resource* _resource, ResourceState _state);
            static void Unload(Resource* _resource);
            static void AddDependency(Resource* _resource, const ResourcePointer& _dependency);
            static void GetDependencies(const Resource* _resource, DependencyCollection& _dependencies);
            static bool OnDependenciesLoaded(Resource* _resource);
        };

        Resource() = default;
        virtual ~Resource();

        FileName getPath() const;
        ResourceState getState() const;
        std::string getLoadingError() const;

        bool isLoading() const;
        bool isLoadingDependencies() const;
        bool isLoaded() const;
        bool isFailed() const;

        bool load(const FileName& _path, FileContent&& _content, ResourceLoadingContext& _loadingContext);

        virtual ResourceType getType() const = 0;

        EGO_RTTI_VIRTUAL_BASE(Resource);

    protected:
        using DependenciesLoadedCallback = std::function<bool()>;

        void setLoadingError(std::string _loadingError);
        void setDependenciesLoadedCallback(DependenciesLoadedCallback&& _callback);

        virtual bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) = 0;
        virtual bool onDependenciesLoaded();
        virtual void onUnload();

    private:
        void clearDependencies();
        void addDependency(const ResourcePointer& _resource);
        void getDependencies(DependencyCollection& _dependencies) const;
        DependenciesLoadedCallback takeDependenciesLoadedCallback();

        void prepareLoading(const FileName& _path);
        void setState(ResourceState _state);
        void unload();

        mutable std::mutex m_mutex;

        FileName m_path;
        std::string m_loadingError;
        DependencyCollection m_dependencies;
        DependenciesLoadedCallback m_dependenciesLoadedCallback;
        std::atomic<ResourceState> m_state = ResourceState::Undefined;
    };

    template <typename TResource, typename... TArgs>
    SharedPointer<TResource> CreateResource(TArgs&&... _args)
    {
        static_assert(std::is_base_of_v<Resource, TResource>);
        return SharedPointer<TResource>(new TResource(std::forward<TArgs>(_args)...), ResourceDeleter{});
    }
} // namespace ego

#define EGO_RESOURCE(_RESOURCE, ...)                                                                                                                                               \
    EGO_RTTI_VIRTUAL(_RESOURCE, __VA_ARGS__)                                                                                                                                       \
                                                                                                                                                                                   \
    static const char* GetResourceTypeName()                                                                                                                                       \
    {                                                                                                                                                                              \
        return GetMetaInfoTypeName();                                                                                                                                              \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    static ego::ResourceType GetResourceType()                                                                                                                                     \
    {                                                                                                                                                                              \
        return GetMetaInfoID();                                                                                                                                                    \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    virtual ego::ResourceType getType() const override                                                                                                                             \
    {                                                                                                                                                                              \
        return GetResourceType();                                                                                                                                                  \
    }

#define EGO_RESOURCE_TYPE(_RESOURCE) (_RESOURCE::GetResourceType())
#define EGO_RESOURCE_TYPE_NAME(_RESOURCE) (_RESOURCE::GetResourceTypeName())
