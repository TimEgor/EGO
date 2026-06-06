#include <algorithm>
#include <cctype>
#include <mutex>
#include <string>

#include "EgoCore/FileName/FileNameUtils.h"

#include "ResourceSourceRegistry.h"

namespace
{
    std::string NormalizeProviderExtension(const ego::FileName& _extension)
    {
        std::string extension = _extension.c_str();
        if (extension.empty())
        {
            return extension;
        }

        if (extension.front() != '.')
        {
            extension.insert(extension.begin(), '.');
        }

        std::transform(
            extension.begin(),
            extension.end(),
            extension.begin(),
            [](unsigned char _ch)
            {
                return static_cast<char>(std::tolower(_ch));
            }
        );
        return extension;
    }

    std::string GetProviderExtension(const ego::FileName& _path)
    {
        return NormalizeProviderExtension(ego::file_name_utils::GetFileExtension(_path));
    }
}

void ego::ResourceSourceRegistry::addFileSystem(const FileSystemPointer& _fileSystem)
{
    if (!_fileSystem)
    {
        return;
    }

    std::lock_guard locker(m_mutex);

    const auto foundIt = std::find_if(
        m_fileSystems.begin(),
        m_fileSystems.end(),
        [&_fileSystem](const FileSystemPointer& _registeredFileSystem)
        {
            return _registeredFileSystem.get() == _fileSystem.get();
        }
    );
    if (foundIt != m_fileSystems.end())
    {
        return;
    }

    m_fileSystems.push_back(_fileSystem);
}

bool ego::ResourceSourceRegistry::removeFileSystem(const FileSystemPointer& _fileSystem)
{
    if (!_fileSystem)
    {
        return false;
    }

    std::lock_guard locker(m_mutex);

    const auto foundIt = std::find_if(
        m_fileSystems.begin(),
        m_fileSystems.end(),
        [&_fileSystem](const FileSystemPointer& _registeredFileSystem)
        {
            return _registeredFileSystem.get() == _fileSystem.get();
        }
    );
    if (foundIt == m_fileSystems.end())
    {
        return false;
    }

    m_fileSystems.erase(foundIt);
    return true;
}

void ego::ResourceSourceRegistry::clearFileSystems()
{
    std::lock_guard locker(m_mutex);
    m_fileSystems.clear();
}

bool ego::ResourceSourceRegistry::addResourceProvider(
    const FileName& _extension,
    const ResourceProviderPointer& _provider
)
{
    const std::string extension = NormalizeProviderExtension(_extension);
    if (extension.empty() || !_provider)
    {
        return false;
    }

    std::lock_guard locker(m_mutex);

    auto [providerIt, added] = m_resourceProviders.emplace(extension, _provider);
    if (!added)
    {
        providerIt->second = _provider;
    }

    return true;
}

bool ego::ResourceSourceRegistry::removeResourceProvider(const FileName& _extension)
{
    const std::string extension = NormalizeProviderExtension(_extension);
    if (extension.empty())
    {
        return false;
    }

    std::lock_guard locker(m_mutex);
    return m_resourceProviders.erase(extension) != 0;
}

void ego::ResourceSourceRegistry::clearResourceProviders()
{
    std::lock_guard locker(m_mutex);
    m_resourceProviders.clear();
}

bool ego::ResourceSourceRegistry::loadContent(const FileName& _path, FileContent& _content) const
{
    FileSystemCollection fileSystems;

    {
        std::lock_guard locker(m_mutex);
        fileSystems = m_fileSystems;
    }

    for (const FileSystemPointer& fileSystem : fileSystems)
    {
        if (fileSystem && fileSystem->readFile(_path, _content))
        {
            return true;
        }
    }

    _content.clear();
    return false;
}

ego::ResourceProviderPointer ego::ResourceSourceRegistry::getResourceProvider(const FileName& _path) const
{
    const std::string extension = GetProviderExtension(_path);
    if (extension.empty())
    {
        return nullptr;
    }

    std::lock_guard locker(m_mutex);

    const auto providerIt = m_resourceProviders.find(extension);
    return providerIt != m_resourceProviders.end() ? providerIt->second : nullptr;
}

void ego::ResourceSourceRegistry::clear()
{
    std::lock_guard locker(m_mutex);
    m_fileSystems.clear();
    m_resourceProviders.clear();
}
