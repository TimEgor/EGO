#include "Win32FileSystem.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/PlatformMacros.h"
#include "EgoCore/String/StringConverter.h"

#include "EgoEngine/Engine.h"

#include <algorithm>
#include <cwchar>
#include <limits>
#include <vector>

std::wstring ego::win32::Win32FileSystem::ToWidePath(const FileName& _path)
{
    return ConvertStringToWString(_path.c_str());
}

ego::FileName ego::win32::Win32FileSystem::ToFileName(const std::wstring& _path)
{
    return FileName(ConvertWStringToString(_path));
}

bool ego::win32::Win32FileSystem::IsPathSeparator(wchar_t _ch)
{
    return _ch == L'\\' || _ch == L'/';
}

size_t ego::win32::Win32FileSystem::GetPathRootLength(const std::wstring& _path)
{
    if (_path.empty())
    {
        return 0;
    }

    if (_path.size() >= 2 && IsPathSeparator(_path[0]) && IsPathSeparator(_path[1]))
    {
        const size_t serverEnd = _path.find_first_of(L"\\/", 2);
        if (serverEnd == std::wstring::npos)
        {
            return _path.size();
        }

        const size_t shareEnd = _path.find_first_of(L"\\/", serverEnd + 1);
        return shareEnd == std::wstring::npos ? _path.size() : shareEnd + 1;
    }

    if (_path.size() >= 2 && _path[1] == L':')
    {
        return _path.size() >= 3 && IsPathSeparator(_path[2]) ? 3 : 2;
    }

    return IsPathSeparator(_path[0]) ? 1 : 0;
}

std::wstring ego::win32::Win32FileSystem::TrimTrailingSeparators(std::wstring _path)
{
    const size_t rootLength = GetPathRootLength(_path);

    while (_path.size() > rootLength && IsPathSeparator(_path.back()))
    {
        _path.pop_back();
    }

    return _path;
}

std::wstring ego::win32::Win32FileSystem::GetParentPath(const std::wstring& _path)
{
    const std::wstring path = TrimTrailingSeparators(_path);
    const size_t rootLength = GetPathRootLength(path);

    if (path.size() <= rootLength)
    {
        return path;
    }

    const size_t separatorPos = path.find_last_of(L"\\/");
    if (separatorPos == std::wstring::npos)
    {
        return std::wstring();
    }

    if (separatorPos < rootLength)
    {
        return path.substr(0, rootLength);
    }

    return path.substr(0, separatorPos);
}

std::wstring ego::win32::Win32FileSystem::AppendSearchWildcard(const std::wstring& _path)
{
    std::wstring searchPath = TrimTrailingSeparators(_path);
    if (!searchPath.empty() && !IsPathSeparator(searchPath.back()))
    {
        searchPath += L'\\';
    }

    searchPath += L'*';

    return searchPath;
}

std::wstring ego::win32::Win32FileSystem::BuildChildPath(const std::wstring& _directoryPath, const wchar_t* _childName)
{
    std::wstring childPath = TrimTrailingSeparators(_directoryPath);
    if (!childPath.empty() && !IsPathSeparator(childPath.back()))
    {
        childPath += L'\\';
    }

    childPath += _childName;

    return childPath;
}

DWORD ego::win32::Win32FileSystem::GetPathAttributes(const std::wstring& _path)
{
    return GetFileAttributesW(_path.c_str());
}

bool ego::win32::Win32FileSystem::IsDirectoryPath(const std::wstring& _path)
{
    const DWORD attributes = GetPathAttributes(_path);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

ego::FileSystemEntryType ego::win32::Win32FileSystem::GetEntryType(DWORD _attributes)
{
    if (_attributes == INVALID_FILE_ATTRIBUTES)
    {
        return FileSystemEntryType::Unknown;
    }

    return (_attributes & FILE_ATTRIBUTE_DIRECTORY) ? FileSystemEntryType::Directory : FileSystemEntryType::File;
}

uint64_t ego::win32::Win32FileSystem::GetFileTimeValue(const FILETIME& _fileTime)
{
    ULARGE_INTEGER value;
    value.LowPart = _fileTime.dwLowDateTime;
    value.HighPart = _fileTime.dwHighDateTime;
    return value.QuadPart;
}

uint64_t ego::win32::Win32FileSystem::GetFileSizeValue(DWORD _lowPart, DWORD _highPart)
{
    ULARGE_INTEGER value;
    value.LowPart = _lowPart;
    value.HighPart = _highPart;
    return value.QuadPart;
}

void ego::win32::Win32FileSystem::FillEntryDesc(
    const std::wstring& _path,
    DWORD _attributes,
    const FILETIME& _creationTime,
    const FILETIME& _lastAccessTime,
    const FILETIME& _lastWriteTime,
    uint64_t _size,
    FileSystemEntryDesc& _entry)
{
    _entry.m_path = ToFileName(_path);
    _entry.m_type = GetEntryType(_attributes);
    _entry.m_size = _entry.m_type == FileSystemEntryType::File ? _size : 0;
    _entry.m_creationTime = GetFileTimeValue(_creationTime);
    _entry.m_lastAccessTime = GetFileTimeValue(_lastAccessTime);
    _entry.m_lastWriteTime = GetFileTimeValue(_lastWriteTime);
    _entry.m_isReadOnly = (_attributes & FILE_ATTRIBUTE_READONLY) != 0;
    _entry.m_isHidden = (_attributes & FILE_ATTRIBUTE_HIDDEN) != 0;
}

void ego::win32::Win32FileSystem::FillEntryDescFromFindData(const std::wstring& _path, const WIN32_FIND_DATAW& _findData, FileSystemEntryDesc& _entry)
{
    FillEntryDesc(
        _path,
        _findData.dwFileAttributes,
        _findData.ftCreationTime,
        _findData.ftLastAccessTime,
        _findData.ftLastWriteTime,
        GetFileSizeValue(_findData.nFileSizeLow, _findData.nFileSizeHigh),
        _entry);
}

bool ego::win32::Win32FileSystem::IsSelfOrParentDirectory(const wchar_t* _name)
{
    return wcscmp(_name, L".") == 0 || wcscmp(_name, L"..") == 0;
}

bool ego::win32::Win32FileSystem::CreateDirectoryRecursive(const std::wstring& _path)
{
    if (_path.empty())
    {
        return false;
    }

    if (IsDirectoryPath(_path))
    {
        return true;
    }

    const std::wstring parentPath = GetParentPath(_path);
    if (!parentPath.empty() && parentPath != _path && !IsDirectoryPath(parentPath))
    {
        if (!CreateDirectoryRecursive(parentPath))
        {
            return false;
        }
    }

    if (CreateDirectoryW(_path.c_str(), nullptr))
    {
        return true;
    }

    return GetLastError() == ERROR_ALREADY_EXISTS && IsDirectoryPath(_path);
}

bool ego::win32::Win32FileSystem::EnumerateDirectory(const std::wstring& _directoryPath, bool _recursive, FileSystemEntryCollection& _entries)
{
    WIN32_FIND_DATAW findData;
    FindHandle findHandle(FindFirstFileW(AppendSearchWildcard(_directoryPath).c_str(), &findData));

    if (!findHandle.isValid())
    {
        return GetLastError() == ERROR_FILE_NOT_FOUND;
    }

    do
    {
        if (IsSelfOrParentDirectory(findData.cFileName))
        {
            continue;
        }

        const std::wstring entryPath = BuildChildPath(_directoryPath, findData.cFileName);

        FileSystemEntryDesc entry;
        FillEntryDescFromFindData(entryPath, findData, entry);
        _entries.push_back(entry);

        const bool isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        const bool isReparsePoint = (findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
        if (_recursive && isDirectory && !isReparsePoint)
        {
            if (!EnumerateDirectory(entryPath, true, _entries))
            {
                return false;
            }
        }
    } while (FindNextFileW(findHandle.get(), &findData));

    return GetLastError() == ERROR_NO_MORE_FILES;
}

bool ego::win32::Win32FileSystem::RemoveDirectoryRecursive(const std::wstring& _directoryPath)
{
    WIN32_FIND_DATAW findData;
    FindHandle findHandle(FindFirstFileW(AppendSearchWildcard(_directoryPath).c_str(), &findData));

    if (!findHandle.isValid() && GetLastError() != ERROR_FILE_NOT_FOUND)
    {
        return false;
    }

    if (findHandle.isValid())
    {
        do
        {
            if (IsSelfOrParentDirectory(findData.cFileName))
            {
                continue;
            }

            const std::wstring entryPath = BuildChildPath(_directoryPath, findData.cFileName);
            const bool isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            const bool isReparsePoint = (findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;

            if (isDirectory)
            {
                if (isReparsePoint)
                {
                    if (!RemoveDirectoryW(entryPath.c_str()))
                    {
                        return false;
                    }
                }
                else if (!RemoveDirectoryRecursive(entryPath))
                {
                    return false;
                }
            }
            else if (!DeleteFileW(entryPath.c_str()))
            {
                return false;
            }
        } while (FindNextFileW(findHandle.get(), &findData));

        if (GetLastError() != ERROR_NO_MORE_FILES)
        {
            return false;
        }
    }

    return RemoveDirectoryW(_directoryPath.c_str()) != FALSE;
}

bool ego::win32::Win32FileSystem::ReadFileContent(HANDLE _fileHandle, FileContent& _content)
{
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(_fileHandle, &fileSize) || fileSize.QuadPart < 0)
    {
        return false;
    }

    if (static_cast<uint64_t>(fileSize.QuadPart) > std::numeric_limits<size_t>::max())
    {
        return false;
    }

    _content.clear();
    _content.resize(static_cast<size_t>(fileSize.QuadPart));
    if (_content.empty())
    {
        return true;
    }

    size_t remainingSize = _content.size();
    uint8_t* writePosition = _content.data();

    while (remainingSize > 0)
    {
        const DWORD readSize = static_cast<DWORD>(std::min<size_t>(remainingSize, std::numeric_limits<DWORD>::max()));
        DWORD processedSize = 0;

        if (!ReadFile(_fileHandle, writePosition, readSize, &processedSize, nullptr) || processedSize == 0)
        {
            return false;
        }

        remainingSize -= processedSize;
        writePosition += processedSize;
    }

    return true;
}

bool ego::win32::Win32FileSystem::WriteFileContent(HANDLE _fileHandle, const FileContent& _content)
{
    size_t remainingSize = _content.size();
    const uint8_t* readPosition = _content.data();

    while (remainingSize > 0)
    {
        const DWORD writeSize = static_cast<DWORD>(std::min<size_t>(remainingSize, std::numeric_limits<DWORD>::max()));
        DWORD processedSize = 0;

        if (!WriteFile(_fileHandle, readPosition, writeSize, &processedSize, nullptr) || processedSize == 0)
        {
            return false;
        }

        remainingSize -= processedSize;
        readPosition += processedSize;
    }

    return true;
}

ego::FileContent ego::win32::Win32FileSystem::ToFileContent(const std::string& _content)
{
    return FileContent(_content.begin(), _content.end());
}

DWORD ego::win32::Win32FileSystem::ToWin32NotifyFilter(FileSystemWatchFilter _filter)
{
    DWORD filter = 0;

    if (HasFileSystemWatchFilter(_filter, FileSystemWatchFilter::FileName))
    {
        filter |= FILE_NOTIFY_CHANGE_FILE_NAME;
    }

    if (HasFileSystemWatchFilter(_filter, FileSystemWatchFilter::DirectoryName))
    {
        filter |= FILE_NOTIFY_CHANGE_DIR_NAME;
    }

    if (HasFileSystemWatchFilter(_filter, FileSystemWatchFilter::Attributes))
    {
        filter |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
    }

    if (HasFileSystemWatchFilter(_filter, FileSystemWatchFilter::Size))
    {
        filter |= FILE_NOTIFY_CHANGE_SIZE;
    }

    if (HasFileSystemWatchFilter(_filter, FileSystemWatchFilter::LastWrite))
    {
        filter |= FILE_NOTIFY_CHANGE_LAST_WRITE;
    }

    if (HasFileSystemWatchFilter(_filter, FileSystemWatchFilter::LastAccess))
    {
        filter |= FILE_NOTIFY_CHANGE_LAST_ACCESS;
    }

    if (HasFileSystemWatchFilter(_filter, FileSystemWatchFilter::Creation))
    {
        filter |= FILE_NOTIFY_CHANGE_CREATION;
    }

    if (HasFileSystemWatchFilter(_filter, FileSystemWatchFilter::Security))
    {
        filter |= FILE_NOTIFY_CHANGE_SECURITY;
    }

    return filter ? filter : ToWin32NotifyFilter(FileSystemWatchFilter::Default);
}

ego::FileSystemChangeType ego::win32::Win32FileSystem::ToFileSystemChangeType(DWORD _action)
{
    switch (_action)
    {
    case FILE_ACTION_ADDED:
        return FileSystemChangeType::Added;
    case FILE_ACTION_REMOVED:
        return FileSystemChangeType::Removed;
    case FILE_ACTION_MODIFIED:
        return FileSystemChangeType::Modified;
    case FILE_ACTION_RENAMED_OLD_NAME:
        return FileSystemChangeType::RenamedOldName;
    case FILE_ACTION_RENAMED_NEW_NAME:
        return FileSystemChangeType::RenamedNewName;
    default:
        return FileSystemChangeType::Unknown;
    }
}

void ego::win32::Win32FileSystem::PushPendingFileSystemChangeEvent(
    FileSystemWatchID _watchID,
    const FileName& _directoryPath,
    const FileName& _path,
    FileSystemChangeType _changeType,
    std::vector<PendingFileSystemChangeEvent>& _events)
{
    _events.push_back(PendingFileSystemChangeEvent{_watchID, _directoryPath, _path, _changeType});
}

bool ego::win32::Win32FileSystem::IsWatchHandleValid(const WatchRecord& _watch)
{
    return _watch.m_directoryHandle != INVALID_HANDLE_VALUE && _watch.m_directoryHandle != nullptr;
}

void ego::win32::Win32FileSystem::CloseDirectoryWatch(WatchRecord& _watch)
{
    if (IsWatchHandleValid(_watch))
    {
        if (_watch.m_isReadPending)
        {
            CancelIoEx(_watch.m_directoryHandle, &_watch.m_overlapped);
            _watch.m_isReadPending = false;
        }

        CloseHandle(_watch.m_directoryHandle);
        _watch.m_directoryHandle = INVALID_HANDLE_VALUE;
    }

    if (_watch.m_overlapped.hEvent)
    {
        CloseHandle(_watch.m_overlapped.hEvent);
        _watch.m_overlapped.hEvent = nullptr;
    }
}

bool ego::win32::Win32FileSystem::StartDirectoryWatchRead(WatchRecord& _watch)
{
    EGO_ASSERT(IsWatchHandleValid(_watch));
    EGO_ASSERT(_watch.m_overlapped.hEvent);

    HANDLE eventHandle = _watch.m_overlapped.hEvent;
    std::memset(&_watch.m_overlapped, 0, sizeof(_watch.m_overlapped));
    _watch.m_overlapped.hEvent = eventHandle;

    ResetEvent(eventHandle);

    const BOOL isStarted = ReadDirectoryChangesW(
        _watch.m_directoryHandle,
        _watch.m_buffer.data(),
        static_cast<DWORD>(_watch.m_buffer.size()),
        _watch.m_recursive,
        ToWin32NotifyFilter(_watch.m_filter),
        nullptr,
        &_watch.m_overlapped,
        nullptr);

    if (isStarted)
    {
        _watch.m_isReadPending = true;
        return true;
    }

    const DWORD error = GetLastError();
    if (error == ERROR_IO_PENDING)
    {
        _watch.m_isReadPending = true;
        return true;
    }

    _watch.m_isReadPending = false;
    return false;
}

void ego::win32::Win32FileSystem::ParseDirectoryWatchBuffer(const WatchRecord& _watch, DWORD _bytesTransferred, std::vector<PendingFileSystemChangeEvent>& _events)
{
    if (_bytesTransferred == 0)
    {
        PushPendingFileSystemChangeEvent(_watch.m_watchID, _watch.m_directoryPath, _watch.m_directoryPath, FileSystemChangeType::Unknown, _events);
        return;
    }

    const uint8_t* currentEntryPtr = _watch.m_buffer.data();
    const uint8_t* const bufferEnd = _watch.m_buffer.data() + _bytesTransferred;

    while (currentEntryPtr < bufferEnd)
    {
        auto notifyInfo = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(currentEntryPtr);

        const std::wstring relativePath(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(wchar_t));
        const std::wstring fullPath = BuildChildPath(_watch.m_directoryPathW, relativePath.c_str());

        PushPendingFileSystemChangeEvent(_watch.m_watchID, _watch.m_directoryPath, ToFileName(fullPath), ToFileSystemChangeType(notifyInfo->Action), _events);

        if (notifyInfo->NextEntryOffset == 0)
        {
            break;
        }

        currentEntryPtr += notifyInfo->NextEntryOffset;
    }
}

ego::win32::Win32FileSystem::~Win32FileSystem()
{
    release();
}

bool ego::win32::Win32FileSystem::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    m_isChangeEventRegistered = engine::GetEngine().getEventController().registerEvent<FileSystemChangeEvent>();
    m_isInitialized = true;

    return true;
}

void ego::win32::Win32FileSystem::release()
{
    std::vector<FileSystemWatchID> watchIDs;
    watchIDs.reserve(m_watches.size());

    for (const auto& watch : m_watches)
    {
        watchIDs.push_back(watch.first);
    }

    for (FileSystemWatchID watchID : watchIDs)
    {
        unwatchDirectory(watchID);
    }

    if (m_isChangeEventRegistered)
    {
        engine::GetEngine().getEventController().unregisterEvent<FileSystemChangeEvent>();
        m_isChangeEventRegistered = false;
    }

    m_isInitialized = false;
}

bool ego::win32::Win32FileSystem::exists(const FileName& _path) const
{
    return GetPathAttributes(ToWidePath(_path)) != INVALID_FILE_ATTRIBUTES;
}

bool ego::win32::Win32FileSystem::isFile(const FileName& _path) const
{
    const DWORD attributes = GetPathAttributes(ToWidePath(_path));
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool ego::win32::Win32FileSystem::isDirectory(const FileName& _path) const
{
    return IsDirectoryPath(ToWidePath(_path));
}

ego::FileName ego::win32::Win32FileSystem::getWorkingDirectory() const
{
    const DWORD requiredSize = GetCurrentDirectoryW(0, nullptr);
    if (requiredSize == 0)
    {
        return FileName();
    }

    std::wstring path(requiredSize, L'\0');
    const DWORD writtenSize = GetCurrentDirectoryW(requiredSize, path.data());
    if (writtenSize == 0 || writtenSize >= requiredSize)
    {
        return FileName();
    }

    path.resize(writtenSize);

    return ToFileName(path);
}

bool ego::win32::Win32FileSystem::setWorkingDirectory(const FileName& _path)
{
    return SetCurrentDirectoryW(ToWidePath(_path).c_str()) != FALSE;
}

ego::FileName ego::win32::Win32FileSystem::getAbsolutePath(const FileName& _path) const
{
    const std::wstring path = ToWidePath(_path);
    const DWORD requiredSize = GetFullPathNameW(path.c_str(), 0, nullptr, nullptr);
    if (requiredSize == 0)
    {
        return FileName();
    }

    std::wstring absolutePath(requiredSize, L'\0');
    const DWORD writtenSize = GetFullPathNameW(path.c_str(), requiredSize, absolutePath.data(), nullptr);
    if (writtenSize == 0 || writtenSize >= requiredSize)
    {
        return FileName();
    }

    absolutePath.resize(writtenSize);

    return ToFileName(absolutePath);
}

bool ego::win32::Win32FileSystem::getEntryInfo(const FileName& _path, FileSystemEntryDesc& _entry) const
{
    const std::wstring path = ToWidePath(_path);

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &data))
    {
        _entry = FileSystemEntryDesc();
        return false;
    }

    FillEntryDesc(path, data.dwFileAttributes, data.ftCreationTime, data.ftLastAccessTime, data.ftLastWriteTime, GetFileSizeValue(data.nFileSizeLow, data.nFileSizeHigh), _entry);

    return true;
}

bool ego::win32::Win32FileSystem::enumerate(const FileName& _directoryPath, FileSystemEntryCollection& _entries, bool _recursive) const
{
    _entries.clear();

    const std::wstring directoryPath = ToWidePath(_directoryPath);
    if (!IsDirectoryPath(directoryPath))
    {
        return false;
    }

    return EnumerateDirectory(directoryPath, _recursive, _entries);
}

bool ego::win32::Win32FileSystem::createDirectory(const FileName& _path, bool _recursive)
{
    const std::wstring path = ToWidePath(_path);
    if (_recursive)
    {
        return CreateDirectoryRecursive(path);
    }

    if (CreateDirectoryW(path.c_str(), nullptr))
    {
        return true;
    }

    return GetLastError() == ERROR_ALREADY_EXISTS && IsDirectoryPath(path);
}

bool ego::win32::Win32FileSystem::removeFile(const FileName& _path)
{
    return DeleteFileW(ToWidePath(_path).c_str()) != FALSE;
}

bool ego::win32::Win32FileSystem::removeDirectory(const FileName& _path, bool _recursive)
{
    const std::wstring path = ToWidePath(_path);
    return _recursive ? RemoveDirectoryRecursive(path) : RemoveDirectoryW(path.c_str()) != FALSE;
}

bool ego::win32::Win32FileSystem::remove(const FileName& _path, bool _recursive)
{
    const std::wstring path = ToWidePath(_path);
    const DWORD attributes = GetPathAttributes(path);
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    return (attributes & FILE_ATTRIBUTE_DIRECTORY) ? removeDirectory(_path, _recursive) : removeFile(_path);
}

bool ego::win32::Win32FileSystem::copyFile(const FileName& _sourcePath, const FileName& _destinationPath, bool _overwrite)
{
    return CopyFileW(ToWidePath(_sourcePath).c_str(), ToWidePath(_destinationPath).c_str(), !_overwrite) != FALSE;
}

bool ego::win32::Win32FileSystem::move(const FileName& _sourcePath, const FileName& _destinationPath, bool _overwrite)
{
    DWORD flags = MOVEFILE_COPY_ALLOWED;
    if (_overwrite)
    {
        flags |= MOVEFILE_REPLACE_EXISTING;
    }

    return MoveFileExW(ToWidePath(_sourcePath).c_str(), ToWidePath(_destinationPath).c_str(), flags) != FALSE;
}

bool ego::win32::Win32FileSystem::readFile(const FileName& _path, FileContent& _content) const
{
    FileHandle fileHandle(CreateFileW(ToWidePath(_path).c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));

    if (!fileHandle.isValid())
    {
        _content.clear();
        return false;
    }

    if (!ReadFileContent(fileHandle.get(), _content))
    {
        _content.clear();
        return false;
    }

    return true;
}

bool ego::win32::Win32FileSystem::readTextFile(const FileName& _path, std::string& _content) const
{
    FileContent content;
    if (!readFile(_path, content))
    {
        _content.clear();
        return false;
    }

    if (content.empty())
    {
        _content.clear();
        return true;
    }

    _content.assign(reinterpret_cast<const char*>(content.data()), content.size());

    return true;
}

bool ego::win32::Win32FileSystem::writeFile(const FileName& _path, const FileContent& _content, bool _overwrite)
{
    FileHandle fileHandle(CreateFileW(ToWidePath(_path).c_str(), GENERIC_WRITE, 0, nullptr, _overwrite ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr));

    return fileHandle.isValid() && WriteFileContent(fileHandle.get(), _content);
}

bool ego::win32::Win32FileSystem::writeTextFile(const FileName& _path, const std::string& _content, bool _overwrite)
{
    return writeFile(_path, ToFileContent(_content), _overwrite);
}

bool ego::win32::Win32FileSystem::appendFile(const FileName& _path, const FileContent& _content)
{
    FileHandle fileHandle(CreateFileW(ToWidePath(_path).c_str(), FILE_APPEND_DATA, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));

    return fileHandle.isValid() && WriteFileContent(fileHandle.get(), _content);
}

bool ego::win32::Win32FileSystem::appendTextFile(const FileName& _path, const std::string& _content)
{
    return appendFile(_path, ToFileContent(_content));
}

ego::FileSystemWatchID ego::win32::Win32FileSystem::watchDirectory(const FileName& _directoryPath, FileSystemWatchFilter _filter, bool _recursive)
{
    const std::wstring directoryPath = ToWidePath(_directoryPath);
    if (!IsDirectoryPath(directoryPath))
    {
        return InvalidFileSystemWatchID;
    }

    FileSystemWatchID watchID = engine::GetEngine().getEventController().registerInstancedEvent<FileSystemChangeEvent>();
    if (watchID == InvalidFileSystemWatchID)
    {
        return InvalidFileSystemWatchID;
    }

    auto watch = std::make_unique<WatchRecord>();
    watch->m_directoryHandle = CreateFileW(
        directoryPath.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        nullptr);

    if (!IsWatchHandleValid(*watch))
    {
        engine::GetEngine().getEventController().unregisterInstancedEvent(watchID);
        return InvalidFileSystemWatchID;
    }

    watch->m_overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!watch->m_overlapped.hEvent)
    {
        CloseDirectoryWatch(*watch);
        engine::GetEngine().getEventController().unregisterInstancedEvent(watchID);
        return InvalidFileSystemWatchID;
    }

    watch->m_watchID = watchID;
    watch->m_directoryPath = getAbsolutePath(_directoryPath);
    watch->m_directoryPathW = ToWidePath(watch->m_directoryPath);
    watch->m_filter = _filter;
    watch->m_recursive = _recursive;
    watch->m_buffer.resize(DirectoryWatchBufferSize);

    if (!StartDirectoryWatchRead(*watch))
    {
        CloseDirectoryWatch(*watch);
        engine::GetEngine().getEventController().unregisterInstancedEvent(watchID);
        return InvalidFileSystemWatchID;
    }

    m_watches.insert(std::make_pair(watchID, std::move(watch)));

    return watchID;
}

bool ego::win32::Win32FileSystem::unwatchDirectory(FileSystemWatchID _watchID)
{
    const auto watchIter = m_watches.find(_watchID);
    if (watchIter == m_watches.end())
    {
        return false;
    }

    CloseDirectoryWatch(*watchIter->second);
    m_watches.erase(watchIter);

    engine::GetEngine().getEventController().unregisterInstancedEvent(_watchID);

    return true;
}

void ego::win32::Win32FileSystem::updateDirectoryWatches()
{
    std::vector<PendingFileSystemChangeEvent> pendingEvents;
    std::vector<FileSystemWatchID> failedWatchIDs;

    for (auto& watch : m_watches)
    {
        WatchRecord& watchRecord = *watch.second;
        if (!watchRecord.m_isReadPending)
        {
            if (!StartDirectoryWatchRead(watchRecord))
            {
                failedWatchIDs.push_back(watch.first);
            }

            continue;
        }

        const DWORD waitResult = WaitForSingleObject(watchRecord.m_overlapped.hEvent, 0);
        if (waitResult != WAIT_OBJECT_0)
        {
            if (waitResult == WAIT_FAILED)
            {
                failedWatchIDs.push_back(watch.first);
            }

            continue;
        }

        DWORD bytesTransferred = 0;
        if (GetOverlappedResult(watchRecord.m_directoryHandle, &watchRecord.m_overlapped, &bytesTransferred, FALSE))
        {
            watchRecord.m_isReadPending = false;
            ParseDirectoryWatchBuffer(watchRecord, bytesTransferred, pendingEvents);

            if (!StartDirectoryWatchRead(watchRecord))
            {
                failedWatchIDs.push_back(watch.first);
            }
        }
        else
        {
            const DWORD error = GetLastError();
            watchRecord.m_isReadPending = false;

            if (error != ERROR_OPERATION_ABORTED && !StartDirectoryWatchRead(watchRecord))
            {
                failedWatchIDs.push_back(watch.first);
            }
        }
    }

    for (FileSystemWatchID watchID : failedWatchIDs)
    {
        unwatchDirectory(watchID);
    }

    EventController& eventController = engine::GetEngine().getEventController();
    for (const PendingFileSystemChangeEvent& pendingEvent : pendingEvents)
    {
        const FileSystemChangeEvent event(pendingEvent.m_directoryPath, pendingEvent.m_path, pendingEvent.m_changeType);

        eventController.emitEvent(event);
        eventController.emitInstancedEvent(pendingEvent.m_watchID, event);
    }
}
