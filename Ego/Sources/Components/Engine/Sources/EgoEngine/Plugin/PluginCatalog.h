#pragma once

#include <string>
#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoPlugin/Plugin.h"

namespace ego::engine
{
    class PluginCatalog final
    {
    public:
        struct Plugin final
        {
            PluginType m_type = rtti::InvalidTypeMetaInfoID;
            std::string m_name;
        };

        using PluginCollection = std::vector<Plugin>;

        struct Entry final
        {
            FileName m_moduleName;
            PluginCollection m_plugins;
        };

        using EntryCollection = std::vector<Entry>;

        PluginCatalog() = default;

        bool add(const Entry& _entry);
        void clear();
        FileName getModulePath(PluginType _pluginType, const char* _pluginName) const;
        const EntryCollection& getEntries() const;

    private:
        static bool isPluginValid(const Plugin& _plugin);
        static bool isEntryValid(const Entry& _entry);

        Entry* findEntry(const FileName& _moduleName);
        Plugin* findPlugin(Entry& _entry, PluginType _pluginType, const char* _pluginName);
        const Plugin* findPlugin(const Entry& _entry, PluginType _pluginType, const char* _pluginName) const;

        EntryCollection m_entries;
    };

    EGO_POINTER(PluginCatalog);
} // namespace ego::engine
