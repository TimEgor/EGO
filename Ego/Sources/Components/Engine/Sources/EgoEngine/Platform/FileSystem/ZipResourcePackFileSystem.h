#pragma once

#include "FileSystem.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace ego
{
    class ZipResourcePackFileSystem final : public FileSystem
    {
    public:
        ZipResourcePackFileSystem() = default;
        ZipResourcePackFileSystem(const FileSystem& _sourceFileSystem, const FileName& _packPath);
        ~ZipResourcePackFileSystem() override;

        bool open(const FileSystem& _sourceFileSystem, const FileName& _packPath);
        bool open(const FileContent& _content, const FileName& _packPath = FileName());
        bool isOpen() const;
        const FileName& getPackPath() const;

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

        EGO_RTTI_VIRTUAL(ZipResourcePackFileSystem, FileSystem);

    private:
        struct Entry final
        {
            FileName m_path;
            std::string m_normalizedPath;
            FileSystemEntryType m_type = FileSystemEntryType::Unknown;
            uint16_t m_generalPurposeFlags = 0;
            uint16_t m_compressionMethod = 0;
            uint32_t m_crc32 = 0;
            uint64_t m_compressedSize = 0;
            uint64_t m_uncompressedSize = 0;
            uint64_t m_localHeaderOffset = 0;
            uint64_t m_creationTime = 0;
            uint64_t m_lastAccessTime = 0;
            uint64_t m_lastWriteTime = 0;
            bool m_isReadOnly = true;
            bool m_isHidden = false;
            bool m_isSynthetic = false;
        };

        using EntryIndexMap = std::unordered_map<std::string, size_t>;

        static bool IsPathSeparator(char _ch);
        static bool IsDirectoryName(const std::string& _path);
        static std::string GetParentPath(const std::string& _path);
        static bool NormalizeArchivePath(const std::string& _path, std::string& _normalizedPath);
        static FileName ToFileName(const std::string& _path);

        void clear();
        bool loadArchive(FileContent&& _content, const FileName& _packPath);
        bool addEntry(Entry&& _entry);
        bool ensureDirectory(const std::string& _path);
        bool resolvePath(const FileName& _path, std::string& _normalizedPath) const;
        const Entry* findEntry(const std::string& _path) const;
        Entry* findEntry(const std::string& _path);
        void fillEntryDesc(const Entry& _entry, FileSystemEntryDesc& _desc) const;
        bool getEntryData(const Entry& _entry, const uint8_t*& _data, size_t& _size) const;

        FileContent m_archiveData;
        std::vector<Entry> m_entries;
        EntryIndexMap m_entryIndices;
        FileName m_packPath;
        std::string m_workingDirectory;
        bool m_isInitialized = false;
        bool m_isOpen = false;
    };
}
