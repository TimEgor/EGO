#include "PluginCatalog.h"

#include <cctype>
#include <cstring>

namespace
{
    bool IsEmptyString(const char* _value)
    {
        return !_value || _value[0] == '\0';
    }

    bool EqualsNoCase(const std::string& _left, const char* _right)
    {
        if (!_right)
        {
            return _left.empty();
        }

        const size_t rightLength = std::strlen(_right);
        if (_left.size() != rightLength)
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
} // namespace

bool ego::PluginCatalog::add(const Entry& _entry)
{
    if (!isEntryValid(_entry))
    {
        return false;
    }

    Entry* entry = findEntry(_entry.m_moduleName);
    if (!entry)
    {
        m_entries.push_back(_entry);
        return true;
    }

    for (const Plugin& plugin : _entry.m_plugins)
    {
        Plugin* existingPlugin = findPlugin(*entry, plugin.m_type, plugin.m_name.c_str());
        if (existingPlugin)
        {
            *existingPlugin = plugin;
        }
        else
        {
            entry->m_plugins.push_back(plugin);
        }
    }

    return true;
}

void ego::PluginCatalog::clear()
{
    m_entries.clear();
}

ego::FileName ego::PluginCatalog::getModulePath(PluginType _pluginType) const
{
    if (_pluginType == rtti::InvalidTypeMetaInfoID)
    {
        return FileName();
    }

    for (const Entry& entry : m_entries)
    {
        for (const Plugin& plugin : entry.m_plugins)
        {
            if (plugin.m_type == _pluginType)
            {
                return entry.m_moduleName;
            }
        }
    }

    return FileName();
}

ego::FileName ego::PluginCatalog::getModulePath(PluginType _pluginType, const char* _pluginName) const
{
    if (_pluginType == rtti::InvalidTypeMetaInfoID || IsEmptyString(_pluginName))
    {
        return FileName();
    }

    for (const Entry& entry : m_entries)
    {
        if (findPlugin(entry, _pluginType, _pluginName))
        {
            return entry.m_moduleName;
        }
    }

    return FileName();
}

const ego::PluginCatalog::EntryCollection& ego::PluginCatalog::getEntries() const
{
    return m_entries;
}

bool ego::PluginCatalog::isPluginValid(const Plugin& _plugin)
{
    return _plugin.m_type != rtti::InvalidTypeMetaInfoID && !_plugin.m_name.empty();
}

bool ego::PluginCatalog::isEntryValid(const Entry& _entry)
{
    if (!_entry.m_moduleName || _entry.m_plugins.empty())
    {
        return false;
    }

    for (const Plugin& plugin : _entry.m_plugins)
    {
        if (!isPluginValid(plugin))
        {
            return false;
        }
    }

    return true;
}

ego::PluginCatalog::Entry* ego::PluginCatalog::findEntry(const FileName& _moduleName)
{
    for (Entry& entry : m_entries)
    {
        if (entry.m_moduleName == _moduleName)
        {
            return &entry;
        }
    }

    return nullptr;
}

ego::PluginCatalog::Plugin* ego::PluginCatalog::findPlugin(Entry& _entry, PluginType _pluginType, const char* _pluginName)
{
    for (Plugin& plugin : _entry.m_plugins)
    {
        if (plugin.m_type == _pluginType && EqualsNoCase(plugin.m_name, _pluginName))
        {
            return &plugin;
        }
    }

    return nullptr;
}

const ego::PluginCatalog::Plugin* ego::PluginCatalog::findPlugin(const Entry& _entry, PluginType _pluginType, const char* _pluginName) const
{
    for (const Plugin& plugin : _entry.m_plugins)
    {
        if (plugin.m_type == _pluginType && EqualsNoCase(plugin.m_name, _pluginName))
        {
            return &plugin;
        }
    }

    return nullptr;
}
