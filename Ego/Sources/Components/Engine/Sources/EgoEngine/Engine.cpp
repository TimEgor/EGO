#include "Engine.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Job/Job.h"
#include "EgoCore/Job/JobController.h"

#include "Event/EventController.h"
#include "Graphic/Presenter/WindowGraphicPresenter.h"
#include "Graphic/Render/DefaultRender.h"
#include "Graphic/RenderHardware/RenderHardwarePlugin.h"
#include "Level/LevelController.h"
#include "Platform/Platform.h"
#include "Platform/PlatformPlugin.h"
#include "Plugin/EnginePluginController.h"
#include "Plugin/PluginCatalogBuilder.h"
#include "Resources/Resource/ResourceController.h"

bool ego::engine::Engine::init(const Engine::InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(initPluginController());

    m_eventController = new EventController();
    EGO_CHECK_INITIALIZATION(m_eventController && m_eventController->init());

    m_jobController = new JobController();

    const uint32_t jobThreadCount = JobController::GetHardwareThreadCount();
    EGO_CHECK_INITIALIZATION(m_jobController && m_jobController->init(jobThreadCount ? jobThreadCount : 1));

    EGO_CHECK_INITIALIZATION(initPlatform(_initData));

    m_resourceController = new ResourceController();
    EGO_CHECK_INITIALIZATION(m_resourceController && m_resourceController->init());
    m_resourceController->addFileSystem(m_platform->getFileSystem());

    EGO_CHECK_INITIALIZATION(initPluginCatalog(_initData));
    EGO_CHECK_INITIALIZATION(initGraphicDevice(_initData));

    m_levelController = new LevelController();
    EGO_CHECK_INITIALIZATION(m_levelController && m_levelController->init());

    m_render = new DefaultRender();
    EGO_CHECK_INITIALIZATION(m_render && m_render->init());

    return true;
}

void ego::engine::Engine::release()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_render);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_graphicPresenter);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_graphicDevice);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_levelController);

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_resourceController);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_platform);

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_jobController);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_eventController);

    m_renderHardwarePlugin = nullptr;
    m_platformPlugin = nullptr;

    m_pluginCatalog.clear();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_enginePluginController);
}

void ego::engine::Engine::run()
{
    if (!m_platform->getMainWindowProvider().isWindowPlatformProvided())
    {
        m_platform->getMainWindowProvider().prepareMainWindow("EGO", WindowSize(500, 500));
    }

    if (!prepareMainWindowPresenter())
    {
        return;
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

        JobGraphReference mainLoopJobGraph = getMainLoopJobGraph();
        m_jobController->addJobGraph(mainLoopJobGraph);
        m_jobController->waitAndExecute(mainLoopJobGraph);

        endFrame();

        ++m_currentFrame;
    }

    if (m_render)
    {
        m_render->clearResources();
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

const ego::GraphicDevice& ego::engine::Engine::getGraphicDevice() const
{
    EGO_ASSERT(m_graphicDevice);
    return *m_graphicDevice;
}

ego::GraphicDevice& ego::engine::Engine::getGraphicDevice()
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

const ego::JobController& ego::engine::Engine::getJobController() const
{
    EGO_ASSERT(m_jobController);
    return *m_jobController;
}

ego::JobController& ego::engine::Engine::getJobController()
{
    EGO_ASSERT(m_jobController);
    return *m_jobController;
}

const ego::ResourceController& ego::engine::Engine::getResourceController() const
{
    EGO_ASSERT(m_resourceController);
    return *m_resourceController;
}

ego::ResourceController& ego::engine::Engine::getResourceController()
{
    EGO_ASSERT(m_resourceController);
    return *m_resourceController;
}

const ego::LevelController& ego::engine::Engine::getLevelController() const
{
    EGO_ASSERT(m_levelController);
    return *m_levelController;
}

ego::LevelController& ego::engine::Engine::getLevelController()
{
    EGO_ASSERT(m_levelController);
    return *m_levelController;
}

const ego::DefaultRender& ego::engine::Engine::getRender() const
{
    EGO_ASSERT(m_render);
    return *m_render;
}

ego::DefaultRender& ego::engine::Engine::getRender()
{
    EGO_ASSERT(m_render);
    return *m_render;
}

ego::ecs::Entity ego::engine::Engine::getRenderCameraEntity() const
{
    return m_renderCameraEntity;
}

void ego::engine::Engine::setRenderCameraEntity(ecs::Entity _cameraEntity)
{
    m_renderCameraEntity = _cameraEntity;
}

void ego::engine::Engine::clearRenderCameraEntity()
{
    m_renderCameraEntity = ecs::Entity();
}

ego::JobGraphReference ego::engine::Engine::getMainLoopJobGraph()
{
    JobGraphBuilder graphBuilder;
    graphBuilder.addJob(
        CreateLambdaJob(
            [this]()
            {
                renderFrame();
            },
            "Render"
        )
    );

    return graphBuilder.getGraph();
}

const ego::engine::PluginCatalog& ego::engine::Engine::getPluginCatalog() const
{
    return m_pluginCatalog;
}

ego::engine::PluginCatalog& ego::engine::Engine::getPluginCatalog()
{
    return m_pluginCatalog;
}

bool ego::engine::Engine::initPluginController()
{
    m_enginePluginController = new EnginePluginController();
    EGO_CHECK_RETURN_FALSE(m_enginePluginController && m_enginePluginController->init());

    return true;
}

bool ego::engine::Engine::initPlatform(const InitData& _initData)
{
    EGO_ASSERT(m_enginePluginController);

    FileName platformPluginModuleName = _initData.m_platformPluginModuleName;
    if (!platformPluginModuleName)
    {
        platformPluginModuleName = m_enginePluginController->selectEnginePluginModule<PlatformPlugin>();
    }

    EGO_CHECK_RETURN_FALSE(platformPluginModuleName);

    m_platformPlugin = m_enginePluginController->loadEnginePlugin<PlatformPlugin>(platformPluginModuleName);
    EGO_CHECK_RETURN_FALSE(m_platformPlugin);

    m_platform = m_platformPlugin->createPlatform(_initData.m_nativeInstanceHandle);
    EGO_CHECK_RETURN_FALSE(m_platform && m_platform->init());

    return true;
}

bool ego::engine::Engine::initPluginCatalog(const InitData& _initData)
{
    EGO_CHECK_RETURN_FALSE(m_platform);

    m_pluginCatalog.clear();

    const FileSystemPointer fileSystem = m_platform->getFileSystem();
    EGO_CHECK_RETURN_FALSE(fileSystem);

    for (const FileName& pluginDirectory : _initData.m_pluginDirectories)
    {
        PluginCatalogBuilder::AddPluginsFromPath(m_pluginCatalog, *fileSystem, pluginDirectory);
    }

    return true;
}

bool ego::engine::Engine::initGraphicDevice(const InitData& _initData)
{
    EGO_ASSERT(m_enginePluginController);

    FileName renderHardwarePluginModuleName = _initData.m_renderHardwarePluginModuleName;
    if (!renderHardwarePluginModuleName)
    {
        renderHardwarePluginModuleName = m_enginePluginController->selectEnginePluginModule<RenderHardwarePlugin>();
    }

    EGO_CHECK_RETURN_FALSE(renderHardwarePluginModuleName);

    m_renderHardwarePlugin =
        m_enginePluginController->loadEnginePlugin<RenderHardwarePlugin>(renderHardwarePluginModuleName);
    EGO_CHECK_RETURN_FALSE(m_renderHardwarePlugin);

    m_graphicDevice = m_renderHardwarePlugin->createGraphicDevice();
    EGO_CHECK_RETURN_FALSE(m_graphicDevice && m_graphicDevice->init(GraphicDevice::InitParams()));

    return true;
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

bool ego::engine::Engine::prepareMainWindowPresenter()
{
    if (m_graphicPresenter)
    {
        return true;
    }

    const WindowPointer mainWindow = m_platform->getMainWindowProvider().getMainWindow();
    EGO_CHECK_RETURN_FALSE(mainWindow && mainWindow->isValid());

    gpu::SwapChainDesc swapChainDesc;
    swapChainDesc.m_format = gpu::GraphicResourceFormat::B8G8R8A8UNorm;
    swapChainDesc.m_bufferCount = 2;

    m_graphicPresenter = new WindowGraphicPresenter();
    EGO_CHECK_RETURN_FALSE(m_graphicPresenter && m_graphicPresenter->init(*mainWindow, swapChainDesc));

    return true;
}

void ego::engine::Engine::renderFrame()
{
    if (!m_render || !m_graphicPresenter || !m_levelController)
    {
        return;
    }

    const LevelPointer activeLevel = m_levelController->getActiveLevel();
    if (!activeLevel || !m_renderCameraEntity)
    {
        return;
    }

    m_render->render(*m_graphicPresenter, *activeLevel, m_renderCameraEntity);
    m_render->wait();
    m_render->present(*m_graphicPresenter);
}

ego::engine::EnginePointer ego::engine::EngineCore::getEngine() const
{
    return m_engine;
}

void ego::engine::EngineCore::init(const EnginePointer& _engine)
{
    EGO_ASSERT(!m_engine || !_engine);
    m_engine = _engine;
}

void ego::engine::EngineCore::release()
{
    m_engine = nullptr;
}

ego::engine::Engine& ego::engine::GetEngine()
{
    const EnginePointer engine = EngineCore::GetInstance().getEngine();

    EGO_ASSERT(engine);
    return *engine.get();
}
