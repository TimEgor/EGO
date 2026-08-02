#include "Project.h"

bool ego::engine::Project::setName(const std::string& _name)
{
    if (_name.empty())
    {
        return false;
    }

    m_name = _name;

    return true;
}

bool ego::engine::Project::addAssetDirectory(const FileName& _directory)
{
    if (!_directory)
    {
        return false;
    }

    m_assetDirectories.push_back(_directory);
    return true;
}

bool ego::engine::Project::addPluginDirectory(const FileName& _directory)
{
    if (!_directory)
    {
        return false;
    }

    m_pluginDirectories.push_back(_directory);
    return true;
}

bool ego::engine::Project::addPlugin(const PluginDesc& _plugin)
{
    if (_plugin.m_typeName.empty() || (_plugin.m_name.empty() && !_plugin.m_moduleName))
    {
        return false;
    }

    m_plugins.push_back(_plugin);
    return true;
}

bool ego::engine::Project::setEngineLogicPlugin(const PluginDesc& _plugin)
{
    if (_plugin.m_typeName.empty() || (_plugin.m_name.empty() && !_plugin.m_moduleName))
    {
        return false;
    }

    m_engineLogicPlugin = _plugin;
    return true;
}

void ego::engine::Project::clear()
{
    m_name.clear();

    m_assetDirectories.clear();
    m_pluginDirectories.clear();
    m_plugins.clear();
    m_engineLogicPlugin.reset();
}

const std::string& ego::engine::Project::getName() const
{
    return m_name;
}

const ego::engine::Project::DirectoryCollection& ego::engine::Project::getAssetDirectories() const
{
    return m_assetDirectories;
}

const ego::engine::Project::DirectoryCollection& ego::engine::Project::getPluginDirectories() const
{
    return m_pluginDirectories;
}

const ego::engine::Project::PluginCollection& ego::engine::Project::getPlugins() const
{
    return m_plugins;
}

const ego::engine::Project::OptionalPluginDesc& ego::engine::Project::getEngineLogicPlugin() const
{
    return m_engineLogicPlugin;
}
