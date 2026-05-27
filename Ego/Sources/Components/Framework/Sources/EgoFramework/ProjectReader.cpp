#include "ProjectReader.h"

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/UtilsMacros.h"

#include <cstring>

namespace
{
    bool IsEmptyString(const char* _value)
    {
        return !_value || _value[0] == '\0';
    }

    void ReadGameLogicPluginNode(const ego::XmlNode& _pluginNode, ego::framework::Project& _project)
    {
        const char* name = _pluginNode.getAttributeValue("Name");
        const char* moduleName = _pluginNode.getAttributeValue("Module");
        if (!IsEmptyString(name) || !IsEmptyString(moduleName))
        {
            _project.addGameLogicPlugin(
                name,
                IsEmptyString(moduleName) ? ego::FileName() : ego::FileName(moduleName)
            );
        }
    }
}

bool ego::framework::ProjectReader::ReadFromFile(const FileName& _fileName, Project& _project)
{
    EGO_CHECK_RETURN_FALSE(_fileName);

    XmlDocument document;
    EGO_CHECK_RETURN_FALSE(document.loadFromFile(_fileName));

    const XmlNode rootNode = document.getRootNode().getFirstChild();
    return ReadFromRootNode(rootNode, _project);
}

bool ego::framework::ProjectReader::ReadFromRootNode(const XmlNode& _rootNode, Project& _project)
{
    EGO_CHECK_RETURN_FALSE(_rootNode);
    EGO_CHECK_RETURN_FALSE(std::strcmp(_rootNode.getName(), "Project") == 0);

    _project.clear();

    ReadAssetDirectories(_rootNode, _project);
    ReadPluginDirectories(_rootNode, _project);
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
        const char* path = assetDirectoryNode.getAttributeValue("Path");
        if (!IsEmptyString(path))
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
        const char* path = pluginDirectoryNode.getAttributeValue("Path");
        if (!IsEmptyString(path))
        {
            _project.addPluginDirectory(path);
        }
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
