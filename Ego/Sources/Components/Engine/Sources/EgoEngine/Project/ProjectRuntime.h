#pragma once

#include <string_view>
#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoPlugin/Catalog/PluginCatalog.h"

#include "EngineLogicPlugin.h"
#include "Project.h"

namespace ego
{
    class FileSystem;
    class PluginController;
    class ResourceController;

    EGO_POINTER(FileSystem);
    EGO_POINTER(PluginController);
    EGO_POINTER(ResourceController);
} // namespace ego

namespace ego::engine
{
    class ProjectRuntime final : public NonCopyable
    {
    public:
        ProjectRuntime() = default;
        ~ProjectRuntime() override;

        bool init(const ProjectPointer& _project);
        void release();

        FileName resolvePluginModuleName(PluginType _pluginType) const;

        EngineLogicPluginPointer getEngineLogicPluginPointer() const;

    private:
        FileName resolvePluginModuleName(PluginType _pluginType, std::string_view _pluginName) const;

        FileSystemPointer getFileSystemPointer() const;
        PluginControllerPointer getPluginControllerPointer() const;
        ResourceControllerPointer getResourceControllerPointer() const;

        bool buildPluginCatalog(const Project& _project);
        bool registerAssetFileSystems(const Project& _project);
        void releaseAssetFileSystems();
        FileSystemPointer createAssetFileSystem(const FileSystemPointer& _sourceFileSystem, const FileName& _rootPath) const;

        bool loadPlugins(const Project& _project);
        bool loadPlugin(const Project::PluginDesc& _pluginDesc);
        bool loadEngineLogicPlugin(const Project& _project);
        FileName resolveProjectPluginModuleName(const Project::PluginDesc& _pluginDesc) const;

        using AssetFileSystemCollection = std::vector<FileSystemPointer>;
        using PluginCollection = std::vector<PluginPointer>;

        AssetFileSystemCollection m_assetFileSystems;
        PluginCollection m_plugins;
        PluginCatalog m_pluginCatalog;
        EngineLogicPluginPointer m_engineLogicPlugin = nullptr;
    };
} // namespace ego::engine
