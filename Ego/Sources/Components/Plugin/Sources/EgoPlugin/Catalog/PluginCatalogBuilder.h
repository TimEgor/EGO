#pragma once

#include <string>
#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonInstanceable.h"

#include "PluginCatalog.h"
#include "PluginManifestReader.h"

namespace ego
{
    class FileSystem;
} // namespace ego

namespace ego
{
    class PluginCatalogBuilder final : public NonInstanceable
    {
    public:
        enum class Mode
        {
            Strict,
            BestEffort
        };

        struct Options final
        {
            Mode m_mode = Mode::Strict;
            bool m_recursive = true;
        };

        static PluginCatalog::RegistrationID AddPluginsFromPath(PluginCatalog& _catalog, const FileSystem& _fileSystem, const FileName& _path, bool _recursive = true);
        static PluginCatalog::RegistrationID AddPluginsFromPath(PluginCatalog& _catalog, const FileSystem& _fileSystem, const FileName& _path, const Options& _options);
        static PluginCatalog::RegistrationID AddPluginsFromManifest(PluginCatalog& _catalog, const FileSystem& _fileSystem, const FileName& _manifestPath);
        static PluginCatalog::RegistrationID AddPluginsFromManifest(PluginCatalog& _catalog, const FileSystem& _fileSystem, const FileName& _manifestPath, const Options& _options);

    private:
        enum class IssueSeverity
        {
            Warning,
            Error
        };

        enum class IssueCode
        {
            InvalidPath,
            PathNotFound,
            InvalidManifestPath,
            EnumerationFailed,
            ReadFailed,
            ModuleNotFound,
            InvalidManifest,
            InvalidCatalogEntry
        };

        struct Issue final
        {
            IssueSeverity m_severity = IssueSeverity::Error;
            IssueCode m_code = IssueCode::InvalidPath;
            FileName m_path;
            std::string m_message;
        };

        using IssueCollection = std::vector<Issue>;

        struct Report final
        {
            size_t m_discoveredManifestCount = 0;
            size_t m_loadedManifestCount = 0;
            size_t m_skippedManifestCount = 0;
            size_t m_pluginCount = 0;
            IssueCollection m_issues;

            bool hasErrors() const;
        };

        static Report ReadPluginsFromPath(PluginCatalog::ModuleCollection& _modules, const FileSystem& _fileSystem, const FileName& _path, const Options& _options);
        static Report ReadPluginsFromManifest(PluginCatalog::ModuleCollection& _modules, const FileSystem& _fileSystem, const FileName& _manifestPath, const Options& _options);

        static PluginCatalog::RegistrationID CommitRegistration(
            PluginCatalog& _catalog,
            PluginCatalog::ModuleCollection&& _modules,
            Report& _report,
            const Options& _options,
            const FileName& _path);

        static bool EqualsNoCase(const char* _left, const char* _right);
        static bool IsPluginManifestPath(const FileName& _path);
        static FileName GetManifestModuleFileName(const FileName& _manifestPath);
        static FileName CombinePath(const FileName& _directoryPath, const FileName& _fileName);
        static FileName ResolveModuleName(const FileSystem& _fileSystem, const FileName& _manifestPath);
        static PluginManifestReader::Context CreateManifestReaderContext();

        static void LogIssues(const Report& _report);
        static void AddIssue(Report& _report, const Options& _options, IssueCode _code, const FileName& _path, const std::string& _message, bool _recoverable);
        static void MergeReport(Report& _target, Report&& _source);
    };
} // namespace ego
