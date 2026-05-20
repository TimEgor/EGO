#include "DemoController.h"

bool ego::demo::DemoController::init(const DemoControllerInitData& _initData)
{
    m_engine = new engine::Engine();
    engine::EngineCore::GetInstance().init(m_engine);

    EGO_CHECK_INITIALIZATION(m_engine && m_engine->init(_initData.m_engineInitData));

    m_pluginController = new DemoPluginController();
    EGO_CHECK_INITIALIZATION(m_pluginController && m_pluginController->init());

    EGO_CHECK_INITIALIZATION(loadDemo(_initData.m_demoPluginModuleName));

    return true;
}

bool ego::demo::DemoController::init(const engine::EngineInitData& _initData)
{
    DemoControllerInitData initData;
    initData.m_engineInitData = _initData;
    return init(initData);
}

void ego::demo::DemoController::release()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_currentDemo);
    m_currentDemoPlugin = nullptr;

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_pluginController);

    engine::EngineCore::GetInstance().release();

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_engine);
}

void ego::demo::DemoController::run()
{
    EGO_ASSERT(m_engine);
    m_engine->run();
}

ego::demo::DemoPointer ego::demo::DemoController::getCurrentDemo() const
{
    return m_currentDemo;
}

const ego::demo::Demo& ego::demo::DemoController::getCurrentDemoRef() const
{
    EGO_ASSERT(m_currentDemo);
    return *m_currentDemo.get();
}

ego::demo::Demo& ego::demo::DemoController::getCurrentDemoRef()
{
    EGO_ASSERT(m_currentDemo);
    return *m_currentDemo.get();
}

bool ego::demo::DemoController::loadDemo(const FileName& _moduleName)
{
    EGO_ASSERT(m_pluginController);

    DemoPluginPointer demoPlugin = m_pluginController->loadDemoPlugin<DemoPlugin>(_moduleName);
    EGO_CHECK_RETURN_FALSE(demoPlugin);

    DemoPointer demo = demoPlugin->createDemo();
    EGO_CHECK_RETURN_FALSE(demo);

    if (!demo->init())
    {
        demo->release();
        return false;
    }

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_currentDemo);
    m_currentDemoPlugin = nullptr;

    m_currentDemoPlugin = demoPlugin;
    m_currentDemo = demo;

    return true;
}

ego::demo::DemoControllerPointer ego::demo::DemoControllerCore::getController() const
{
    return m_controller;
}

void ego::demo::DemoControllerCore::init(const DemoControllerPointer& _controller)
{
    m_controller = _controller;
}

void ego::demo::DemoControllerCore::release()
{
    m_controller = nullptr;
}

ego::demo::DemoController& ego::demo::GetDemoController()
{
    const DemoControllerPointer demoController = DemoControllerCore::GetInstance().getController();

    EGO_ASSERT(demoController);
    return *demoController.get();
}
