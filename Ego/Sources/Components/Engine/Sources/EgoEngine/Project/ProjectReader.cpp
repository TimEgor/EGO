#include "ProjectReader.h"

#include <string>

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/UtilsMacros.h"

bool ego::engine::ProjectReader::ReadFromFile(const FileName& _fileName, Project& _project)
{
    EGO_CHECK_RETURN_FALSE(_fileName);

    XmlDocument document;
    EGO_CHECK_RETURN_FALSE(document.loadFromFile(_fileName));

    const XmlNode rootNode = document.getRootNode();
    return ReadFromRootNode(rootNode, _project);
}

bool ego::engine::ProjectReader::ReadFromRootNode(const XmlNode& _rootNode, Project& _project)
{
    EGO_CHECK_RETURN_FALSE(_rootNode);
    EGO_CHECK_RETURN_FALSE(_rootNode.getNameView() == "Project");

    _project.clear();

    EGO_CHECK_RETURN_FALSE(ReadName(_rootNode, _project));
    EGO_CHECK_RETURN_FALSE(ReadPluginDirectories(_rootNode, _project));
    EGO_CHECK_RETURN_FALSE(ReadAssetDirectories(_rootNode, _project));
    EGO_CHECK_RETURN_FALSE(ReadPlugins(_rootNode, _project));
    EGO_CHECK_RETURN_FALSE(ReadEngineLogic(_rootNode, _project));
    EGO_CHECK_RETURN_FALSE(ReadEngineLogicPlugins(_rootNode, _project));

    return true;
}

bool ego::engine::ProjectReader::ReadName(const XmlNode& _rootNode, Project& _project)
{
    const std::string name = _rootNode.getAttributeOr<std::string>("Name", "");

    return name.empty() || _project.setName(name);
}

bool ego::engine::ProjectReader::ReadAssetDirectories(const XmlNode& _rootNode, Project& _project)
{
    const XmlNode assetDirectoriesNode = _rootNode.getChild("AssetDirectories");
    if (!assetDirectoriesNode)
    {
        return true;
    }

    for (const XmlNode assetDirectoryNode : assetDirectoriesNode.getChildren("AssetDirectory"))
    {
        const std::string path = assetDirectoryNode.getAttributeOr<std::string>("Path", "");
        if (!path.empty())
        {
            EGO_CHECK_RETURN_FALSE(_project.addAssetDirectory(path));
        }
    }

    return true;
}

bool ego::engine::ProjectReader::ReadPluginDirectories(const XmlNode& _rootNode, Project& _project)
{
    const XmlNode pluginDirectoriesNode = _rootNode.getChild("PluginDirectories");
    if (!pluginDirectoriesNode)
    {
        return true;
    }

    for (const XmlNode pluginDirectoryNode : pluginDirectoriesNode.getChildren("PluginDirectory"))
    {
        const std::string path = pluginDirectoryNode.getAttributeOr<std::string>("Path", "");
        if (!path.empty())
        {
            EGO_CHECK_RETURN_FALSE(_project.addPluginDirectory(path));
        }
    }

    return true;
}

bool ego::engine::ProjectReader::ReadPlugins(const XmlNode& _rootNode, Project& _project)
{
    const XmlNode pluginsNode = _rootNode.getChild("Plugins");
    if (!pluginsNode)
    {
        return true;
    }

    for (const XmlNode pluginNode : pluginsNode.getChildren("Plugin"))
    {
        EGO_CHECK_RETURN_FALSE(ReadPluginNode(pluginNode, _project));
    }

    return true;
}

bool ego::engine::ProjectReader::ReadPluginNode(const XmlNode& _pluginNode, Project& _project)
{
    const std::string type = _pluginNode.getAttributeOr<std::string>("Type", "");
    const std::string name = _pluginNode.getAttributeOr<std::string>("Name", "");
    const std::string moduleName = _pluginNode.getAttributeOr<std::string>("Module", "");
    if (type.empty() || (name.empty() && moduleName.empty()))
    {
        return true;
    }

    Project::PluginDesc plugin;
    plugin.m_typeName = type;
    plugin.m_name = name;
    plugin.m_moduleName = moduleName.empty() ? FileName() : FileName(moduleName);
    return _project.addPlugin(plugin);
}

bool ego::engine::ProjectReader::ReadEngineLogic(const XmlNode& _rootNode, Project& _project)
{
    const XmlNode engineLogicNode = _rootNode.getChild("EngineLogic");
    if (!engineLogicNode)
    {
        return true;
    }

    for (const XmlNode pluginNode : engineLogicNode.getChildren("Plugin"))
    {
        EGO_CHECK_RETURN_FALSE(ReadEngineLogicPluginNode(pluginNode, _project));
    }

    return true;
}

bool ego::engine::ProjectReader::ReadEngineLogicPlugins(const XmlNode& _rootNode, Project& _project)
{
    const XmlNode engineLogicPluginsNode = _rootNode.getChild("EngineLogicPlugins");
    if (!engineLogicPluginsNode)
    {
        return true;
    }

    for (const XmlNode engineLogicPluginNode : engineLogicPluginsNode.getChildren("EngineLogicPlugin"))
    {
        EGO_CHECK_RETURN_FALSE(ReadEngineLogicPluginNode(engineLogicPluginNode, _project));
    }

    return true;
}

bool ego::engine::ProjectReader::ReadEngineLogicPluginNode(const XmlNode& _pluginNode, Project& _project)
{
    if (_project.getEngineLogicPlugin().has_value())
    {
        return true;
    }

    const std::string name = _pluginNode.getAttributeOr<std::string>("Name", "");
    const std::string moduleName = _pluginNode.getAttributeOr<std::string>("Module", "");
    if (name.empty() && moduleName.empty())
    {
        return true;
    }

    Project::PluginDesc plugin;
    plugin.m_typeName = "EngineLogicPlugin";
    plugin.m_name = name;
    plugin.m_moduleName = moduleName.empty() ? FileName() : FileName(moduleName);
    return _project.setEngineLogicPlugin(plugin);
}
