#include <string>

#include "ProjectReader.h"

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/UtilsMacros.h"

namespace
{
    void ReadGameLogicPluginNode(const ego::XmlNode& _pluginNode, ego::framework::Project& _project)
    {
        const std::string name = _pluginNode.getAttributeOr<std::string>("Name", "");
        const std::string moduleName = _pluginNode.getAttributeOr<std::string>("Module", "");
        if (!name.empty() || !moduleName.empty())
        {
            _project.addGameLogicPlugin(name.c_str(), moduleName.empty() ? ego::FileName() : ego::FileName(moduleName));
        }
    }

    void ReadPluginNode(const ego::XmlNode& _pluginNode, ego::framework::Project& _project)
    {
        const std::string type = _pluginNode.getAttributeOr<std::string>("Type", "");
        const std::string name = _pluginNode.getAttributeOr<std::string>("Name", "");
        const std::string moduleName = _pluginNode.getAttributeOr<std::string>("Module", "");
        if (!type.empty() && (!name.empty() || !moduleName.empty()))
        {
            _project.addPlugin(type.c_str(), name.c_str(), moduleName.empty() ? ego::FileName() : ego::FileName(moduleName));
        }
    }
} // namespace

bool ego::framework::ProjectReader::ReadFromFile(const FileName& _fileName, Project& _project)
{
    EGO_CHECK_RETURN_FALSE(_fileName);

    XmlDocument document;
    EGO_CHECK_RETURN_FALSE(document.loadFromFile(_fileName));

    const XmlNode rootNode = document.getRootNode();
    return ReadFromRootNode(rootNode, _project);
}

bool ego::framework::ProjectReader::ReadFromRootNode(const XmlNode& _rootNode, Project& _project)
{
    EGO_CHECK_RETURN_FALSE(_rootNode);
    EGO_CHECK_RETURN_FALSE(_rootNode.getNameView() == "Project");

    _project.clear();

    ReadAssetDirectories(_rootNode, _project);
    ReadPluginDirectories(_rootNode, _project);
    ReadPlugins(_rootNode, _project);
    ReadGameLogic(_rootNode, _project);
    ReadGameLogicPlugins(_rootNode, _project);

    return true;
}

void ego::framework::ProjectReader::ReadAssetDirectories(const XmlNode& _rootNode, Project& _project)
{
    const XmlNode assetDirectoriesNode = _rootNode.getChild("AssetDirectories");
    if (!assetDirectoriesNode)
    {
        return;
    }

    for (const XmlNode assetDirectoryNode : assetDirectoriesNode.getChildren("AssetDirectory"))
    {
        const std::string path = assetDirectoryNode.getAttributeOr<std::string>("Path", "");
        if (!path.empty())
        {
            _project.addAssetDirectory(path);
        }
    }
}

void ego::framework::ProjectReader::ReadPluginDirectories(const XmlNode& _rootNode, Project& _project)
{
    const XmlNode pluginDirectoriesNode = _rootNode.getChild("PluginDirectories");
    if (!pluginDirectoriesNode)
    {
        return;
    }

    for (const XmlNode pluginDirectoryNode : pluginDirectoriesNode.getChildren("PluginDirectory"))
    {
        const std::string path = pluginDirectoryNode.getAttributeOr<std::string>("Path", "");
        if (!path.empty())
        {
            _project.addPluginDirectory(path);
        }
    }
}

void ego::framework::ProjectReader::ReadPlugins(const XmlNode& _rootNode, Project& _project)
{
    const XmlNode pluginsNode = _rootNode.getChild("Plugins");
    if (!pluginsNode)
    {
        return;
    }

    for (const XmlNode pluginNode : pluginsNode.getChildren("Plugin"))
    {
        ReadPluginNode(pluginNode, _project);
    }
}

void ego::framework::ProjectReader::ReadGameLogic(const XmlNode& _rootNode, Project& _project)
{
    const XmlNode gameLogicNode = _rootNode.getChild("GameLogic");
    if (!gameLogicNode)
    {
        return;
    }

    for (const XmlNode pluginNode : gameLogicNode.getChildren("Plugin"))
    {
        ReadGameLogicPluginNode(pluginNode, _project);
    }
}

void ego::framework::ProjectReader::ReadGameLogicPlugins(const XmlNode& _rootNode, Project& _project)
{
    const XmlNode gameLogicPluginsNode = _rootNode.getChild("GameLogicPlugins");
    if (!gameLogicPluginsNode)
    {
        return;
    }

    for (const XmlNode gameLogicPluginNode : gameLogicPluginsNode.getChildren("GameLogicPlugin"))
    {
        ReadGameLogicPluginNode(gameLogicPluginNode, _project);
    }
}
