#include "PluginManifestReader.h"

#include <cctype>

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"

bool ego::PluginManifestReader::Result::isLoaded() const
{
    return m_status == Status::Loaded;
}

bool ego::PluginManifestReader::Result::isSkipped() const
{
    return m_status == Status::NotApplicable;
}

ego::PluginManifestReader::Result ego::PluginManifestReader::Read(const std::string& _content, const FileName& _moduleName, const Context& _context)
{
    Result result;
    if (_content.empty())
    {
        result.m_message = "Plugin manifest is empty.";
        return result;
    }

    if (!_moduleName)
    {
        result.m_message = "Plugin module path is empty.";
        return result;
    }

    XmlDocument document;
    if (!document.loadFromString(_content.c_str()))
    {
        result.m_message = "Plugin manifest XML is invalid.";
        return result;
    }

    return ReadNode(document.getRootNode(), _moduleName, _context);
}

ego::PluginManifestReader::Result ego::PluginManifestReader::ReadNode(const XmlNode& _manifestNode, const FileName& _moduleName, const Context& _context)
{
    Result result;
    if (!_manifestNode || _manifestNode.getNameView() != "PluginModule")
    {
        result.m_message = "Plugin manifest root must be PluginModule.";
        return result;
    }

    const std::string platform = _manifestNode.getAttributeOr<std::string>("Platform", "");
    const std::string configuration = _manifestNode.getAttributeOr<std::string>("Configuration", "");
    if (!EqualsNoCase(platform, _context.m_platform) || !EqualsNoCase(configuration, _context.m_configuration))
    {
        result.m_status = Status::NotApplicable;
        result.m_message = "Plugin manifest targets another platform or configuration.";
        return result;
    }

    result.m_module.m_moduleName = _moduleName;
    for (const XmlNode pluginNode : _manifestNode.getChildren("Plugin"))
    {
        const std::string typeName = pluginNode.getAttributeOr<std::string>("Type", "");
        const std::string pluginName = pluginNode.getAttributeOr<std::string>("Name", "");
        if (typeName.empty() || pluginName.empty())
        {
            result.m_message = "Plugin manifest entry must define non-empty Type and Name attributes.";
            result.m_module = PluginCatalog::ModuleDesc();
            return result;
        }

        const PluginType pluginType = GetPluginType(typeName.c_str());
        if (pluginType == rtti::InvalidTypeMetaInfoID || ContainsPlugin(result.m_module, pluginType, pluginName))
        {
            result.m_message = "Plugin manifest contains an invalid or duplicate plugin entry.";
            result.m_module = PluginCatalog::ModuleDesc();
            return result;
        }

        PluginCatalog::PluginDesc plugin;
        plugin.m_type = pluginType;
        plugin.m_name = pluginName;
        result.m_module.m_plugins.push_back(plugin);
    }

    if (result.m_module.m_plugins.empty())
    {
        result.m_message = "Plugin manifest does not contain any plugins.";
        result.m_module = PluginCatalog::ModuleDesc();
        return result;
    }

    result.m_status = Status::Loaded;
    result.m_message.clear();
    return result;
}

bool ego::PluginManifestReader::EqualsNoCase(const std::string& _left, const std::string& _right)
{
    if (_left.size() != _right.size())
    {
        return false;
    }

    for (size_t index = 0; index < _left.size(); ++index)
    {
        const char left = static_cast<char>(std::tolower(static_cast<unsigned char>(_left[index])));
        const char right = static_cast<char>(std::tolower(static_cast<unsigned char>(_right[index])));
        if (left != right)
        {
            return false;
        }
    }

    return true;
}

bool ego::PluginManifestReader::ContainsPlugin(const PluginCatalog::ModuleDesc& _module, PluginType _pluginType, const std::string& _pluginName)
{
    for (const PluginCatalog::PluginDesc& plugin : _module.m_plugins)
    {
        if (plugin.m_type == _pluginType && EqualsNoCase(plugin.m_name, _pluginName))
        {
            return true;
        }
    }

    return false;
}
