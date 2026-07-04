#pragma once

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonInstanceable.h"

#include "PluginCatalog.h"

namespace ego
{
    class FileSystem;
    class XmlNode;
} // namespace ego

namespace ego
{
    class PluginCatalogBuilder final : public NonInstanceable
    {
    public:
        static size_t AddPluginsFromPath(PluginCatalog& _catalog, const FileSystem& _fileSystem, const FileName& _path, bool _recursive = true);
        static size_t AddPluginsFromManifest(PluginCatalog& _catalog, const FileSystem& _fileSystem, const FileName& _manifestPath);

    private:
        static size_t AddPluginsFromManifestNode(PluginCatalog& _catalog, const XmlNode& _manifestNode, const FileName& _moduleName);
        static bool AddPluginFromNode(PluginCatalog::Entry& _entry, const XmlNode& _pluginNode);
    };
} // namespace ego
