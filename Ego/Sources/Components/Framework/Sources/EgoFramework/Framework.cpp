#include "Framework.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/UtilsMacros.h"

bool ego::framework::Framework::init(const Framework::InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(initPluginController());

    m_project = _initData.m_project;

    FileName gameLogicPluginModuleName = _initData.m_gameLogicPluginModuleName;
    if (!gameLogicPluginModuleName && m_project && !m_project->getGameLogicPlugins().empty())
    {
        gameLogicPluginModuleName = m_project->getGameLogicPlugins().front().m_moduleName;
    }

    EGO_CHECK_INITIALIZATION(initEngine(_initData));

    m_gameLogicPluginController = new GameLogicPluginController();
    EGO_CHECK_INITIALIZATION(m_gameLogicPluginController && m_gameLogicPluginController->init());

    if (gameLogicPluginModuleName)
    {
        EGO_CHECK_INITIALIZATION(loadGameLogic(gameLogicPluginModuleName));
    }

    return true;
}

void ego::framework::Framework::release()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_currentGameLogic);
    m_currentGameLogicPlugin = nullptr;

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

    return true;
}

void ego::framework::Framework::run()
{
    EGO_ASSERT(m_engine);
    m_engine->run();
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

