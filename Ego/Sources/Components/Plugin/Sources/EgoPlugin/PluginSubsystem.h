#pragma once

#include "EgoCore/Subsystem/Subsystem.h"

namespace ego
{
    class PluginCatalog;

    EGO_POINTER(PluginCatalog);

    class PluginSubsystem final : public subsystem::Subsystem
    {
    public:
        PluginSubsystem() = default;
        ~PluginSubsystem() override = default;

        bool init();
        void release() override;

        PluginCatalogPointer getPluginCatalogPointer() const;
        PluginCatalog& getPluginCatalog() const;

        EGO_SUBSYSTEM(PluginSubsystem, subsystem::Subsystem);

    private:
        PluginCatalogPointer m_pluginCatalog = nullptr;
    };

    EGO_POINTER(PluginSubsystem);

    PluginSubsystemPointer GetPluginSubsystemPointer();
    PluginSubsystem& GetPluginSubsystem();
} // namespace ego
