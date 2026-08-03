#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Pointer/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

namespace ego
{
    using FileContent = std::vector<uint8_t>;

    enum class FileSystemEntryType
    {
        Unknown,
        File,
        Directory
    };

    struct FileSystemEntryDesc final
    {
        FileName m_path;
        FileSystemEntryType m_type = FileSystemEntryType::Unknown;
        uint64_t m_size = 0;
        uint64_t m_creationTime = 0;
        uint64_t m_lastAccessTime = 0;
        uint64_t m_lastWriteTime = 0;
        bool m_isReadOnly = false;
        bool m_isHidden = false;

        bool isFile() const
        {
            return m_type == FileSystemEntryType::File;
        }
        bool isDirectory() const
        {
            return m_type == FileSystemEntryType::Directory;
        }
    };

    using FileSystemEntryCollection = std::vector<FileSystemEntryDesc>;

    class FileSystem
    {
    public:
        FileSystem() = default;
        virtual ~FileSystem() = default;

        virtual bool init() = 0;

        virtual bool exists(const FileName& _path) const = 0;
        virtual bool isFile(const FileName& _path) const = 0;
        virtual bool isDirectory(const FileName& _path) const = 0;

        virtual FileName getWorkingDirectory() const = 0;
        virtual bool setWorkingDirectory(const FileName& _path) = 0;
        virtual FileName getAbsolutePath(const FileName& _path) const = 0;

        virtual bool getEntryInfo(const FileName& _path, FileSystemEntryDesc& _entry) const = 0;
        virtual bool enumerate(const FileName& _directoryPath, FileSystemEntryCollection& _entries, bool _recursive = false) const = 0;

        virtual bool createDirectory(const FileName& _path, bool _recursive = true) = 0;
        virtual bool removeFile(const FileName& _path) = 0;
        virtual bool removeDirectory(const FileName& _path, bool _recursive = false) = 0;
        virtual bool remove(const FileName& _path, bool _recursive = false) = 0;
        virtual bool copyFile(const FileName& _sourcePath, const FileName& _destinationPath, bool _overwrite = true) = 0;
        virtual bool move(const FileName& _sourcePath, const FileName& _destinationPath, bool _overwrite = true) = 0;

        virtual bool readFile(const FileName& _path, FileContent& _content) const = 0;
        virtual bool readTextFile(const FileName& _path, std::string& _content) const = 0;
        virtual bool writeFile(const FileName& _path, const FileContent& _content, bool _overwrite = true) = 0;
        virtual bool writeTextFile(const FileName& _path, const std::string& _content, bool _overwrite = true) = 0;
        virtual bool appendFile(const FileName& _path, const FileContent& _content) = 0;
        virtual bool appendTextFile(const FileName& _path, const std::string& _content) = 0;

        EGO_RTTI_VIRTUAL_BASE(FileSystem);
    };

    EGO_POINTER(FileSystem);
    EGO_WEAK_POINTER(FileSystem);
} // namespace ego
