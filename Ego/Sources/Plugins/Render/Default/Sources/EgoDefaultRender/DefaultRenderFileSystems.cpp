#include "DefaultRenderFileSystems.h"

#include <string>

#include "EgoCore/Context/ContextStack.h"
#include "EgoCore/Context/PlatformContext.h"
#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoRuntime/Plugin/PluginModule.h"
#include "EgoRuntime/Resource/ResourceController.h"
#include "EgoRuntime/RuntimeContext.h"

#include "EgoEngine/Resources/GeneralResources/XmlResource.h"

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

    ResourceController& resourceController = context::GetRuntimeContext().getResourceController();
    resourceController.addFileSystem(configFileSystem);

    const XmlResourcePointer configResource = resourceController.load<XmlResource>(DefaultRenderConfigPath);
    const bool loadResult = configResource && configResource->isLoaded() && ParseDefaultRenderAssetsRootPath(configResource->getRootNode(), _assetsRootPath);

    resourceController.removeFileSystem(configFileSystem);
    configFileSystem->release();
    return loadResult;
}

bool ego::render::DefaultRenderFileSystems::initAssetsFileSystem(const FileName& _assetsRootPath)
{
    EGO_CHECK_RETURN_FALSE(_assetsRootPath);

    ResourceController& resourceController = context::GetRuntimeContext().getResourceController();
    if (m_assetsFileSystem)
    {
        resourceController.removeFileSystem(m_assetsFileSystem);
        m_assetsFileSystem->release();
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
    const context::RuntimeContextPointer runtimeContext = context::GetRuntimeContextPointer();
    const ResourceControllerPointer resourceController = runtimeContext ? runtimeContext->getResourceControllerPointer() : nullptr;
    if (!m_assetsFileSystem)
    {
        return;
    }

    if (resourceController)
    {
        resourceController->removeFileSystem(m_assetsFileSystem);
    }

    m_assetsFileSystem->release();
    m_assetsFileSystem = nullptr;
}

ego::RootedFileSystemPointer ego::render::DefaultRenderFileSystems::CreateFileSystem(const FileName& _rootPath)
{
    const PlatformPointer platform = context::GetPlatformPointer();
    EGO_CHECK_RETURN_NULL(platform);

    const FileSystemPointer sourceFileSystem = platform->getFileSystem();
    EGO_CHECK_RETURN_NULL(sourceFileSystem);

    RootedFileSystemPointer fileSystem = new RootedFileSystem(sourceFileSystem, _rootPath);
    if (!fileSystem)
    {
        return nullptr;
    }

    if (!fileSystem->init())
    {
        fileSystem->release();
        return nullptr;
    }

    return fileSystem;
}
