#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "EgoEngine/Platform/FileSystem/FileSystem.h"

#include "ResourceProvider.h"

namespace ego
{
    class ResourceSourceRegistry final
    {
    public:
        void addFileSystem(const FileSystemPointer& _fileSystem);
        bool removeFileSystem(const FileSystemPointer& _fileSystem);
        void clearFileSystems();

        bool addResourceProvider(const FileName& _extension, const ResourceProviderPointer& _provider);
        bool removeResourceProvider(const FileName& _extension);
        void clearResourceProviders();

        bool loadContent(const FileName& _path, FileContent& _content) const;
        ResourceProviderPointer getResourceProvider(const FileName& _path) const;
        void clear();

    private:
        using FileSystemCollection = std::vector<FileSystemPointer>;
        using ResourceProviderCollection = std::unordered_map<std::string, ResourceProviderPointer>;

        mutable std::mutex m_mutex;
        FileSystemCollection m_fileSystems;
        ResourceProviderCollection m_resourceProviders;
    };
} // namespace ego
