#pragma once

#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoRuntime/Plugin/Plugin.h"

#include "Plugin/EngineLogicPlugin.h"

namespace ego::engine_framework
{
    class Project final : public NonCopyable
    {
    public:
        using DirectoryCollection = std::vector<FileName>;
        using PluginCollection = std::vector<PluginPointer>;

        Project() = default;
        ~Project() override = default;

        bool addAssetDirectory(const FileName& _directory);
        bool addPlugin(const PluginPointer& _plugin);
        bool setEngineLogicPlugin(const EngineLogicPluginPointer& _plugin);

        void clear();

        const DirectoryCollection& getAssetDirectories() const;
        const PluginCollection& getPlugins() const;
        EngineLogicPluginPointer getEngineLogicPlugin() const;

    private:
        DirectoryCollection m_assetDirectories;
        PluginCollection m_plugins;
        EngineLogicPluginPointer m_engineLogicPlugin = nullptr;
    };

    EGO_POINTER(Project);
} // namespace ego::engine_framework
