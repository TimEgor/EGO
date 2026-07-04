#include "EngineFramework.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Context/ContextStack.h"
#include "EgoCore/Context/PlatformContext.h"
#include "EgoCore/Platform/FileSystem/RootedFileSystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoRuntime/Job/JobDescriptor.h"
#include "EgoRuntime/Resource/ResourceController.h"
#include "EgoRuntime/RuntimeContext.h"

bool ego::engine_framework::EngineFramework::init(const EngineFramework::InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(initEngine(_initData));
    EGO_CHECK_INITIALIZATION(initProject(_initData));
    EGO_CHECK_INITIALIZATION(registerProjectAssetFileSystems());

    EGO_CHECK_INITIALIZATION(loadProjectPlugins());
    EGO_CHECK_INITIALIZATION(loadProjectEngineLogic());

    return true;
}

void ego::engine_framework::EngineFramework::release()
{
    m_updateEngineLogicJobID = JobDescriptorID();

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_currentEngineLogic);
    m_currentEngineLogicPlugin = nullptr;

    m_plugins.clear();

    releaseProjectAssetFileSystems();

    releaseEngine();

    m_project = nullptr;
}

void ego::engine_framework::EngineFramework::run()
{
    getEngine().run();
}

bool ego::engine_framework::EngineFramework::runFrame()
{
    return getEngine().runFrame();
}

ego::engine::Engine& ego::engine_framework::EngineFramework::getEngine() const
{
    EGO_ASSERT(m_engineContext);
    return m_engineContext->getEngine();
}

const ego::engine_framework::Project& ego::engine_framework::EngineFramework::getProject() const
{
    EGO_ASSERT(m_project);
    return *m_project;
}

ego::engine_framework::EngineLogic& ego::engine_framework::EngineFramework::getCurrentEngineLogic() const
{
    EGO_ASSERT(m_currentEngineLogic);
    return *m_currentEngineLogic;
}

bool ego::engine_framework::EngineFramework::initEngine(const EngineFramework::InitData& _initData)
{
    const context::ContextScopePointer contextScope = context::ContextStackCore::GetInstance().getCurrentScope();
    EGO_ASSERT(contextScope);
    EGO_CHECK_RETURN_FALSE(contextScope);

    m_engineContext = new engine::EngineContext();
    EGO_CHECK_RETURN_FALSE(m_engineContext);

    contextScope->addContext(m_engineContext);

    engine::EngineContext::InitData contextInitData;
    contextInitData.m_engineInitData = _initData.m_engineInitData;
    EGO_CHECK_RETURN_FALSE(m_engineContext->init(contextInitData));

    EGO_CHECK_RETURN_FALSE(m_engineContext->getEnginePointer());
    EGO_CHECK_RETURN_FALSE(registerEngineLogicMainLoopJob());

    return true;
}

bool ego::engine_framework::EngineFramework::initProject(const EngineFramework::InitData& _initData)
{
    m_project = _initData.m_project;
    return true;
}

bool ego::engine_framework::EngineFramework::registerEngineLogicMainLoopJob()
{
    engine::Engine& currentEngine = getEngine();

    engine::MainLoop& mainLoop = currentEngine.getMainLoop();
    const JobDescriptorID frameLogicBeginJobID = mainLoop.getFrameLogicBeginJobID();
    const JobDescriptorID frameLogicEndJobID = mainLoop.getFrameLogicEndJobID();
    EGO_CHECK_RETURN_FALSE(frameLogicBeginJobID.isValid());
    EGO_CHECK_RETURN_FALSE(frameLogicEndJobID.isValid());

    m_updateEngineLogicJobID = mainLoop.addJobBetween(
        CreateJobDescriptor(
            [this]()
            {
                updateCurrentEngineLogic(getEngine().getDeltaTime());
            },
            "Engine logic update"),
        frameLogicBeginJobID,
        frameLogicEndJobID);

    return m_updateEngineLogicJobID.isValid();
}

void ego::engine_framework::EngineFramework::releaseEngine()
{
    if (m_engineContext)
    {
        m_engineContext->release();
    }

    const context::ContextScopePointer contextScope = context::ContextStackCore::GetInstance().getCurrentScope();
    if (contextScope && m_engineContext)
    {
        contextScope->removeContext(m_engineContext);
    }

    m_engineContext = nullptr;
}

void ego::engine_framework::EngineFramework::releaseProjectAssetFileSystems()
{
    const context::RuntimeContextPointer runtimeContext = context::GetRuntimeContextPointer();
    const ResourceControllerPointer resourceController = runtimeContext ? runtimeContext->getResourceControllerPointer() : nullptr;

    for (const FileSystemPointer& assetFileSystem : m_projectAssetFileSystems)
    {
        if (assetFileSystem)
        {
            if (resourceController)
            {
                resourceController->removeFileSystem(assetFileSystem);
            }

            assetFileSystem->release();
        }
    }

    m_projectAssetFileSystems.clear();
}

bool ego::engine_framework::EngineFramework::registerProjectAssetFileSystems()
{
    if (!m_project)
    {
        return true;
    }

    ResourceController& resourceController = context::GetRuntimeContext().getResourceController();

    const FileSystemPointer sourceFileSystem = context::GetPlatform().getFileSystem();
    EGO_CHECK_RETURN_FALSE(sourceFileSystem);

    for (const FileName& assetDirectory : m_project->getAssetDirectories())
    {
        FileSystemPointer assetFileSystem = createProjectAssetFileSystem(sourceFileSystem, assetDirectory);
        EGO_CHECK_RETURN_FALSE(assetFileSystem);

        resourceController.addFileSystem(assetFileSystem);
        m_projectAssetFileSystems.push_back(assetFileSystem);
    }

    return true;
}

ego::FileSystemPointer ego::engine_framework::EngineFramework::createProjectAssetFileSystem(const FileSystemPointer& _sourceFileSystem, const FileName& _rootPath) const
{
    RootedFileSystemPointer fileSystem = new RootedFileSystem(_sourceFileSystem, _rootPath);
    return fileSystem && fileSystem->init() ? fileSystem : nullptr;
}

bool ego::engine_framework::EngineFramework::loadProjectPlugins()
{
    return m_project ? loadPlugins(m_project->getPlugins()) : true;
}

bool ego::engine_framework::EngineFramework::loadPlugins(const Project::PluginCollection& _plugins)
{
    for (const PluginPointer& plugin : _plugins)
    {
        EGO_CHECK_RETURN_FALSE(loadPlugin(plugin));
    }

    return true;
}

bool ego::engine_framework::EngineFramework::loadPlugin(const PluginPointer& _plugin)
{
    const engine::EnginePointer engine = m_engineContext ? m_engineContext->getEnginePointer() : nullptr;
    EGO_ASSERT(engine);
    EGO_CHECK_RETURN_FALSE(engine && _plugin);

    m_plugins.push_back(_plugin);
    return true;
}

bool ego::engine_framework::EngineFramework::loadProjectEngineLogic()
{
    if (!m_project)
    {
        return true;
    }

    const EngineLogicPluginPointer engineLogicPlugin = m_project->getEngineLogicPlugin();
    if (!engineLogicPlugin)
    {
        return true;
    }

    return loadEngineLogic(engineLogicPlugin);
}

bool ego::engine_framework::EngineFramework::loadEngineLogic(const EngineLogicPluginPointer& _plugin)
{
    EGO_CHECK_RETURN_FALSE(_plugin);

    EngineLogicPointer engineLogic = _plugin->createEngineLogic();
    EGO_CHECK_RETURN_FALSE(engineLogic);

    if (!engineLogic->init())
    {
        engineLogic->release();
        return false;
    }

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_currentEngineLogic);
    m_currentEngineLogicPlugin = nullptr;

    m_currentEngineLogicPlugin = _plugin;
    m_currentEngineLogic = engineLogic;

    return true;
}

void ego::engine_framework::EngineFramework::updateCurrentEngineLogic(float _deltaTime)
{
    if (m_currentEngineLogic)
    {
        m_currentEngineLogic->update(_deltaTime);
    }
}
