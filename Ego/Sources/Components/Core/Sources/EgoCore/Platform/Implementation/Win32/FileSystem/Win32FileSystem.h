#pragma once

#include <string>
#include <vector>

#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/Platform/PlatformMacros.h"

namespace ego::win32
{
    class Win32FileSystem final : public FileSystem
    {
    public:
        Win32FileSystem() = default;
        ~Win32FileSystem() override;

        bool init() override;
        void release() override;

        bool exists(const FileName& _path) const override;
        bool isFile(const FileName& _path) const override;
        bool isDirectory(const FileName& _path) const override;

        FileName getWorkingDirectory() const override;
        bool setWorkingDirectory(const FileName& _path) override;
        FileName getAbsolutePath(const FileName& _path) const override;

        bool getEntryInfo(const FileName& _path, FileSystemEntryDesc& _entry) const override;
        bool enumerate(const FileName& _directoryPath, FileSystemEntryCollection& _entries, bool _recursive = false) const override;

        bool createDirectory(const FileName& _path, bool _recursive = true) override;
        bool removeFile(const FileName& _path) override;
        bool removeDirectory(const FileName& _path, bool _recursive = false) override;
        bool remove(const FileName& _path, bool _recursive = false) override;
        bool copyFile(const FileName& _sourcePath, const FileName& _destinationPath, bool _overwrite = true) override;
        bool move(const FileName& _sourcePath, const FileName& _destinationPath, bool _overwrite = true) override;

        bool readFile(const FileName& _path, FileContent& _content) const override;
        bool readTextFile(const FileName& _path, std::string& _content) const override;
        bool writeFile(const FileName& _path, const FileContent& _content, bool _overwrite = true) override;
        bool writeTextFile(const FileName& _path, const std::string& _content, bool _overwrite = true) override;
        bool appendFile(const FileName& _path, const FileContent& _content) override;
        bool appendTextFile(const FileName& _path, const std::string& _content) override;

        EGO_RTTI_VIRTUAL(Win32FileSystem, FileSystem);

    private:
        class FileHandle final
        {
        public:
            FileHandle() = default;

            explicit FileHandle(HANDLE _handle)
                : m_handle(_handle)
            {
            }

            ~FileHandle()
            {
                close();
            }

            FileHandle(const FileHandle&) = delete;
            FileHandle& operator=(const FileHandle&) = delete;

            bool isValid() const
            {
                return m_handle != INVALID_HANDLE_VALUE && m_handle != nullptr;
            }

            HANDLE get() const
            {
                return m_handle;
            }

        private:
            void close()
            {
                if (isValid())
                {
                    CloseHandle(m_handle);
                    m_handle = INVALID_HANDLE_VALUE;
                }
            }

            HANDLE m_handle = INVALID_HANDLE_VALUE;
        };

        class FindHandle final
        {
        public:
            explicit FindHandle(HANDLE _handle)
                : m_handle(_handle)
            {
            }

            ~FindHandle()
            {
                if (isValid())
                {
                    FindClose(m_handle);
                }
            }

            FindHandle(const FindHandle&) = delete;
            FindHandle& operator=(const FindHandle&) = delete;

            bool isValid() const
            {
                return m_handle != INVALID_HANDLE_VALUE;
            }

            HANDLE get() const
            {
                return m_handle;
            }

        private:
            HANDLE m_handle = INVALID_HANDLE_VALUE;
        };

        static std::wstring ToWidePath(const FileName& _path);
        static FileName ToFileName(const std::wstring& _path);
        static bool IsPathSeparator(wchar_t _ch);
        static size_t GetPathRootLength(const std::wstring& _path);
        static std::wstring TrimTrailingSeparators(std::wstring _path);
        static std::wstring GetParentPath(const std::wstring& _path);
        static std::wstring AppendSearchWildcard(const std::wstring& _path);
        static std::wstring BuildChildPath(const std::wstring& _directoryPath, const wchar_t* _childName);
        static DWORD GetPathAttributes(const std::wstring& _path);
        static bool IsDirectoryPath(const std::wstring& _path);
        static FileSystemEntryType GetEntryType(DWORD _attributes);
        static uint64_t GetFileTimeValue(const FILETIME& _fileTime);
        static uint64_t GetFileSizeValue(DWORD _lowPart, DWORD _highPart);
        static void FillEntryDesc(
            const std::wstring& _path,
            DWORD _attributes,
            const FILETIME& _creationTime,
            const FILETIME& _lastAccessTime,
            const FILETIME& _lastWriteTime,
            uint64_t _size,
            FileSystemEntryDesc& _entry);
        static void FillEntryDescFromFindData(const std::wstring& _path, const WIN32_FIND_DATAW& _findData, FileSystemEntryDesc& _entry);
        static bool IsSelfOrParentDirectory(const wchar_t* _name);
        static bool CreateDirectoryRecursive(const std::wstring& _path);
        static bool EnumerateDirectory(const std::wstring& _directoryPath, bool _recursive, FileSystemEntryCollection& _entries);
        static bool RemoveDirectoryRecursive(const std::wstring& _directoryPath);
        static bool ReadFileContent(HANDLE _fileHandle, FileContent& _content);
        static bool WriteFileContent(HANDLE _fileHandle, const FileContent& _content);
        static FileContent ToFileContent(const std::string& _content);

        bool m_isInitialized = false;
    };

    EGO_POINTER(Win32FileSystem)
} // namespace ego::win32
