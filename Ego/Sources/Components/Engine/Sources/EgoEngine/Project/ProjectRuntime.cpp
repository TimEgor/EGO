#include "ProjectRuntime.h"

#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/Platform/FileSystem/RootedFileSystem.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoPlugin/Catalog/PluginCatalogBuilder.h"
#include "EgoPlugin/PluginController.h"
#include "EgoPlugin/PluginSubsystem.h"

#include "EgoResource/ResourceController.h"
#include "EgoResource/ResourceSubsystem.h"

ego::engine::ProjectRuntime::~ProjectRuntime()
{
    release();
}

bool ego::engine::ProjectRuntime::init(const ProjectPointer& _project)
{
    release();

    const PluginSubsystemPointer pluginSubsystem = subsystem::FindSubsystem<PluginSubsystem>();
    EGO_CHECK_INITIALIZATION(pluginSubsystem);

    if (!_project)
    {
        return true;
    }

    EGO_CHECK_INITIALIZATION(buildPluginCatalog(*_project));
    EGO_CHECK_INITIALIZATION(registerAssetFileSystems(*_project));
    EGO_CHECK_INITIALIZATION(loadPlugins(*_project));
    EGO_CHECK_INITIALIZATION(loadEngineLogicPlugin(*_project));

    return true;
}

void ego::engine::ProjectRuntime::release()
{
    if (m_engineLogicPlugin || !m_plugins.empty() || !m_assetFileSystems.empty())
    {
        const ResourceControllerPointer resourceController = getResourceControllerPointer();
        if (resourceController)
        {
            resourceController->waitAllLoading();
        }
    }

    m_engineLogicPlugin = nullptr;

    while (!m_plugins.empty())
    {
        m_plugins.pop_back();
    }

    releaseAssetFileSystems();
    m_pluginCatalog.clear();
}

ego::FileName ego::engine::ProjectRuntime::resolvePluginModuleName(PluginType _pluginType) const
{
    const FileName projectModuleName = m_pluginCatalog.resolve(_pluginType);
    if (projectModuleName)
    {
        return projectModuleName;
    }

    const PluginSubsystemPointer pluginSubsystem = subsystem::FindSubsystem<PluginSubsystem>();
    return pluginSubsystem ? pluginSubsystem->getPluginCatalog().resolve(_pluginType) : FileName();
}

ego::FileName ego::engine::ProjectRuntime::resolvePluginModuleName(PluginType _pluginType, std::string_view _pluginName) const
{
    const FileName projectModuleName = m_pluginCatalog.resolve(_pluginType, _pluginName);
    if (projectModuleName)
    {
        return projectModuleName;
    }

    const PluginSubsystemPointer pluginSubsystem = subsystem::FindSubsystem<PluginSubsystem>();
    return pluginSubsystem ? pluginSubsystem->getPluginCatalog().resolve(_pluginType, _pluginName) : FileName();
}

ego::engine::EngineLogicPluginPointer ego::engine::ProjectRuntime::getEngineLogicPluginPointer() const
{
    return m_engineLogicPlugin;
}

ego::FileSystemPointer ego::engine::ProjectRuntime::getFileSystemPointer() const
{
    const PlatformSubsystemPointer platformSubsystem = subsystem::FindSubsystem<PlatformSubsystem>();
    const PlatformPointer platform = platformSubsystem ? platformSubsystem->getPlatformPointer() : nullptr;
    return platform ? platform->getFileSystem() : nullptr;
}

ego::PluginControllerPointer ego::engine::ProjectRuntime::getPluginControllerPointer() const
{
    const PluginSubsystemPointer pluginSubsystem = subsystem::FindSubsystem<PluginSubsystem>();
    return pluginSubsystem ? pluginSubsystem->getPluginControllerPointer() : nullptr;
}

ego::ResourceControllerPointer ego::engine::ProjectRuntime::getResourceControllerPointer() const
{
    const ResourceSubsystemPointer resourceSubsystem = subsystem::FindSubsystem<ResourceSubsystem>();
    return resourceSubsystem ? resourceSubsystem->getResourceControllerPointer() : nullptr;
}

bool ego::engine::ProjectRuntime::buildPluginCatalog(const Project& _project)
{
    const FileSystemPointer fileSystem = getFileSystemPointer();
    EGO_CHECK_RETURN_FALSE(fileSystem);

    for (const FileName& pluginDirectory : _project.getPluginDirectories())
    {
        PluginCatalogBuilder::Options options;
        options.m_mode = PluginCatalogBuilder::Mode::BestEffort;
        const PluginCatalog::RegistrationID registrationID = PluginCatalogBuilder::AddPluginsFromPath(m_pluginCatalog, *fileSystem, pluginDirectory, options);
        EGO_CHECK_RETURN_FALSE(registrationID != PluginCatalog::InvalidRegistrationID);
    }

    return true;
}

bool ego::engine::ProjectRuntime::registerAssetFileSystems(const Project& _project)
{
    const ResourceControllerPointer resourceController = getResourceControllerPointer();
    const FileSystemPointer sourceFileSystem = getFileSystemPointer();
    EGO_CHECK_RETURN_FALSE(resourceController && sourceFileSystem);

    for (const FileName& assetDirectory : _project.getAssetDirectories())
    {
        FileSystemPointer assetFileSystem = createAssetFileSystem(sourceFileSystem, assetDirectory);
        EGO_CHECK_RETURN_FALSE(assetFileSystem);

        resourceController->addFileSystem(assetFileSystem);
        m_assetFileSystems.push_back(assetFileSystem);
    }

    return true;
}

void ego::engine::ProjectRuntime::releaseAssetFileSystems()
{
    const ResourceControllerPointer resourceController = getResourceControllerPointer();

    while (!m_assetFileSystems.empty())
    {
        const FileSystemPointer assetFileSystem = m_assetFileSystems.back();
        m_assetFileSystems.pop_back();
        if (!assetFileSystem)
        {
            continue;
        }

        if (resourceController)
        {
            resourceController->removeFileSystem(assetFileSystem);
        }

        assetFileSystem->release();
    }
}

ego::FileSystemPointer ego::engine::ProjectRuntime::createAssetFileSystem(const FileSystemPointer& _sourceFileSystem, const FileName& _rootPath) const
{
    RootedFileSystemPointer fileSystem =
        MakePointer<RootedFileSystem>(_sourceFileSystem, _rootPath);
    return fileSystem && fileSystem->init() ? fileSystem : nullptr;
}

bool ego::engine::ProjectRuntime::loadPlugins(const Project& _project)
{
    for (const Project::PluginDesc& pluginDesc : _project.getPlugins())
    {
        EGO_CHECK_RETURN_FALSE(loadPlugin(pluginDesc));
    }

    return true;
}

bool ego::engine::ProjectRuntime::loadPlugin(const Project::PluginDesc& _pluginDesc)
{
    const FileName moduleName = resolveProjectPluginModuleName(_pluginDesc);
    EGO_CHECK_RETURN_FALSE(moduleName);

    const PluginControllerPointer pluginController = getPluginControllerPointer();
    EGO_CHECK_RETURN_FALSE(pluginController);

    const PluginPointer plugin = pluginController->loadPlugin(moduleName, _pluginDesc.m_typeName.c_str());
    EGO_CHECK_RETURN_FALSE(plugin);

    m_plugins.push_back(plugin);
    return true;
}

bool ego::engine::ProjectRuntime::loadEngineLogicPlugin(const Project& _project)
{
    if (!_project.getEngineLogicPlugin().has_value())
    {
        return true;
    }

    const Project::PluginDesc& pluginDesc = _project.getEngineLogicPlugin().value();
    const FileName moduleName = resolveProjectPluginModuleName(pluginDesc);
    EGO_CHECK_RETURN_FALSE(moduleName);

    const PluginControllerPointer pluginController = getPluginControllerPointer();
    EGO_CHECK_RETURN_FALSE(pluginController);

    m_engineLogicPlugin = pluginController->loadPlugin<EngineLogicPlugin>(moduleName);
    return static_cast<bool>(m_engineLogicPlugin);
}

ego::FileName ego::engine::ProjectRuntime::resolveProjectPluginModuleName(const Project::PluginDesc& _pluginDesc) const
{
    if (_pluginDesc.m_moduleName)
    {
        return _pluginDesc.m_moduleName;
    }

    if (_pluginDesc.m_typeName.empty() || _pluginDesc.m_name.empty())
    {
        return FileName();
    }

    const PluginType pluginType = GetPluginType(_pluginDesc.m_typeName.c_str());
    return resolvePluginModuleName(pluginType, _pluginDesc.m_name);
}
