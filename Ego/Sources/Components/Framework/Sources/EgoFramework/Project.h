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

        struct GameLogicPlugin final
        {
            std::string m_name;
            FileName m_moduleName;
        };

        using GameLogicPluginCollection = std::vector<GameLogicPlugin>;

        Project() = default;
        ~Project() = default;

        bool addAssetDirectory(const FileName& _directory);
        bool addPluginDirectory(const FileName& _directory);
        bool addGameLogicPlugin(const GameLogicPlugin& _plugin);
        bool addGameLogicPlugin(const char* _name, const FileName& _moduleName);

        void clear();

        const DirectoryCollection& getAssetDirectories() const;
        const DirectoryCollection& getPluginDirectories() const;
        const GameLogicPluginCollection& getGameLogicPlugins() const;

    private:
        DirectoryCollection m_assetDirectories;
        DirectoryCollection m_pluginDirectories;
        GameLogicPluginCollection m_gameLogicPlugins;
    };

    EGO_POINTER(Project);
}
