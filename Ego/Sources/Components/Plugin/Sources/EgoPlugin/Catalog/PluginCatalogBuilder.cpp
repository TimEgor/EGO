#include "PluginCatalogBuilder.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string_view>
#include <utility>

#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/Log/Log.h"
#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/String/Format.h"

#ifndef EGO_MODULE_PLATFORM_NAME
    #define EGO_MODULE_PLATFORM_NAME ""
#endif

#ifndef EGO_MODULE_CONFIGURATION_NAME
    #define EGO_MODULE_CONFIGURATION_NAME ""
#endif

bool ego::PluginCatalogBuilder::Report::hasErrors() const
{
    for (const Issue& issue : m_issues)
    {
        if (issue.m_severity == IssueSeverity::Error)
        {
            return true;
        }
    }

    return false;
}

void ego::PluginCatalogBuilder::LogIssues(const Report& _report)
{
    for (const Issue& issue : _report.m_issues)
    {
        if (issue.m_severity == IssueSeverity::Error)
        {
            EGO_LOG_ERROR(StringFormat("Plugin catalog '{}': {}", issue.m_path.c_str(), issue.m_message));
        }
        else
        {
            EGO_LOG_WARNING(StringFormat("Plugin catalog '{}': {}", issue.m_path.c_str(), issue.m_message));
        }
    }
}

ego::PluginCatalog::RegistrationID ego::PluginCatalogBuilder::AddPluginsFromPath(
    PluginCatalog& _catalog,
    const FileSystem& _fileSystem,
    const FileName& _path,
    bool _recursive)
{
    Options options;
    options.m_recursive = _recursive;
    return AddPluginsFromPath(_catalog, _fileSystem, _path, options);
}

ego::PluginCatalog::RegistrationID ego::PluginCatalogBuilder::AddPluginsFromPath(
    PluginCatalog& _catalog,
    const FileSystem& _fileSystem,
    const FileName& _path,
    const Options& _options)
{
    PluginCatalog::ModuleCollection modules;
    Report report = ReadPluginsFromPath(modules, _fileSystem, _path, _options);

    PluginCatalog::RegistrationID registrationID = PluginCatalog::InvalidRegistrationID;
    if (!report.hasErrors())
    {
        registrationID = CommitRegistration(_catalog, std::move(modules), report, _options, _path);
    }

    LogIssues(report);
    return registrationID;
}

ego::PluginCatalog::RegistrationID ego::PluginCatalogBuilder::AddPluginsFromManifest(
    PluginCatalog& _catalog,
    const FileSystem& _fileSystem,
    const FileName& _manifestPath)
{
    return AddPluginsFromManifest(_catalog, _fileSystem, _manifestPath, Options());
}

ego::PluginCatalog::RegistrationID ego::PluginCatalogBuilder::AddPluginsFromManifest(
    PluginCatalog& _catalog,
    const FileSystem& _fileSystem,
    const FileName& _manifestPath,
    const Options& _options)
{
    PluginCatalog::ModuleCollection modules;
    Report report = ReadPluginsFromManifest(modules, _fileSystem, _manifestPath, _options);

    PluginCatalog::RegistrationID registrationID = PluginCatalog::InvalidRegistrationID;
    if (!report.hasErrors())
    {
        registrationID = CommitRegistration(_catalog, std::move(modules), report, _options, _manifestPath);
    }

    LogIssues(report);
    return registrationID;
}

ego::PluginCatalogBuilder::Report ego::PluginCatalogBuilder::ReadPluginsFromPath(
    PluginCatalog::ModuleCollection& _modules,
    const FileSystem& _fileSystem,
    const FileName& _path,
    const Options& _options)
{
    Report report;
    if (!_path)
    {
        AddIssue(report, _options, IssueCode::InvalidPath, _path, "Plugin catalog path is empty.", false);
        return report;
    }

    if (_fileSystem.isFile(_path))
    {
        return ReadPluginsFromManifest(_modules, _fileSystem, _path, _options);
    }

    if (!_fileSystem.isDirectory(_path))
    {
        AddIssue(report, _options, IssueCode::PathNotFound, _path, "Plugin catalog path does not exist.", true);
        return report;
    }

    FileSystemEntryCollection entries;
    if (!_fileSystem.enumerate(_path, entries, _options.m_recursive))
    {
        AddIssue(report, _options, IssueCode::EnumerationFailed, _path, "Failed to enumerate plugin catalog directory.", true);
        return report;
    }

    std::sort(
        entries.begin(),
        entries.end(),
        [](const FileSystemEntryDesc& _left, const FileSystemEntryDesc& _right)
        {
            return _left.m_path.getView() < _right.m_path.getView();
        });

    for (const FileSystemEntryDesc& entry : entries)
    {
        if (entry.isFile() && IsPluginManifestPath(entry.m_path))
        {
            MergeReport(report, ReadPluginsFromManifest(_modules, _fileSystem, entry.m_path, _options));
        }
    }

    return report;
}

ego::PluginCatalogBuilder::Report ego::PluginCatalogBuilder::ReadPluginsFromManifest(
    PluginCatalog::ModuleCollection& _modules,
    const FileSystem& _fileSystem,
    const FileName& _manifestPath,
    const Options& _options)
{
    Report report;
    ++report.m_discoveredManifestCount;

    if (!IsPluginManifestPath(_manifestPath))
    {
        AddIssue(report, _options, IssueCode::InvalidManifestPath, _manifestPath, "Plugin manifest path must end with .plugin.xml.", true);
        return report;
    }

    std::string content;
    if (!_fileSystem.readTextFile(_manifestPath, content))
    {
        AddIssue(report, _options, IssueCode::ReadFailed, _manifestPath, "Failed to read plugin manifest.", true);
        return report;
    }

    const FileName moduleName = ResolveModuleName(_fileSystem, _manifestPath);
    if (!moduleName)
    {
        AddIssue(report, _options, IssueCode::InvalidManifestPath, _manifestPath, "Failed to resolve plugin module path from manifest name.", true);
        return report;
    }

    PluginManifestReader::Result readResult = PluginManifestReader::Read(content, moduleName, CreateManifestReaderContext());
    if (readResult.isSkipped())
    {
        ++report.m_skippedManifestCount;
        return report;
    }

    if (!readResult.isLoaded())
    {
        AddIssue(report, _options, IssueCode::InvalidManifest, _manifestPath, readResult.m_message, true);
        return report;
    }

    if (!_fileSystem.isFile(moduleName))
    {
        AddIssue(report, _options, IssueCode::ModuleNotFound, _manifestPath, "Plugin module referenced by manifest does not exist.", true);
        return report;
    }

    const size_t pluginCount = readResult.m_module.m_plugins.size();
    _modules.push_back(std::move(readResult.m_module));
    ++report.m_loadedManifestCount;
    report.m_pluginCount += pluginCount;
    return report;
}

ego::PluginCatalog::RegistrationID ego::PluginCatalogBuilder::CommitRegistration(
    PluginCatalog& _catalog,
    PluginCatalog::ModuleCollection&& _modules,
    Report& _report,
    const Options& _options,
    const FileName& _path)
{
    const PluginCatalog::RegistrationID registrationID = _catalog.registerModules(_modules);
    if (registrationID == PluginCatalog::InvalidRegistrationID)
    {
        AddIssue(_report, _options, IssueCode::InvalidCatalogEntry, _path, "Plugin manifests produced an invalid catalog registration.", false);
    }

    return registrationID;
}

bool ego::PluginCatalogBuilder::EqualsNoCase(const char* _left, const char* _right)
{
    if (!_left || !_right)
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

bool ego::PluginCatalogBuilder::IsPluginManifestPath(const FileName& _path)
{
    const FileName fileName = file_name_utils::GetFileName(_path);
    if (!EqualsNoCase(file_name_utils::GetFileExtension(fileName).c_str(), ".xml"))
    {
        return false;
    }

    return EqualsNoCase(file_name_utils::GetFileExtension(file_name_utils::RemoveExtension(fileName)).c_str(), ".plugin");
}

ego::FileName ego::PluginCatalogBuilder::GetManifestModuleFileName(const FileName& _manifestPath)
{
    if (!IsPluginManifestPath(_manifestPath))
    {
        return FileName();
    }

    return file_name_utils::RemoveExtension(file_name_utils::RemoveExtension(file_name_utils::GetFileName(_manifestPath)));
}

ego::FileName ego::PluginCatalogBuilder::CombinePath(const FileName& _directoryPath, const FileName& _fileName)
{
    if (!_directoryPath || !_fileName)
    {
        return _fileName;
    }

    FileName result = _directoryPath;
    const size_t length = result.length();
    if (length > 0 && result[length - 1] != '/' && result[length - 1] != '\\')
    {
        result += '/';
    }

    result += _fileName;
    return result;
}

ego::FileName ego::PluginCatalogBuilder::ResolveModuleName(const FileSystem& _fileSystem, const FileName& _manifestPath)
{
    const FileName moduleFileName = GetManifestModuleFileName(_manifestPath);
    if (!moduleFileName)
    {
        return FileName();
    }

    const FileName moduleName = CombinePath(file_name_utils::GetFileDirPath(_manifestPath), moduleFileName);
    const FileName absoluteName = _fileSystem.getAbsolutePath(moduleName);
    return absoluteName ? absoluteName : moduleName;
}

ego::PluginManifestReader::Context ego::PluginCatalogBuilder::CreateManifestReaderContext()
{
    PluginManifestReader::Context context;
    context.m_platform = EGO_MODULE_PLATFORM_NAME;
    context.m_configuration = EGO_MODULE_CONFIGURATION_NAME;
    return context;
}

void ego::PluginCatalogBuilder::AddIssue(
    Report& _report,
    const Options& _options,
    IssueCode _code,
    const FileName& _path,
    const std::string& _message,
    bool _recoverable)
{
    Issue issue;
    issue.m_severity = _recoverable && _options.m_mode == Mode::BestEffort ? IssueSeverity::Warning : IssueSeverity::Error;
    issue.m_code = _code;
    issue.m_path = _path;
    issue.m_message = _message;
    _report.m_issues.push_back(std::move(issue));
}

void ego::PluginCatalogBuilder::MergeReport(Report& _target, Report&& _source)
{
    _target.m_discoveredManifestCount += _source.m_discoveredManifestCount;
    _target.m_loadedManifestCount += _source.m_loadedManifestCount;
    _target.m_skippedManifestCount += _source.m_skippedManifestCount;
    _target.m_pluginCount += _source.m_pluginCount;

    for (Issue& issue : _source.m_issues)
    {
        _target.m_issues.push_back(std::move(issue));
    }
}
