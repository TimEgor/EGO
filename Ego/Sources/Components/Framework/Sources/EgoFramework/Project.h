#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoCore/FileName/FileName.h"

#include <string>
#include <vector>

namespace ego::framework
{
    class Project final : public NonCopyable
    {
    public:
        using DirectoryCollection = std::vector<FileName>;

        struct Plugin final
        {
            std::string m_type;
            std::string m_name;
            FileName m_moduleName;
        };

        struct GameLogicPlugin final
        {
            std::string m_name;
            FileName m_moduleName;
        };

        using PluginCollection = std::vector<Plugin>;
        using GameLogicPluginCollection = std::vector<GameLogicPlugin>;

        Project() = default;
        ~Project() = default;

        bool addAssetDirectory(const FileName& _directory);
        bool addPluginDirectory(const FileName& _directory);
        bool addPlugin(const Plugin& _plugin);
        bool addPlugin(const char* _type, const char* _name, const FileName& _moduleName);
        bool addGameLogicPlugin(const GameLogicPlugin& _plugin);
        bool addGameLogicPlugin(const char* _name, const FileName& _moduleName);

        void clear();

        const DirectoryCollection& getAssetDirectories() const;
        const DirectoryCollection& getPluginDirectories() const;
        const PluginCollection& getPlugins() const;
        const GameLogicPluginCollection& getGameLogicPlugins() const;

    private:
        DirectoryCollection m_assetDirectories;
        DirectoryCollection m_pluginDirectories;
        PluginCollection m_plugins;
        GameLogicPluginCollection m_gameLogicPlugins;
    };

    EGO_POINTER(Project);
}
