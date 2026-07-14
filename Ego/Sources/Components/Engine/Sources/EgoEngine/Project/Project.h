#pragma once

#include <optional>
#include <string>
#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

namespace ego::engine
{
    class Project final : public NonCopyable
    {
    public:
        struct PluginDesc final
        {
            std::string m_typeName;
            std::string m_name;
            FileName m_moduleName;
        };

        using DirectoryCollection = std::vector<FileName>;
        using PluginCollection = std::vector<PluginDesc>;
        using OptionalPluginDesc = std::optional<PluginDesc>;

        Project() = default;
        ~Project() override = default;

        bool addAssetDirectory(const FileName& _directory);
        bool addPluginDirectory(const FileName& _directory);
        bool addPlugin(const PluginDesc& _plugin);
        bool setEngineLogicPlugin(const PluginDesc& _plugin);

        void clear();

        const DirectoryCollection& getAssetDirectories() const;
        const DirectoryCollection& getPluginDirectories() const;
        const PluginCollection& getPlugins() const;
        const OptionalPluginDesc& getEngineLogicPlugin() const;

    private:
        DirectoryCollection m_assetDirectories;
        DirectoryCollection m_pluginDirectories;
        PluginCollection m_plugins;
        OptionalPluginDesc m_engineLogicPlugin;
    };

    EGO_POINTER(Project);
} // namespace ego::engine
