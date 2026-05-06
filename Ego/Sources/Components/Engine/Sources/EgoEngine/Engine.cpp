#include "Engine.h"

#include "EgoCore/Assert/AssertCore.h"

#include "EgoPlugin/PluginController.h"

#include "Event/EventController.h"
#include "Platform/Platform.h"
#include "Plugin/EnginePluginController.h"

bool ego::engine::Engine::init(const EngineInitData& _initData)
{
    m_pluginController = new PluginController;
    EGO_CHECK_INITIALIZATION(PluginControllerCore::GetInstance().init(m_pluginController))
    EGO_CHECK_INITIALIZATION(m_pluginController && m_pluginController->init());

    m_enginePluginController = new EnginePluginController();
    EGO_CHECK_INITIALIZATION(m_enginePluginController && m_enginePluginController->init());

    m_eventController = new EventController();
    EGO_CHECK_INITIALIZATION(m_eventController && m_eventController->init());

    m_platformPlugin = m_enginePluginController->loadEnginePlugin<PlatformPlugin>(_initData.m_platformPluginModuleName);
    EGO_CHECK_INITIALIZATION(m_platformPlugin);

    m_platform = m_platformPlugin->createPlatform(_initData.m_nativeInstanceHandle);
    EGO_CHECK_INITIALIZATION(m_platform && m_platform->init());

    m_renderHardwarePlugin = m_enginePluginController->loadEnginePlugin<RenderHardwarePlugin>(_initData.m_renderHardwarePluginModuleName);
    EGO_CHECK_INITIALIZATION(m_renderHardwarePlugin);

    m_graphicDevice = m_renderHardwarePlugin->createGraphicDevice();
    EGO_CHECK_INITIALIZATION(m_graphicDevice && m_graphicDevice->init(gpu::GraphicDeviceInitParams()));

    return true;
}

void ego::engine::Engine::release()
{
    EGO_SAFE_DESTROY_WITH_RELEASING(m_platform);
    EGO_SAFE_DESTROY_WITH_RELEASING(m_eventController);
    EGO_SAFE_DESTROY(m_enginePluginController);
    EGO_SAFE_DESTROY_WITH_RELEASING(m_pluginController);
}

void ego::engine::Engine::run()
{
    if (!m_platform->getMainWindowProvider().isWindowPlatformProvided())
    {
        m_platform->getMainWindowProvider().prepareMainWindow("EGO", WindowSize(500, 500));
    }

    beginFrame();
    endFrame();

    while (!m_isStopped)
    {
        //EDGE_PROFILE_BLOCK_EVENT("Frame");

        beginFrame();

        if (!m_platform->getMainWindowProvider().getMainWindow()->isValid())
        {
            stop();
            continue;
        }

        m_platform->getPlatformEventController().updateNativeEvents();
        //m_inputDeviceController->update();

        //JobGraphReference mailLoopJobGraph = getMainLoopJobGraph();
        //m_jobController->addJobGraph(mailLoopJobGraph);
        //m_jobController->waitAndExecute(mailLoopJobGraph);

        endFrame();

        ++m_currentFrame;
    }
}

void ego::engine::Engine::stop()
{
    m_isStopped = true;
}

void ego::engine::Engine::pause()
{
    m_isPaused = true;
}

void ego::engine::Engine::unpause()
{
    m_isPaused = false;
}

uint32_t ego::engine::Engine::getCurrentFrameNum() const
{
    return m_currentFrame;
}

float ego::engine::Engine::getDeltaTime() const
{
    return m_deltaTime * m_timeScale;
}

void ego::engine::Engine::setTimeScale(float _scale)
{
    EGO_ASSERT(_scale >= 0);
    m_timeScale = _scale;
}

bool ego::engine::Engine::isStopped() const
{
    return m_isStopped;
}

bool ego::engine::Engine::isPaused() const
{
    return m_isPaused;
}

const ego::Platform& ego::engine::Engine::getPlatform() const
{
    EGO_ASSERT(m_platform);
    return *m_platform;
}

ego::Platform& ego::engine::Engine::getPlatform()
{
    EGO_ASSERT(m_platform);
    return *m_platform;
}

const ego::gpu::GraphicDevice& ego::engine::Engine::getGraphicDevice() const
{
    EGO_ASSERT(m_graphicDevice);
    return *m_graphicDevice;
}

ego::gpu::GraphicDevice& ego::engine::Engine::getGraphicDevice()
{
    EGO_ASSERT(m_graphicDevice);
    return *m_graphicDevice;
}

const ego::EventController& ego::engine::Engine::getEventController() const
{
    EGO_ASSERT(m_eventController);
    return *m_eventController;
}

ego::EventController& ego::engine::Engine::getEventController()
{
    EGO_ASSERT(m_eventController);
    return *m_eventController;
}

const ego::engine::EnginePluginController& ego::engine::Engine::getPluginController() const
{
    EGO_ASSERT(m_enginePluginController);
    return *m_enginePluginController;
}

ego::engine::EnginePluginController& ego::engine::Engine::getPluginController()
{
    EGO_ASSERT(m_enginePluginController);
    return *m_enginePluginController;
}

void ego::engine::Engine::beginFrame()
{
    m_currentFrameTime = Clock::GetCurrentTimePoint();
    m_deltaTime = Clock::CalcTimePointDelta<float>(m_currentFrameTime, m_prevFrameStartTime);
}

void ego::engine::Engine::endFrame()
{
    m_prevFrameStartTime = m_currentFrameTime;
}

ego::engine::Engine& ego::engine::EngineCore::getEngine() const
{
    EGO_ASSERT(m_engine);
    return *m_engine;
}

void ego::engine::EngineCore::init(Engine* _engine)
{
    EGO_ASSERT(!m_engine);
    m_engine = _engine;
}

ego::engine::Engine& ego::engine::GetEngine()
{
    return EngineCore::GetInstance().getEngine();
}
