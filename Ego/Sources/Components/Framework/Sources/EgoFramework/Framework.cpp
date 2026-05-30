#include "Framework.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Job/JobDescriptor.h"
#include "EgoCore/UtilsMacros.h"

bool ego::framework::Framework::init(const Framework::InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(initPluginController());

    m_project = _initData.m_project;

    EGO_CHECK_INITIALIZATION(initEngine(_initData));

    m_gameLogicPluginController = new GameLogicPluginController();
    EGO_CHECK_INITIALIZATION(m_gameLogicPluginController && m_gameLogicPluginController->init());

    EGO_CHECK_INITIALIZATION(loadProfilerPlugin(_initData));
    EGO_CHECK_INITIALIZATION(loadProjectPlugins());

    FileName gameLogicPluginModuleName;
    if (m_project && !m_project->getGameLogicPlugins().empty())
    {
        gameLogicPluginModuleName = selectProjectGameLogicPluginModule();
        EGO_CHECK_INITIALIZATION(gameLogicPluginModuleName);
    }

    if (gameLogicPluginModuleName)
    {
        EGO_CHECK_INITIALIZATION(loadGameLogic(gameLogicPluginModuleName));
    }

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
        if (plugin.m_moduleName)
        {
            return plugin.m_moduleName;
        }

        if (!plugin.m_name.empty())
        {
            const FileName moduleName = m_engine->getPluginCatalog().getModulePath(
                GameLogicPlugin::GetPluginType(),
                plugin.m_name.c_str()
            );
            if (moduleName)
            {
                return moduleName;
            }
        }
    }

    return FileName();
}

void ego::framework::Framework::release()
{
    m_updateGameLogicJobID = JobDescriptorID();

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_currentGameLogic);
    m_currentGameLogicPlugin = nullptr;

    m_plugins.clear();

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_gameLogicPluginController);

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_engine);
    engine::EngineCore::GetInstance().release();

    m_project = nullptr;

    releasePluginController();
}

bool ego::framework::Framework::initPluginController()
{
    m_pluginController = new PluginController();
    EGO_CHECK_RETURN_FALSE(PluginControllerCore::GetInstance().init(m_pluginController));
    EGO_CHECK_RETURN_FALSE(m_pluginController && m_pluginController->init());

    return true;
}

void ego::framework::Framework::releasePluginController()
{
    PluginControllerCore::GetInstance().release();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_pluginController);
}

bool ego::framework::Framework::initEngine(const Framework::InitData& _initData)
{
    engine::Engine::InitData engineInitData = _initData.m_engineInitData;

    m_engine = new engine::Engine();
    engine::EngineCore::GetInstance().init(m_engine);

    if (m_project)
    {
        const Project::DirectoryCollection& pluginDirectories = m_project->getPluginDirectories();
        engineInitData.m_pluginDirectories.insert(
            engineInitData.m_pluginDirectories.end(),
            pluginDirectories.begin(),
            pluginDirectories.end()
        );
    }

    EGO_CHECK_RETURN_FALSE(m_engine && m_engine->init(engineInitData));
    EGO_CHECK_RETURN_FALSE(registerGameLogicMainLoopJob());

    return true;
}

bool ego::framework::Framework::loadProfilerPlugin(const InitData& _initData)
{
    if (!_initData.m_profilerPluginModuleName)
    {
        return true;
    }

    return loadPlugin("ProfilerPlugin", _initData.m_profilerPluginModuleName);
}

bool ego::framework::Framework::loadPlugin(const Project::Plugin& _plugin)
{
    EGO_ASSERT(m_engine);
    EGO_ASSERT(m_pluginController);
    EGO_CHECK_RETURN_FALSE(m_engine && m_pluginController);
    EGO_CHECK_RETURN_FALSE(!_plugin.m_type.empty());

    FileName moduleName = _plugin.m_moduleName;
    if (!moduleName)
    {
        EGO_CHECK_RETURN_FALSE(!_plugin.m_name.empty());
        moduleName = m_engine->getPluginCatalog().getModulePath(
            GetPluginType(_plugin.m_type.c_str()),
            _plugin.m_name.c_str()
        );
    }

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

bool ego::framework::Framework::loadPlugins(const Project::PluginCollection& _plugins)
{
    for (const Project::Plugin& plugin : _plugins)
    {
        EGO_CHECK_RETURN_FALSE(loadPlugin(plugin));
    }

    return true;
}

bool ego::framework::Framework::loadProjectPlugins()
{
    return m_project ? loadPlugins(m_project->getPlugins()) : true;
}

bool ego::framework::Framework::registerGameLogicMainLoopJob()
{
    EGO_ASSERT(m_engine);

    engine::MainLoop& mainLoop = m_engine->getMainLoop();
    const JobDescriptorID gameLogicBeginJobID = mainLoop.getGameLogicBeginJobID();
    const JobDescriptorID gameLogicEndJobID = mainLoop.getGameLogicEndJobID();
    EGO_CHECK_RETURN_FALSE(gameLogicBeginJobID.isValid());
    EGO_CHECK_RETURN_FALSE(gameLogicEndJobID.isValid());

    m_updateGameLogicJobID = mainLoop.addJobBetween(
        CreateJobDescriptor(
            [this]()
            {
                updateCurrentGameLogic(m_engine->getDeltaTime());
            },
            "Game logic update"
        ),
        gameLogicBeginJobID,
        gameLogicEndJobID
    );

    return m_updateGameLogicJobID.isValid();
}

void ego::framework::Framework::run()
{
    EGO_ASSERT(m_engine);
    m_engine->run();
}

void ego::framework::Framework::updateCurrentGameLogic(float _deltaTime)
{
    if (m_currentGameLogic)
    {
        m_currentGameLogic->update(_deltaTime);
    }
}

bool ego::framework::Framework::loadGameLogic(const FileName& _moduleName)
{
    EGO_ASSERT(m_gameLogicPluginController);
    EGO_CHECK_RETURN_FALSE(_moduleName);

    GameLogicPluginPointer gameLogicPlugin =
        m_gameLogicPluginController->loadGameLogicPlugin<GameLogicPlugin>(_moduleName);
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

ego::framework::ProjectPointer ego::framework::Framework::getProject() const
{
    return m_project;
}

ego::framework::GameLogic& ego::framework::Framework::getCurrentGameLogic() const
{
    EGO_ASSERT(m_currentGameLogic);
    return *m_currentGameLogic;
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
