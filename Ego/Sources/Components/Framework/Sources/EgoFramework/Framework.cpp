#include "Framework.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Job/JobDescriptor.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Platform/FileSystem/RootedFileSystem.h"

bool ego::framework::Framework::init(const Framework::InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(initPluginController());

    m_project = _initData.m_project;

    EGO_CHECK_INITIALIZATION(initEngine(_initData));
    EGO_CHECK_INITIALIZATION(registerProjectAssetFileSystems());
    EGO_CHECK_INITIALIZATION(initGameLogicPluginController());

    EGO_CHECK_INITIALIZATION(loadProfilerPlugin(_initData));
    EGO_CHECK_INITIALIZATION(loadProjectPlugins());
    EGO_CHECK_INITIALIZATION(loadProjectGameLogic());

    return true;
}

void ego::framework::Framework::release()
{
    m_updateGameLogicJobID = JobDescriptorID();

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_currentGameLogic);
    m_currentGameLogicPlugin = nullptr;

    m_plugins.clear();

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_gameLogicPluginController);

    releaseProjectAssetFileSystems();

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_engine);
    engine::EngineCore::GetInstance().release();

    m_project = nullptr;

    releasePluginController();
}

void ego::framework::Framework::run()
{
    EGO_ASSERT(m_engine);
    m_engine->run();
}

bool ego::framework::Framework::runFrame()
{
    EGO_ASSERT(m_engine);
    return m_engine->runFrame();
}

ego::PluginController& ego::framework::Framework::getPluginController() const
{
    EGO_ASSERT(m_pluginController);
    return *m_pluginController;
}

ego::engine::Engine& ego::framework::Framework::getEngine() const
{
    EGO_ASSERT(m_engine);
    return *m_engine;
}

const ego::framework::Project& ego::framework::Framework::getProject() const
{
    EGO_ASSERT(m_project);
    return *m_project;
}

ego::framework::GameLogic& ego::framework::Framework::getCurrentGameLogic() const
{
    EGO_ASSERT(m_currentGameLogic);
    return *m_currentGameLogic;
}

bool ego::framework::Framework::initPluginController()
{
    m_pluginController = PluginControllerCore::GetInstance().getPluginController();
    if (m_pluginController)
    {
        return true;
    }

    m_pluginController = new PluginController();
    EGO_CHECK_RETURN_FALSE(m_pluginController);
    EGO_CHECK_RETURN_FALSE(PluginControllerCore::GetInstance().init(m_pluginController));

    m_isPluginControllerCoreInitialized = true;
    if (!m_pluginController->init())
    {
        PluginControllerCore::GetInstance().release();
        m_pluginController = nullptr;
        m_isPluginControllerCoreInitialized = false;
        return false;
    }

    return true;
}

bool ego::framework::Framework::initEngine(const Framework::InitData& _initData)
{
    engine::Engine::InitData engineInitData = _initData.m_engineInitData;
    appendProjectPluginDirectories(engineInitData);

    m_engine = new engine::Engine();
    engine::EngineCore::GetInstance().init(m_engine);

    EGO_CHECK_RETURN_FALSE(m_engine && m_engine->init(engineInitData));
    EGO_CHECK_RETURN_FALSE(registerGameLogicMainLoopJob());

    return true;
}

bool ego::framework::Framework::initGameLogicPluginController()
{
    m_gameLogicPluginController = new GameLogicPluginController();
    EGO_CHECK_RETURN_FALSE(m_gameLogicPluginController && m_gameLogicPluginController->init());

    return true;
}

bool ego::framework::Framework::registerGameLogicMainLoopJob()
{
    EGO_ASSERT(m_engine);

    engine::MainLoop& mainLoop = m_engine->getMainLoop();
    const JobDescriptorID frameLogicBeginJobID = mainLoop.getFrameLogicBeginJobID();
    const JobDescriptorID frameLogicEndJobID = mainLoop.getFrameLogicEndJobID();
    EGO_CHECK_RETURN_FALSE(frameLogicBeginJobID.isValid());
    EGO_CHECK_RETURN_FALSE(frameLogicEndJobID.isValid());

    m_updateGameLogicJobID = mainLoop.addJobBetween(
        CreateJobDescriptor(
            [this]()
            {
                updateCurrentGameLogic(m_engine->getDeltaTime());
            },
            "Game logic update"),
        frameLogicBeginJobID,
        frameLogicEndJobID);

    return m_updateGameLogicJobID.isValid();
}

void ego::framework::Framework::releasePluginController()
{
    if (m_isPluginControllerCoreInitialized)
    {
        PluginControllerCore::GetInstance().release();
        EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_pluginController);
        m_isPluginControllerCoreInitialized = false;
        return;
    }

    m_pluginController = nullptr;
}

void ego::framework::Framework::releaseProjectAssetFileSystems()
{
    for (const FileSystemPointer& assetFileSystem : m_projectAssetFileSystems)
    {
        if (assetFileSystem)
        {
            if (m_engine)
            {
                m_engine->getResourceController().removeFileSystem(assetFileSystem);
            }

            assetFileSystem->release();
        }
    }

    m_projectAssetFileSystems.clear();
}

void ego::framework::Framework::appendProjectPluginDirectories(engine::Engine::InitData& _engineInitData) const
{
    if (!m_project)
    {
        return;
    }

    const Project::DirectoryCollection& pluginDirectories = m_project->getPluginDirectories();
    _engineInitData.m_pluginDirectories.insert(_engineInitData.m_pluginDirectories.end(), pluginDirectories.begin(), pluginDirectories.end());
}

bool ego::framework::Framework::registerProjectAssetFileSystems()
{
    if (!m_project)
    {
        return true;
    }

    EGO_ASSERT(m_engine);
    EGO_CHECK_RETURN_FALSE(m_engine);

    ResourceController& resourceController = m_engine->getResourceController();
    const FileSystemPointer sourceFileSystem = m_engine->getPlatform().getFileSystem();
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

ego::FileSystemPointer ego::framework::Framework::createProjectAssetFileSystem(const FileSystemPointer& _sourceFileSystem, const FileName& _rootPath) const
{
    RootedFileSystemPointer fileSystem = new RootedFileSystem(_sourceFileSystem, _rootPath);
    return fileSystem && fileSystem->init() ? fileSystem : nullptr;
}

bool ego::framework::Framework::loadProfilerPlugin(const InitData& _initData)
{
    if (!_initData.m_profilerPluginModuleName)
    {
        return true;
    }

    return loadPlugin("ProfilerPlugin", _initData.m_profilerPluginModuleName);
}

bool ego::framework::Framework::loadProjectPlugins()
{
    return m_project ? loadPlugins(m_project->getPlugins()) : true;
}

bool ego::framework::Framework::loadPlugins(const Project::PluginCollection& _plugins)
{
    for (const Project::Plugin& plugin : _plugins)
    {
        EGO_CHECK_RETURN_FALSE(loadPlugin(plugin));
    }

    return true;
}

bool ego::framework::Framework::loadPlugin(const Project::Plugin& _plugin)
{
    EGO_ASSERT(m_engine);
    EGO_ASSERT(m_pluginController);
    EGO_CHECK_RETURN_FALSE(m_engine && m_pluginController);
    EGO_CHECK_RETURN_FALSE(!_plugin.m_type.empty());

    const FileName moduleName = resolvePluginModuleName(_plugin);
    EGO_CHECK_RETURN_FALSE(moduleName);

    PluginPointer loadedPlugin = m_pluginController->loadPlugin(moduleName, _plugin.m_type.c_str());
    EGO_CHECK_RETURN_FALSE(loadedPlugin);

    m_plugins.push_back(loadedPlugin);
    return true;
}

bool ego::framework::Framework::loadPlugin(const char* _type, const FileName& _moduleName)
{
    Project::Plugin plugin;
    plugin.m_type = _type ? _type : "";
    plugin.m_moduleName = _moduleName;
    return loadPlugin(plugin);
}

ego::FileName ego::framework::Framework::resolvePluginModuleName(const Project::Plugin& _plugin) const
{
    if (_plugin.m_moduleName)
    {
        return _plugin.m_moduleName;
    }

    if (!_plugin.m_type.empty() && !_plugin.m_name.empty() && m_engine)
    {
        return m_engine->getPluginCatalog().getModulePath(GetPluginType(_plugin.m_type.c_str()), _plugin.m_name.c_str());
    }

    return FileName();
}

bool ego::framework::Framework::loadProjectGameLogic()
{
    if (!m_project || m_project->getGameLogicPlugins().empty())
    {
        return true;
    }

    const FileName gameLogicPluginModuleName = selectProjectGameLogicPluginModule();
    EGO_CHECK_RETURN_FALSE(gameLogicPluginModuleName);

    return loadGameLogic(gameLogicPluginModuleName);
}

bool ego::framework::Framework::loadGameLogic(const FileName& _moduleName)
{
    EGO_ASSERT(m_gameLogicPluginController);
    EGO_CHECK_RETURN_FALSE(_moduleName);

    GameLogicPluginPointer gameLogicPlugin = m_gameLogicPluginController->loadGameLogicPlugin<GameLogicPlugin>(_moduleName);
    EGO_CHECK_RETURN_FALSE(gameLogicPlugin);

    GameLogicPointer gameLogic = gameLogicPlugin->createGameLogic();
    EGO_CHECK_RETURN_FALSE(gameLogic);

    if (!gameLogic->init())
    {
        gameLogic->release();
        return false;
    }

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_currentGameLogic);
    m_currentGameLogicPlugin = nullptr;

    m_currentGameLogicPlugin = gameLogicPlugin;
    m_currentGameLogic = gameLogic;

    return true;
}

ego::FileName ego::framework::Framework::selectProjectGameLogicPluginModule() const
{
    if (!m_project || !m_engine)
    {
        return FileName();
    }

    for (const Project::GameLogicPlugin& plugin : m_project->getGameLogicPlugins())
    {
        const FileName moduleName = resolveGameLogicPluginModuleName(plugin);
        if (moduleName)
        {
            return moduleName;
        }
    }

    return FileName();
}

ego::FileName ego::framework::Framework::resolveGameLogicPluginModuleName(const Project::GameLogicPlugin& _plugin) const
{
    if (_plugin.m_moduleName)
    {
        return _plugin.m_moduleName;
    }

    if (!_plugin.m_name.empty() && m_engine)
    {
        return m_engine->getPluginCatalog().getModulePath(GameLogicPlugin::GetPluginType(), _plugin.m_name.c_str());
    }

    return FileName();
}

void ego::framework::Framework::updateCurrentGameLogic(float _deltaTime)
{
    if (m_currentGameLogic)
    {
        m_currentGameLogic->update(_deltaTime);
    }
}

bool ego::framework::FrameworkCore::init(const FrameworkPointer& _framework)
{
    EGO_ASSERT(_framework);

    if (m_framework)
    {
        EGO_ASSERT_FAIL_MESSAGE("Framework has been already inited.");
        return false;
    }

    m_framework = _framework;

    return static_cast<bool>(m_framework);
}

void ego::framework::FrameworkCore::release()
{
    m_framework = nullptr;
}

ego::framework::FrameworkPointer ego::framework::FrameworkCore::getFramework() const
{
    return m_framework;
}

ego::framework::Framework& ego::framework::GetFramework()
{
    const FrameworkPointer framework = FrameworkCore::GetInstance().getFramework();

    EGO_ASSERT(framework);
    return *framework.get();
}
