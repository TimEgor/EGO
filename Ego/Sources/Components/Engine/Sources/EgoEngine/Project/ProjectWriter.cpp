#include "ProjectWriter.h"

#include "EgoCore/UtilsMacros.h"

#include <pugixml.hpp>

bool ego::engine::ProjectWriter::WriteToFile(const FileName& _fileName, const Project& _project)
{
    EGO_CHECK_RETURN_FALSE(_fileName);

    pugi::xml_document document;
    pugi::xml_node rootNode = document.append_child("Project");
    EGO_CHECK_RETURN_FALSE(rootNode && WriteRootNode(rootNode, _project));

    return document.save_file(_fileName.c_str(), "    ");
}

bool ego::engine::ProjectWriter::WriteRootNode(pugi::xml_node _rootNode, const Project& _project)
{
    EGO_CHECK_RETURN_FALSE(_rootNode);

    if (!_project.getName().empty())
    {
        pugi::xml_attribute nameAttribute = _rootNode.append_attribute("Name");
        EGO_CHECK_RETURN_FALSE(nameAttribute && nameAttribute.set_value(_project.getName().c_str()));
    }

    EGO_CHECK_RETURN_FALSE(WriteDirectoryCollection(_rootNode, "PluginDirectories", "PluginDirectory", _project.getPluginDirectories()));
    EGO_CHECK_RETURN_FALSE(WriteDirectoryCollection(_rootNode, "AssetDirectories", "AssetDirectory", _project.getAssetDirectories()));
    EGO_CHECK_RETURN_FALSE(WritePlugins(_rootNode, _project));
    EGO_CHECK_RETURN_FALSE(WriteEngineLogic(_rootNode, _project));

    return true;
}

bool ego::engine::ProjectWriter::WriteDirectoryCollection(
    pugi::xml_node _rootNode,
    const char* _collectionNodeName,
    const char* _directoryNodeName,
    const Project::DirectoryCollection& _directories)
{
    if (_directories.empty())
    {
        return true;
    }

    pugi::xml_node collectionNode = _rootNode.append_child(_collectionNodeName);
    EGO_CHECK_RETURN_FALSE(collectionNode);

    for (const FileName& directory : _directories)
    {
        pugi::xml_node directoryNode = collectionNode.append_child(_directoryNodeName);
        pugi::xml_attribute pathAttribute = directoryNode.append_attribute("Path");
        EGO_CHECK_RETURN_FALSE(directoryNode && pathAttribute && pathAttribute.set_value(directory.c_str()));
    }

    return true;
}

bool ego::engine::ProjectWriter::WritePlugins(pugi::xml_node _rootNode, const Project& _project)
{
    if (_project.getPlugins().empty())
    {
        return true;
    }

    pugi::xml_node pluginsNode = _rootNode.append_child("Plugins");
    EGO_CHECK_RETURN_FALSE(pluginsNode);

    for (const Project::PluginDesc& plugin : _project.getPlugins())
    {
        EGO_CHECK_RETURN_FALSE(WritePlugin(pluginsNode, plugin, true));
    }

    return true;
}

bool ego::engine::ProjectWriter::WritePlugin(pugi::xml_node _parentNode, const Project::PluginDesc& _plugin, bool _writeType)
{
    pugi::xml_node pluginNode = _parentNode.append_child("Plugin");
    EGO_CHECK_RETURN_FALSE(pluginNode);

    if (_writeType)
    {
        pugi::xml_attribute typeAttribute = pluginNode.append_attribute("Type");
        EGO_CHECK_RETURN_FALSE(typeAttribute && typeAttribute.set_value(_plugin.m_typeName.c_str()));
    }

    if (!_plugin.m_name.empty())
    {
        pugi::xml_attribute nameAttribute = pluginNode.append_attribute("Name");
        EGO_CHECK_RETURN_FALSE(nameAttribute && nameAttribute.set_value(_plugin.m_name.c_str()));
    }

    if (_plugin.m_moduleName)
    {
        pugi::xml_attribute moduleAttribute = pluginNode.append_attribute("Module");
        EGO_CHECK_RETURN_FALSE(moduleAttribute && moduleAttribute.set_value(_plugin.m_moduleName.c_str()));
    }

    return true;
}

bool ego::engine::ProjectWriter::WriteEngineLogic(pugi::xml_node _rootNode, const Project& _project)
{
    const Project::OptionalPluginDesc& engineLogicPlugin = _project.getEngineLogicPlugin();
    if (!engineLogicPlugin.has_value())
    {
        return true;
    }

    pugi::xml_node engineLogicNode = _rootNode.append_child("EngineLogic");
    EGO_CHECK_RETURN_FALSE(engineLogicNode);

    return WritePlugin(engineLogicNode, engineLogicPlugin.value(), false);
}
