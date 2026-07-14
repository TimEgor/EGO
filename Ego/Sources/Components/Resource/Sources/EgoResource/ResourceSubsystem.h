#pragma once

#include <cstdint>

#include "EgoCore/Subsystem/Subsystem.h"

#include "ResourceProviderPluginController.h"

namespace ego
{
    class FileSystem;
    class ResourceController;

    EGO_POINTER(FileSystem);
    EGO_POINTER(ResourceController);

    class ResourceSubsystem final : public subsystem::Subsystem
    {
    public:
        struct InitData final
        {
            FileSystemPointer m_resourceFileSystem = nullptr;
            uint32_t m_resourceJobThreadCount = 0;
            const char* m_resourceJobThreadName = "EgoResourceJob";
        };

        ResourceSubsystem() = default;
        ~ResourceSubsystem() override = default;

        bool init(const InitData& _initData);
        void release() override;

        ResourceControllerPointer getResourceControllerPointer() const;
        ResourceController& getResourceController() const;
        ResourceProviderPluginController& getResourceProviderPluginController();

        EGO_SUBSYSTEM(ResourceSubsystem, subsystem::Subsystem);

    private:
        ResourceProviderPluginController m_resourceProviderPluginController;
        ResourceControllerPointer m_resourceController = nullptr;
    };

    EGO_POINTER(ResourceSubsystem);

    ResourceSubsystemPointer GetResourceSubsystemPointer();
    ResourceSubsystem& GetResourceSubsystem();
} // namespace ego
