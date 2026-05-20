#pragma once

#include "FileSystem.h"

#include <string>

namespace ego
{
    class RootedFileSystem final : public FileSystem
    {
    public:
        RootedFileSystem() = default;
        RootedFileSystem(const FileSystemPointer& _fileSystem, const FileName& _rootPath);
        ~RootedFileSystem() override;

        bool setSourceFileSystem(const FileSystemPointer& _fileSystem, const FileName& _rootPath);

        FileSystemPointer getSourceFileSystem() const;
        const FileName& getRootPath() const;

        virtual bool init() override;
        virtual void release() override;

        virtual bool exists(const FileName& _path) const override;
        virtual bool isFile(const FileName& _path) const override;
        virtual bool isDirectory(const FileName& _path) const override;

        virtual FileName getWorkingDirectory() const override;
        virtual bool setWorkingDirectory(const FileName& _path) override;
        virtual FileName getAbsolutePath(const FileName& _path) const override;

        virtual bool getEntryInfo(const FileName& _path, FileSystemEntryDesc& _entry) const override;
        virtual bool enumerate(
            const FileName& _directoryPath,
            FileSystemEntryCollection& _entries,
            bool _recursive = false
        ) const override;

        virtual bool createDirectory(const FileName& _path, bool _recursive = true) override;
        virtual bool removeFile(const FileName& _path) override;
        virtual bool removeDirectory(const FileName& _path, bool _recursive = false) override;
        virtual bool remove(const FileName& _path, bool _recursive = false) override;
        virtual bool copyFile(
            const FileName& _sourcePath,
            const FileName& _destinationPath,
            bool _overwrite = true
        ) override;
        virtual bool move(
            const FileName& _sourcePath,
            const FileName& _destinationPath,
            bool _overwrite = true
        ) override;

        virtual bool readFile(const FileName& _path, FileContent& _content) const override;
        virtual bool readTextFile(const FileName& _path, std::string& _content) const override;
        virtual bool writeFile(const FileName& _path, const FileContent& _content, bool _overwrite = true) override;
        virtual bool writeTextFile(const FileName& _path, const std::string& _content, bool _overwrite = true) override;
        virtual bool appendFile(const FileName& _path, const FileContent& _content) override;
        virtual bool appendTextFile(const FileName& _path, const std::string& _content) override;

        virtual FileSystemWatchID watchDirectory(
            const FileName& _directoryPath,
            FileSystemWatchFilter _filter = FileSystemWatchFilter::Default,
            bool _recursive = false
        ) override;
        virtual bool unwatchDirectory(FileSystemWatchID _watchID) override;
        virtual void updateDirectoryWatches() override;

        EGO_RTTI_VIRTUAL(RootedFileSystem, FileSystem);

    private:
        static bool IsPathSeparator(char _ch);
        static size_t GetPathRootLength(const std::string& _path);
        static std::string NormalizeRootPath(const FileName& _path);
        static FileName ToFileName(const std::string& _path);

        bool resolveVirtualPath(const FileName& _path, std::string& _virtualPath) const;
        bool resolveSourcePath(const FileName& _path, FileName& _sourcePath) const;
        FileName buildSourcePath(const std::string& _virtualPath) const;
        FileName buildVirtualPath(const FileName& _sourcePath) const;
        void remapEntryPath(FileSystemEntryDesc& _entry) const;

        FileSystemPointer m_fileSystem = nullptr;
        FileName m_rootPath;
        std::string m_workingDirectory;
        bool m_isInitialized = false;
    };

    EGO_POINTER(RootedFileSystem);
}
