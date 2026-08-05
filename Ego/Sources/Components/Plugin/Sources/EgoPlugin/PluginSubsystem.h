#pragma once

#include "EgoCore/Subsystem/Subsystem.h"

#include "Catalog/PluginCatalog.h"

namespace ego
{
    class PluginController;

    EGO_POINTER(PluginController);

    class PluginSubsystem final : public subsystem::Subsystem
    {
    public:
        PluginSubsystem() = default;
        ~PluginSubsystem() override;

        bool init();

        PluginCatalog& getPluginCatalog();
        const PluginCatalog& getPluginCatalog() const;
        PluginControllerPointer getPluginControllerPointer() const;

        EGO_SUBSYSTEM(PluginSubsystem, subsystem::Subsystem);

    private:
        void onUnregistered() override;
        void release();

        PluginCatalog m_pluginCatalog;
        PluginControllerPointer m_pluginController = nullptr;
    };

    EGO_POINTER(PluginSubsystem);

    PluginSubsystemPointer GetPluginSubsystemPointer();
    PluginSubsystem& GetPluginSubsystem();
} // namespace ego
