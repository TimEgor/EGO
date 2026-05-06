#include "DemoController.h"

bool ego::demo::DemoController::init(const engine::EngineInitData& _initData)
{
    m_engine = new engine::Engine();
    engine::EngineCore::GetInstance().init(m_engine);

    EGO_CHECK_INITIALIZATION(m_engine && m_engine->init(_initData));

    return true;
}

void ego::demo::DemoController::release()
{
    engine::EngineCore::GetInstance().init(nullptr);

    EGO_SAFE_DESTROY_WITH_RELEASING(m_engine);
}

void ego::demo::DemoController::run()
{
    EGO_ASSERT(m_engine);
    m_engine->run();
}

ego::demo::DemoController& ego::demo::DemoControllerCore::getController() const
{
    EGO_ASSERT(m_controller);
    return *m_controller;
}

void ego::demo::DemoControllerCore::init(DemoController* _controller)
{
    EGO_ASSERT(!m_controller);
    m_controller = _controller;
}

ego::demo::DemoController& ego::demo::GetDemoController()
{
    return DemoControllerCore::GetInstance().getController();
}
