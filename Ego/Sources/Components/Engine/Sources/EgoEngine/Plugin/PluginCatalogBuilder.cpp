#include "PluginCatalogBuilder.h"

#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"

#include "EgoEngine/Platform/FileSystem/FileSystem.h"

#include <cctype>
#include <cstring>
#include <string>

#ifndef EGO_MODULE_PLATFORM_NAME
#define EGO_MODULE_PLATFORM_NAME ""
#endif

#ifndef EGO_MODULE_CONFIGURATION_NAME
#define EGO_MODULE_CONFIGURATION_NAME ""
#endif

namespace
{
    bool IsEmptyString(const char* _value)
    {
        return !_value || _value[0] == '\0';
    }

    bool IsPathSeparator(char _value)
    {
        return _value == '/' || _value == '\\';
    }

    bool EqualsNoCase(const char* _left, const char* _right)
    {
        if (IsEmptyString(_left) || IsEmptyString(_right))
        {
            return false;
        }

        const size_t leftLength = std::strlen(_left);
        const size_t rightLength = std::strlen(_right);
        if (leftLength != rightLength)
        {
            return false;
        }

        for (size_t index = 0; index < leftLength; ++index)
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

    bool IsPluginManifestPath(const ego::FileName& _path)
    {
        const ego::FileName fileName = ego::file_name_utils::GetFileName(_path);
        if (!EqualsNoCase(ego::file_name_utils::GetFileExtension(fileName).c_str(), ".xml"))
        {
            return false;
        }

        return EqualsNoCase(ego::file_name_utils::GetFileExtension(
            ego::file_name_utils::RemoveExtension(fileName)
        ).c_str(), ".plugin");
    }

    ego::FileName GetManifestModuleFileName(const ego::FileName& _manifestPath)
    {
        if (!IsPluginManifestPath(_manifestPath))
        {
            return ego::FileName();
        }

        return ego::file_name_utils::RemoveExtension(
            ego::file_name_utils::RemoveExtension(ego::file_name_utils::GetFileName(_manifestPath))
        );
    }

    ego::FileName CombinePath(const ego::FileName& _directoryPath, const ego::FileName& _fileName)
    {
        if (!_directoryPath || !_fileName)
        {
            return _fileName;
        }

        ego::FileName result = _directoryPath;
        const size_t length = result.length();
        if (length > 0 && !IsPathSeparator(result[length - 1]))
        {
            result += '/';
        }

        result += _fileName;
        return result;
    }

    ego::FileName ResolveModuleName(const ego::FileSystem& _fileSystem, const ego::FileName& _manifestPath)
    {
        const ego::FileName moduleFileName = GetManifestModuleFileName(_manifestPath);
        if (!moduleFileName)
        {
            return ego::FileName();
        }

        const ego::FileName moduleName = CombinePath(ego::file_name_utils::GetFileDirPath(_manifestPath), moduleFileName);
        const ego::FileName absoluteName = _fileSystem.getAbsolutePath(moduleName);
        return absoluteName ? absoluteName : moduleName;
    }

    bool IsManifestForCurrentModule(const ego::XmlNode& _manifestNode)
    {
        return EqualsNoCase(_manifestNode.getAttributeValue("Platform"), EGO_MODULE_PLATFORM_NAME) &&
            EqualsNoCase(_manifestNode.getAttributeValue("Configuration"), EGO_MODULE_CONFIGURATION_NAME);
    }
}

size_t ego::engine::PluginCatalogBuilder::AddPluginsFromPath(
    PluginCatalog& _catalog,
    const FileSystem& _fileSystem,
    const FileName& _path,
    bool _recursive
)
{
    if (!_path)
    {
        return 0;
    }

    if (_fileSystem.isFile(_path))
    {
        return AddPluginsFromManifest(_catalog, _fileSystem, _path);
    }

    if (!_fileSystem.isDirectory(_path))
    {
        return 0;
    }

    FileSystemEntryCollection entries;
    if (!_fileSystem.enumerate(_path, entries, _recursive))
    {
        return 0;
    }

    size_t addedCount = 0;
    for (const FileSystemEntryDesc& entry : entries)
    {
        if (entry.isFile() && IsPluginManifestPath(entry.m_path))
        {
            addedCount += AddPluginsFromManifest(_catalog, _fileSystem, entry.m_path);
        }
    }

    return addedCount;
}

size_t ego::engine::PluginCatalogBuilder::AddPluginsFromManifest(
    PluginCatalog& _catalog,
    const FileSystem& _fileSystem,
    const FileName& _manifestPath
)
{
    std::string content;
    if (!_fileSystem.readTextFile(_manifestPath, content))
    {
        return 0;
    }

    XmlDocument document;
    if (!document.loadFromString(content.c_str()))
    {
        return 0;
    }

    return AddPluginsFromManifestNode(
        _catalog,
        document.getRootNode().getFirstChild(),
        ResolveModuleName(_fileSystem, _manifestPath)
    );
}

size_t ego::engine::PluginCatalogBuilder::AddPluginsFromManifestNode(
    PluginCatalog& _catalog,
    const XmlNode& _manifestNode,
    const FileName& _moduleName
)
{
    if (!_manifestNode || !_moduleName)
    {
        return 0;
    }

    if (std::strcmp(_manifestNode.getName(), "PluginModule") != 0 || !IsManifestForCurrentModule(_manifestNode))
    {
        return 0;
    }

    PluginCatalog::Entry entry;
    entry.m_moduleName = _moduleName;

    for (const XmlNode pluginNode : _manifestNode.getChildren("Plugin"))
    {
        AddPluginFromNode(entry, pluginNode);
    }

    const size_t addedCount = entry.m_plugins.size();
    return _catalog.add(entry) ? addedCount : 0;
}

bool ego::engine::PluginCatalogBuilder::AddPluginFromNode(
    PluginCatalog::Entry& _entry,
    const XmlNode& _pluginNode
)
{
    const char* typeName = _pluginNode.getAttributeValue("Type");
    const char* name = _pluginNode.getAttributeValue("Name");
    if (IsEmptyString(typeName) || IsEmptyString(name))
    {
        return false;
    }

    PluginCatalog::Plugin plugin;
    plugin.m_type = ego::GetPluginType(typeName);
    plugin.m_name = name;

    _entry.m_plugins.push_back(plugin);
    return true;
}
