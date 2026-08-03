#include "DefaultGuiRenderFileSystems.h"

#include <filesystem>
#include <string>

#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoPlugin/PluginModule.h"

#include "EgoResource/GeneralResources/XmlResource.h"
#include "EgoResource/ResourceController.h"
#include "EgoResource/ResourceSubsystem.h"

#include "DefaultGuiRenderConstants.h"

namespace ego::gui::default_gui_render
{
    static bool ParseDefaultGuiRenderAssetsRootPath(const XmlNode& _configNode, FileName& _assetsRootPath)
    {
        EGO_CHECK_RETURN_FALSE(_configNode && _configNode.getNameView() == "DefaultGuiRender");

        const std::string assetsRootPath = _configNode.getChildValueOr<std::string>("AssetsDir", std::string());
        EGO_CHECK_RETURN_FALSE(!assetsRootPath.empty());

        _assetsRootPath = FileName(assetsRootPath);
        return static_cast<bool>(_assetsRootPath);
    }
} // namespace ego::gui::default_gui_render

ego::gui::default_gui_render::DefaultGuiRenderFileSystems::~DefaultGuiRenderFileSystems()
{
    release();
}

bool ego::gui::default_gui_render::DefaultGuiRenderFileSystems::loadAssetsRootPath(FileName& _assetsRootPath)
{
    const PluginModuleInfo& moduleInfo = GetPluginModuleState().getInfo();
    const FileName pluginDirectoryPath = file_name_utils::GetFileDirPath(moduleInfo.m_modulePath);
    EGO_CHECK_RETURN_FALSE(pluginDirectoryPath);

    RootedFileSystemPointer configFileSystem = CreateFileSystem(pluginDirectoryPath);
    EGO_CHECK_RETURN_FALSE(configFileSystem);

    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    const ResourceControllerPointer resourceController = resourceSubsystem ? resourceSubsystem->getResourceControllerPointer() : nullptr;
    if (!resourceController)
    {
        return false;
    }

    resourceController->addFileSystem(configFileSystem);

    const XmlResourcePointer configResource = resourceController->load<XmlResource>(DefaultGuiRenderConfigPath);
    const bool loadResult = configResource && configResource->isLoaded() && ParseDefaultGuiRenderAssetsRootPath(configResource->getRootNode(), _assetsRootPath);

    resourceController->removeFileSystem(configFileSystem);
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

bool ego::gui::default_gui_render::DefaultGuiRenderFileSystems::initAssetsFileSystem(const FileName& _assetsRootPath)
{
    EGO_CHECK_RETURN_FALSE(_assetsRootPath);

    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    const ResourceControllerPointer resourceController = resourceSubsystem ? resourceSubsystem->getResourceControllerPointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(resourceController);

    if (m_assetsFileSystem)
    {
        resourceController->removeFileSystem(m_assetsFileSystem);
        m_assetsFileSystem = nullptr;
    }

    RootedFileSystemPointer assetsFileSystem = CreateFileSystem(_assetsRootPath);
    EGO_CHECK_RETURN_FALSE(assetsFileSystem);

    resourceController->addFileSystem(assetsFileSystem);
    m_assetsFileSystem = assetsFileSystem;
    return true;
}

void ego::gui::default_gui_render::DefaultGuiRenderFileSystems::release()
{
    if (!m_assetsFileSystem)
    {
        return;
    }

    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    const ResourceControllerPointer resourceController = resourceSubsystem ? resourceSubsystem->getResourceControllerPointer() : nullptr;
    if (resourceController)
    {
        resourceController->removeFileSystem(m_assetsFileSystem);
    }

    m_assetsFileSystem = nullptr;
}

ego::RootedFileSystemPointer ego::gui::default_gui_render::DefaultGuiRenderFileSystems::CreateFileSystem(const FileName& _rootPath)
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
