#include "RootedFileSystem.h"

#include <algorithm>
#include <utility>
#include <vector>

ego::RootedFileSystem::RootedFileSystem(const FileSystemPointer& _fileSystem, const FileName& _rootPath)
{
    setSourceFileSystem(_fileSystem, _rootPath);
}

ego::RootedFileSystem::~RootedFileSystem()
{
    release();
}

bool ego::RootedFileSystem::setSourceFileSystem(const FileSystemPointer& _fileSystem, const FileName& _rootPath)
{
    if (m_isInitialized)
    {
        release();
    }

    m_fileSystem = _fileSystem;
    m_rootPath = NormalizeRootPath(_rootPath);
    m_workingDirectory.clear();

    return static_cast<bool>(m_fileSystem);
}

ego::FileSystemPointer ego::RootedFileSystem::getSourceFileSystem() const
{
    return m_fileSystem;
}

const ego::FileName& ego::RootedFileSystem::getRootPath() const
{
    return m_rootPath;
}

bool ego::RootedFileSystem::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    m_isInitialized = static_cast<bool>(m_fileSystem);
    return m_isInitialized;
}

void ego::RootedFileSystem::release()
{
    m_workingDirectory.clear();
    m_isInitialized = false;
}

bool ego::RootedFileSystem::exists(const FileName& _path) const
{
    FileName sourcePath;
    return m_isInitialized && resolveSourcePath(_path, sourcePath) && m_fileSystem->exists(sourcePath);
}

bool ego::RootedFileSystem::isFile(const FileName& _path) const
{
    FileName sourcePath;
    return m_isInitialized && resolveSourcePath(_path, sourcePath) && m_fileSystem->isFile(sourcePath);
}

bool ego::RootedFileSystem::isDirectory(const FileName& _path) const
{
    FileName sourcePath;
    return m_isInitialized && resolveSourcePath(_path, sourcePath) && m_fileSystem->isDirectory(sourcePath);
}

ego::FileName ego::RootedFileSystem::getWorkingDirectory() const
{
    return ToFileName(m_workingDirectory);
}

bool ego::RootedFileSystem::setWorkingDirectory(const FileName& _path)
{
    if (!m_isInitialized)
    {
        return false;
    }

    std::string virtualPath;
    if (!resolveVirtualPath(_path, virtualPath))
    {
        return false;
    }

    if (!m_fileSystem->isDirectory(buildSourcePath(virtualPath)))
    {
        return false;
    }

    m_workingDirectory = std::move(virtualPath);
    return true;
}

ego::FileName ego::RootedFileSystem::getAbsolutePath(const FileName& _path) const
{
    if (!m_isInitialized)
    {
        return FileName();
    }

    FileName sourcePath;
    return resolveSourcePath(_path, sourcePath) ? m_fileSystem->getAbsolutePath(sourcePath) : FileName();
}

bool ego::RootedFileSystem::getEntryInfo(const FileName& _path, FileSystemEntryDesc& _entry) const
{
    FileName sourcePath;
    std::string virtualPath;
    if (!m_isInitialized || !resolveSourcePath(_path, sourcePath) || !resolveVirtualPath(_path, virtualPath))
    {
        _entry = FileSystemEntryDesc();
        return false;
    }

    if (!m_fileSystem->getEntryInfo(sourcePath, _entry))
    {
        return false;
    }

    _entry.m_path = ToFileName(virtualPath);
    return true;
}

bool ego::RootedFileSystem::enumerate(const FileName& _directoryPath, FileSystemEntryCollection& _entries, bool _recursive) const
{
    _entries.clear();

    FileName sourcePath;
    if (!m_isInitialized || !resolveSourcePath(_directoryPath, sourcePath))
    {
        return false;
    }

    if (!m_fileSystem->enumerate(sourcePath, _entries, _recursive))
    {
        _entries.clear();
        return false;
    }

    for (FileSystemEntryDesc& entry : _entries)
    {
        remapEntryPath(entry);
    }

    return true;
}

bool ego::RootedFileSystem::createDirectory(const FileName& _path, bool _recursive)
{
    FileName sourcePath;
    return m_isInitialized && resolveSourcePath(_path, sourcePath) && m_fileSystem->createDirectory(sourcePath, _recursive);
}

bool ego::RootedFileSystem::removeFile(const FileName& _path)
{
    FileName sourcePath;
    return m_isInitialized && resolveSourcePath(_path, sourcePath) && m_fileSystem->removeFile(sourcePath);
}

bool ego::RootedFileSystem::removeDirectory(const FileName& _path, bool _recursive)
{
    FileName sourcePath;
    return m_isInitialized && resolveSourcePath(_path, sourcePath) && m_fileSystem->removeDirectory(sourcePath, _recursive);
}

bool ego::RootedFileSystem::remove(const FileName& _path, bool _recursive)
{
    FileName sourcePath;
    return m_isInitialized && resolveSourcePath(_path, sourcePath) && m_fileSystem->remove(sourcePath, _recursive);
}

bool ego::RootedFileSystem::copyFile(const FileName& _sourcePath, const FileName& _destinationPath, bool _overwrite)
{
    FileName sourcePath;
    FileName destinationPath;
    return m_isInitialized && resolveSourcePath(_sourcePath, sourcePath) && resolveSourcePath(_destinationPath, destinationPath) &&
           m_fileSystem->copyFile(sourcePath, destinationPath, _overwrite);
}

bool ego::RootedFileSystem::move(const FileName& _sourcePath, const FileName& _destinationPath, bool _overwrite)
{
    FileName sourcePath;
    FileName destinationPath;
    return m_isInitialized && resolveSourcePath(_sourcePath, sourcePath) && resolveSourcePath(_destinationPath, destinationPath) &&
           m_fileSystem->move(sourcePath, destinationPath, _overwrite);
}

bool ego::RootedFileSystem::readFile(const FileName& _path, FileContent& _content) const
{
    FileName sourcePath;
    return m_isInitialized && resolveSourcePath(_path, sourcePath) && m_fileSystem->readFile(sourcePath, _content);
}

bool ego::RootedFileSystem::readTextFile(const FileName& _path, std::string& _content) const
{
    FileName sourcePath;
    return m_isInitialized && resolveSourcePath(_path, sourcePath) && m_fileSystem->readTextFile(sourcePath, _content);
}

bool ego::RootedFileSystem::writeFile(const FileName& _path, const FileContent& _content, bool _overwrite)
{
    FileName sourcePath;
    return m_isInitialized && resolveSourcePath(_path, sourcePath) && m_fileSystem->writeFile(sourcePath, _content, _overwrite);
}

bool ego::RootedFileSystem::writeTextFile(const FileName& _path, const std::string& _content, bool _overwrite)
{
    FileName sourcePath;
    return m_isInitialized && resolveSourcePath(_path, sourcePath) && m_fileSystem->writeTextFile(sourcePath, _content, _overwrite);
}

bool ego::RootedFileSystem::appendFile(const FileName& _path, const FileContent& _content)
{
    FileName sourcePath;
    return m_isInitialized && resolveSourcePath(_path, sourcePath) && m_fileSystem->appendFile(sourcePath, _content);
}

bool ego::RootedFileSystem::appendTextFile(const FileName& _path, const std::string& _content)
{
    FileName sourcePath;
    return m_isInitialized && resolveSourcePath(_path, sourcePath) && m_fileSystem->appendTextFile(sourcePath, _content);
}

bool ego::RootedFileSystem::IsPathSeparator(char _ch)
{
    return _ch == '/' || _ch == '\\';
}

size_t ego::RootedFileSystem::GetPathRootLength(const std::string& _path)
{
    if (_path.empty())
    {
        return 0;
    }

    if (_path.size() >= 2 && IsPathSeparator(_path[0]) && IsPathSeparator(_path[1]))
    {
        const size_t serverEnd = _path.find_first_of("/\\", 2);
        if (serverEnd == std::string::npos)
        {
            return _path.size();
        }

        const size_t shareEnd = _path.find_first_of("/\\", serverEnd + 1);
        return shareEnd == std::string::npos ? _path.size() : shareEnd + 1;
    }

    if (_path.size() >= 2 && _path[1] == ':')
    {
        return _path.size() >= 3 && IsPathSeparator(_path[2]) ? 3 : 2;
    }

    return IsPathSeparator(_path[0]) ? 1 : 0;
}

std::string ego::RootedFileSystem::NormalizeRootPath(const FileName& _path)
{
    std::string result(_path.getView());
    std::replace(result.begin(), result.end(), '\\', '/');

    const size_t rootLength = GetPathRootLength(result);
    while (result.size() > rootLength && IsPathSeparator(result.back()))
    {
        result.pop_back();
    }

    return result;
}

ego::FileName ego::RootedFileSystem::ToFileName(const std::string& _path)
{
    return FileName(_path);
}

bool ego::RootedFileSystem::resolveVirtualPath(const FileName& _path, std::string& _virtualPath) const
{
    _virtualPath.clear();

    const std::string path(_path.getView());
    const bool isAbsolute = !path.empty() && IsPathSeparator(path.front());

    std::vector<std::string> parts;
    if (!isAbsolute && !m_workingDirectory.empty())
    {
        size_t position = 0;
        while (position < m_workingDirectory.size())
        {
            const size_t separatorPosition = m_workingDirectory.find('/', position);
            const size_t partEnd = separatorPosition == std::string::npos ? m_workingDirectory.size() : separatorPosition;

            parts.push_back(m_workingDirectory.substr(position, partEnd - position));

            if (separatorPosition == std::string::npos)
            {
                break;
            }

            position = separatorPosition + 1;
        }
    }

    size_t position = 0;
    while (position < path.size())
    {
        while (position < path.size() && IsPathSeparator(path[position]))
        {
            ++position;
        }

        const size_t partStart = position;
        while (position < path.size() && !IsPathSeparator(path[position]))
        {
            ++position;
        }

        if (partStart == position)
        {
            continue;
        }

        const std::string part = path.substr(partStart, position - partStart);
        if (part == ".")
        {
            continue;
        }

        if (part == "..")
        {
            if (parts.empty())
            {
                return false;
            }

            parts.pop_back();
            continue;
        }

        parts.push_back(part);
    }

    for (size_t index = 0; index < parts.size(); ++index)
    {
        if (index > 0)
        {
            _virtualPath += '/';
        }

        _virtualPath += parts[index];
    }

    return true;
}

bool ego::RootedFileSystem::resolveSourcePath(const FileName& _path, FileName& _sourcePath) const
{
    std::string virtualPath;
    if (!resolveVirtualPath(_path, virtualPath))
    {
        _sourcePath = FileName();
        return false;
    }

    _sourcePath = buildSourcePath(virtualPath);
    return true;
}

ego::FileName ego::RootedFileSystem::buildSourcePath(const std::string& _virtualPath) const
{
    if (!m_rootPath)
    {
        return ToFileName(_virtualPath);
    }

    if (_virtualPath.empty())
    {
        return m_rootPath;
    }

    std::string sourcePath(m_rootPath.getView());
    if (!sourcePath.empty() && !IsPathSeparator(sourcePath.back()))
    {
        sourcePath += '/';
    }

    sourcePath += _virtualPath;
    return ToFileName(sourcePath);
}

ego::FileName ego::RootedFileSystem::buildVirtualPath(const FileName& _sourcePath) const
{
    if (!m_rootPath)
    {
        return _sourcePath;
    }

    const std::string rootAbsolute = NormalizeRootPath(m_fileSystem->getAbsolutePath(m_rootPath));
    const std::string sourceAbsolute = NormalizeRootPath(m_fileSystem->getAbsolutePath(_sourcePath));
    const std::string rootPath = rootAbsolute.empty() ? NormalizeRootPath(m_rootPath) : rootAbsolute;
    const std::string sourcePath = sourceAbsolute.empty() ? NormalizeRootPath(_sourcePath) : sourceAbsolute;

    if (sourcePath == rootPath)
    {
        return FileName();
    }

    const std::string rootPrefix = rootPath + '/';
    if (sourcePath.size() > rootPrefix.size() && sourcePath.compare(0, rootPrefix.size(), rootPrefix) == 0)
    {
        return ToFileName(sourcePath.substr(rootPrefix.size()));
    }

    return _sourcePath;
}

void ego::RootedFileSystem::remapEntryPath(FileSystemEntryDesc& _entry) const
{
    _entry.m_path = buildVirtualPath(_entry.m_path);
}
