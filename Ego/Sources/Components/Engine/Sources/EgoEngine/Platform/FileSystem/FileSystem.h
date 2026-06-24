#pragma once

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "FileSystemEvents.h"

#include <cstdint>
#include <string>
#include <vector>

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
    using FileSystemWatchID = InstancedEventID;

    inline constexpr FileSystemWatchID InvalidFileSystemWatchID = InvalidInstancedEventID;

    enum class FileSystemWatchFilter : uint32_t
    {
        None = 0,
        FileName = 1 << 0,
        DirectoryName = 1 << 1,
        Attributes = 1 << 2,
        Size = 1 << 3,
        LastWrite = 1 << 4,
        LastAccess = 1 << 5,
        Creation = 1 << 6,
        Security = 1 << 7,
        Default = FileName | DirectoryName | Size | LastWrite | Creation,
        All = FileName | DirectoryName | Attributes | Size | LastWrite | LastAccess | Creation | Security
    };

    inline constexpr FileSystemWatchFilter operator|(FileSystemWatchFilter _left, FileSystemWatchFilter _right)
    {
        return static_cast<FileSystemWatchFilter>(static_cast<uint32_t>(_left) | static_cast<uint32_t>(_right));
    }

    inline constexpr FileSystemWatchFilter operator&(FileSystemWatchFilter _left, FileSystemWatchFilter _right)
    {
        return static_cast<FileSystemWatchFilter>(static_cast<uint32_t>(_left) & static_cast<uint32_t>(_right));
    }

    inline constexpr bool HasFileSystemWatchFilter(FileSystemWatchFilter _filter, FileSystemWatchFilter _flag)
    {
        return static_cast<uint32_t>(_filter & _flag) != 0;
    }

    class FileSystem
    {
    public:
        FileSystem() = default;
        virtual ~FileSystem() = default;

        virtual bool init() = 0;
        virtual void release() = 0;

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

        virtual FileSystemWatchID watchDirectory(const FileName& _directoryPath, FileSystemWatchFilter _filter = FileSystemWatchFilter::Default, bool _recursive = false) = 0;
        virtual bool unwatchDirectory(FileSystemWatchID _watchID) = 0;
        virtual void updateDirectoryWatches() = 0;

        EGO_RTTI_VIRTUAL_BASE(FileSystem);
    };

    EGO_POINTER(FileSystem);
    EGO_WEAK_POINTER(FileSystem);
} // namespace ego
