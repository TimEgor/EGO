#include "ProjectReader.h"

#include <string>

#include "EgoCore/Context/PlatformContext.h"
#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoRuntime/Plugin/PluginCatalog.h"
#include "EgoRuntime/Plugin/PluginCatalogBuilder.h"
#include "EgoRuntime/Plugin/PluginController.h"
#include "EgoRuntime/RuntimeContext.h"

bool ego::engine_framework::ProjectReader::ReadFromFile(const FileName& _fileName, Project& _project)
{
    EGO_CHECK_RETURN_FALSE(_fileName);

    XmlDocument document;
    EGO_CHECK_RETURN_FALSE(document.loadFromFile(_fileName));

    const XmlNode rootNode = document.getRootNode();
    return ReadFromRootNode(rootNode, _project);
}

bool ego::engine_framework::ProjectReader::ReadFromRootNode(const XmlNode& _rootNode, Project& _project)
{
    EGO_CHECK_RETURN_FALSE(_rootNode);
    EGO_CHECK_RETURN_FALSE(_rootNode.getNameView() == "Project");

    _project.clear();

    EGO_CHECK_RETURN_FALSE(ReadPluginDirectories(_rootNode));
    EGO_CHECK_RETURN_FALSE(ReadAssetDirectories(_rootNode, _project));
    EGO_CHECK_RETURN_FALSE(ReadPlugins(_rootNode, _project));
    EGO_CHECK_RETURN_FALSE(ReadEngineLogic(_rootNode, _project));
    EGO_CHECK_RETURN_FALSE(ReadEngineLogicPlugins(_rootNode, _project));

    return true;
}

bool ego::engine_framework::ProjectReader::ReadAssetDirectories(const XmlNode& _rootNode, Project& _project)
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

bool ego::engine_framework::ProjectReader::ReadPluginDirectories(const XmlNode& _rootNode)
{
    const XmlNode pluginDirectoriesNode = _rootNode.getChild("PluginDirectories");
    if (!pluginDirectoriesNode)
    {
        return true;
    }

    const FileSystemPointer fileSystem = context::GetPlatform().getFileSystem();
    EGO_CHECK_RETURN_FALSE(fileSystem);

    PluginCatalog& pluginCatalog = context::GetRuntimeContext().getPluginCatalog();
    for (const XmlNode pluginDirectoryNode : pluginDirectoriesNode.getChildren("PluginDirectory"))
    {
        const std::string path = pluginDirectoryNode.getAttributeOr<std::string>("Path", "");
        if (!path.empty())
        {
            PluginCatalogBuilder::AddPluginsFromPath(pluginCatalog, *fileSystem, path);
        }
    }

    return true;
}

bool ego::engine_framework::ProjectReader::ReadPlugins(const XmlNode& _rootNode, Project& _project)
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

bool ego::engine_framework::ProjectReader::ReadPluginNode(const XmlNode& _pluginNode, Project& _project)
{
    const std::string type = _pluginNode.getAttributeOr<std::string>("Type", "");
    const std::string name = _pluginNode.getAttributeOr<std::string>("Name", "");
    const std::string moduleName = _pluginNode.getAttributeOr<std::string>("Module", "");
    if (type.empty() || (name.empty() && moduleName.empty()))
    {
        return true;
    }

    const FileName pluginModuleName = ResolvePluginModuleName(type, name, moduleName.empty() ? FileName() : FileName(moduleName));
    EGO_CHECK_RETURN_FALSE(pluginModuleName);

    const PluginControllerPointer pluginController = GetCurrentPluginController();
    EGO_CHECK_RETURN_FALSE(pluginController);

    PluginPointer plugin = pluginController->loadPlugin(pluginModuleName, type.c_str());
    EGO_CHECK_RETURN_FALSE(plugin);

    return _project.addPlugin(plugin);
}

bool ego::engine_framework::ProjectReader::ReadEngineLogic(const XmlNode& _rootNode, Project& _project)
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

bool ego::engine_framework::ProjectReader::ReadEngineLogicPlugins(const XmlNode& _rootNode, Project& _project)
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

bool ego::engine_framework::ProjectReader::ReadEngineLogicPluginNode(const XmlNode& _pluginNode, Project& _project)
{
    if (_project.getEngineLogicPlugin())
    {
        return true;
    }

    const std::string name = _pluginNode.getAttributeOr<std::string>("Name", "");
    const std::string moduleName = _pluginNode.getAttributeOr<std::string>("Module", "");
    if (name.empty() && moduleName.empty())
    {
        return true;
    }

    const FileName pluginModuleName = ResolveEngineLogicPluginModuleName(name, moduleName.empty() ? FileName() : FileName(moduleName));
    if (!pluginModuleName)
    {
        return true;
    }

    const PluginControllerPointer pluginController = GetCurrentPluginController();
    EGO_CHECK_RETURN_FALSE(pluginController);

    EngineLogicPluginPointer plugin = pluginController->loadPlugin<EngineLogicPlugin>(pluginModuleName);
    EGO_CHECK_RETURN_FALSE(plugin);

    return _project.setEngineLogicPlugin(plugin);
}

ego::FileName ego::engine_framework::ProjectReader::ResolvePluginModuleName(
    const std::string& _type,
    const std::string& _name,
    const FileName& _moduleName)
{
    if (_moduleName)
    {
        return _moduleName;
    }

    if (!_type.empty() && !_name.empty())
    {
        return context::GetRuntimeContext().getPluginCatalog().getModulePath(GetPluginType(_type.c_str()), _name.c_str());
    }

    return FileName();
}

ego::FileName ego::engine_framework::ProjectReader::ResolveEngineLogicPluginModuleName(const std::string& _name, const FileName& _moduleName)
{
    if (_moduleName)
    {
        return _moduleName;
    }

    if (!_name.empty())
    {
        return context::GetRuntimeContext().getPluginCatalog().getModulePath(EngineLogicPlugin::GetPluginType(), _name.c_str());
    }

    return FileName();
}
