#include "Project.h"

bool ego::engine_framework::Project::addAssetDirectory(const FileName& _directory)
{
    if (!_directory)
    {
        return false;
    }

    m_assetDirectories.push_back(_directory);
    return true;
}

bool ego::engine_framework::Project::addPlugin(const PluginPointer& _plugin)
{
    if (!_plugin)
    {
        return false;
    }

    m_plugins.push_back(_plugin);
    return true;
}

bool ego::engine_framework::Project::setEngineLogicPlugin(const EngineLogicPluginPointer& _plugin)
{
    if (!_plugin)
    {
        return false;
    }

    m_engineLogicPlugin = _plugin;
    return true;
}

void ego::engine_framework::Project::clear()
{
    m_assetDirectories.clear();
    m_plugins.clear();
    m_engineLogicPlugin = nullptr;
}

const ego::engine_framework::Project::DirectoryCollection& ego::engine_framework::Project::getAssetDirectories() const
{
    return m_assetDirectories;
}

const ego::engine_framework::Project::PluginCollection& ego::engine_framework::Project::getPlugins() const
{
    return m_plugins;
}

ego::engine_framework::EngineLogicPluginPointer ego::engine_framework::Project::getEngineLogicPlugin() const
{
    return m_engineLogicPlugin;
}
