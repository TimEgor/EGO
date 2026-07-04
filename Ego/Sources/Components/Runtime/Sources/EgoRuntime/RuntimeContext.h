#pragma once

#include <cstdint>

#include "EgoCore/Context/Context.h"

namespace ego
{
    class EventController;
    class FileSystem;
    class PluginCatalog;
    class ResourceController;

    EGO_POINTER(EventController);
    EGO_POINTER(FileSystem);
    EGO_POINTER(PluginCatalog);
    EGO_POINTER(ResourceController);
} // namespace ego

namespace ego::context
{
    class RuntimeContext final : public Context
    {
    public:
        struct InitData final
        {
            FileSystemPointer m_resourceFileSystem = nullptr;
            uint32_t m_resourceJobThreadCount = 0;
            const char* m_resourceJobThreadName = "EgoResourceJob";
        };

        RuntimeContext() = default;
        ~RuntimeContext() override = default;

        bool init(const InitData& _initData);
        void release();

        PluginCatalogPointer getPluginCatalogPointer() const;
        PluginCatalog& getPluginCatalog() const;

        EventControllerPointer getEventControllerPointer() const;
        EventController& getEventController() const;

        ResourceControllerPointer getResourceControllerPointer() const;
        ResourceController& getResourceController() const;

        EGO_RTTI_VIRTUAL(RuntimeContext, Context);

    private:
        bool initResourceController(const InitData& _initData);

        PluginCatalogPointer m_pluginCatalog = nullptr;
        EventControllerPointer m_eventController = nullptr;
        ResourceControllerPointer m_resourceController = nullptr;
    };

    EGO_POINTER(RuntimeContext);

    RuntimeContextPointer GetRuntimeContextPointer();
    RuntimeContext& GetRuntimeContext();
} // namespace ego::context
