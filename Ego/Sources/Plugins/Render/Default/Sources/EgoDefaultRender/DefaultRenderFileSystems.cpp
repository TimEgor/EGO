#include "DefaultRenderFileSystems.h"

#include <filesystem>
#include <string>

#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoPlugin/PluginModule.h"

#include "EgoResource/GeneralResources/XmlResource.h"
#include "EgoResource/ResourceSubsystem.h"
#include "EgoResource/ResourceController.h"

#include "DefaultRenderConstants.h"

namespace ego::render
{
    static bool ParseDefaultRenderAssetsRootPath(const XmlNode& _configNode, FileName& _assetsRootPath)
    {
        EGO_CHECK_RETURN_FALSE(_configNode && _configNode.getNameView() == "DefaultRender");

        const std::string assetsRootPath = _configNode.getChildValueOr<std::string>("AssetsDir", std::string());
        EGO_CHECK_RETURN_FALSE(!assetsRootPath.empty());

        _assetsRootPath = FileName(assetsRootPath);
        return static_cast<bool>(_assetsRootPath);
    }
} // namespace ego::render

ego::render::DefaultRenderFileSystems::~DefaultRenderFileSystems()
{
    release();
}

bool ego::render::DefaultRenderFileSystems::loadAssetsRootPath(FileName& _assetsRootPath)
{
    const PluginModuleInfo& moduleInfo = GetPluginModuleState().getInfo();
    const FileName pluginDirectoryPath = file_name_utils::GetFileDirPath(moduleInfo.m_modulePath);
    EGO_CHECK_RETURN_FALSE(pluginDirectoryPath);

    RootedFileSystemPointer configFileSystem = CreateFileSystem(pluginDirectoryPath);
    EGO_CHECK_RETURN_FALSE(configFileSystem);

    ResourceController& resourceController = GetResourceSubsystem().getResourceController();
    resourceController.addFileSystem(configFileSystem);

    const XmlResourcePointer configResource = resourceController.load<XmlResource>(DefaultRenderConfigPath);
    const bool loadResult = configResource && configResource->isLoaded() && ParseDefaultRenderAssetsRootPath(configResource->getRootNode(), _assetsRootPath);

    resourceController.removeFileSystem(configFileSystem);
    if (!loadResult)
    {
        return false;
    }

    std::filesystem::path assetsRootPath(_assetsRootPath.c_str());
    if (assetsRootPath.is_relative())
    {
        assetsRootPath = std::filesystem::path(pluginDirectoryPath.c_str()) / assetsRootPath;
        _assetsRootPath = assetsRootPath.lexically_normal().string();
    }

    return static_cast<bool>(_assetsRootPath);
}

bool ego::render::DefaultRenderFileSystems::initAssetsFileSystem(const FileName& _assetsRootPath)
{
    EGO_CHECK_RETURN_FALSE(_assetsRootPath);

    ResourceController& resourceController = GetResourceSubsystem().getResourceController();
    if (m_assetsFileSystem)
    {
        resourceController.removeFileSystem(m_assetsFileSystem);
        m_assetsFileSystem = nullptr;
    }

    RootedFileSystemPointer assetsFileSystem = CreateFileSystem(_assetsRootPath);
    EGO_CHECK_RETURN_FALSE(assetsFileSystem);

    resourceController.addFileSystem(assetsFileSystem);
    m_assetsFileSystem = assetsFileSystem;
    return true;
}

void ego::render::DefaultRenderFileSystems::release()
{
    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    const ResourceControllerPointer resourceController = resourceSubsystem ? resourceSubsystem->getResourceControllerPointer() : nullptr;
    if (!m_assetsFileSystem)
    {
        return;
    }

    if (resourceController)
    {
        resourceController->removeFileSystem(m_assetsFileSystem);
    }

    m_assetsFileSystem = nullptr;
}

ego::RootedFileSystemPointer ego::render::DefaultRenderFileSystems::CreateFileSystem(const FileName& _rootPath)
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN_NULL(platform);

    const FileSystemPointer sourceFileSystem = platform->getFileSystem();
    EGO_CHECK_RETURN_NULL(sourceFileSystem);

    RootedFileSystemPointer fileSystem =
        MakePointer<RootedFileSystem>(sourceFileSystem, _rootPath);
    if (!fileSystem)
    {
        return nullptr;
    }

    if (!fileSystem->init())
    {
        return nullptr;
    }

    return fileSystem;
}
