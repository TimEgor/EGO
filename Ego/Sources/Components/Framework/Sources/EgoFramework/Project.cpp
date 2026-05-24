#include "Project.h"

bool ego::framework::Project::addAssetDirectory(const FileName& _directory)
{
    if (!_directory)
    {
        return false;
    }

    m_assetDirectories.push_back(_directory);
    return true;
}

bool ego::framework::Project::addPluginDirectory(const FileName& _directory)
{
    if (!_directory)
    {
        return false;
    }

    m_pluginDirectories.push_back(_directory);
    return true;
}

bool ego::framework::Project::addGameLogicPlugin(const GameLogicPlugin& _plugin)
{
    if (!_plugin.m_moduleName)
    {
        return false;
    }

    m_gameLogicPlugins.push_back(_plugin);
    return true;
}

bool ego::framework::Project::addGameLogicPlugin(const char* _name, const FileName& _moduleName)
{
    GameLogicPlugin plugin;
    plugin.m_name = _name ? _name : "";
    plugin.m_moduleName = _moduleName;
    return addGameLogicPlugin(plugin);
}

void ego::framework::Project::clear()
{
    m_assetDirectories.clear();
    m_pluginDirectories.clear();
    m_gameLogicPlugins.clear();
}

const ego::framework::Project::DirectoryCollection& ego::framework::Project::getAssetDirectories() const
{
    return m_assetDirectories;
}

const ego::framework::Project::DirectoryCollection& ego::framework::Project::getPluginDirectories() const
{
    return m_pluginDirectories;
}

const ego::framework::Project::GameLogicPluginCollection& ego::framework::Project::getGameLogicPlugins() const
{
    return m_gameLogicPlugins;
}
