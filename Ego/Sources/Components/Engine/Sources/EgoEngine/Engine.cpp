#include "Engine.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Context/ContextStack.h"
#include "EgoCore/Context/PlatformContext.h"
#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Profile/Profile.h"

#include "EgoRuntime/Input/InputController.h"
#include "EgoRuntime/Job/JobDescriptor.h"
#include "EgoRuntime/Plugin/PluginController.h"
#include "EgoRuntime/RuntimeContext.h"

#include "EngineContext.h"
#include "Graphic/Render/RenderPlugin.h"
#include "Level/LevelController.h"

namespace
{
    constexpr const char* EngineGuiFontPath = "C:/Windows/Fonts/segoeui.ttf";
} // namespace

bool ego::engine::Engine::init(const Engine::InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(initJobController(_initData));

    EGO_CHECK_INITIALIZATION(initGraphicPresenter(_initData));

    m_levelController = new LevelController();
    EGO_CHECK_INITIALIZATION(m_levelController && m_levelController->init());

    EGO_CHECK_INITIALIZATION(initGuiController(_initData));

    EGO_CHECK_INITIALIZATION(initRender(_initData));
    syncPresenterTargetResolution();

    EGO_CHECK_INITIALIZATION(initMainLoop());

    return true;
}

void ego::engine::Engine::release()
{
    m_mainLoop.release();

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_render);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_guiController);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_graphicPresenter);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_levelController);

    releaseJobController();

    m_renderPlugin = nullptr;
}

void ego::engine::Engine::run()
{
    while (!m_isStopped)
    {
        if (!runFrame())
        {
            break;
        }
    }

    cleanResources();
}

bool ego::engine::Engine::runFrame()
{
    if (m_isStopped)
    {
        return false;
    }

    EGO_PROFILE_BLOCK_EVENT("Frame");

    beginFrame();

    context::GetPlatform().getInputDeviceController().update();
    context::GetRuntimeContext().getInputController().update();

    if (m_isStopped)
    {
        return false;
    }

    JobGraphReference mainLoopJobGraph = getMainLoopJobGraph();
    if (!mainLoopJobGraph)
    {
        EGO_ASSERT_FAIL_MESSAGE("Main loop job graph is invalid.");

        stop();
        return false;
    }

    EGO_ASSERT(m_jobController);
    m_jobController->addJobGraph(mainLoopJobGraph);
    m_jobController->waitAndExecute(mainLoopJobGraph);

    endFrame();

    ++m_currentFrame;
    if (m_isStopped)
    {
        return false;
    }

    return true;
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

const ego::render::Render& ego::engine::Engine::getRender() const
{
    EGO_ASSERT(m_render);
    return *m_render;
}

ego::render::Render& ego::engine::Engine::getRender()
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

const ego::gui::GuiController& ego::engine::Engine::getGuiController() const
{
    EGO_ASSERT(m_guiController);
    return *m_guiController;
}

ego::gui::GuiController& ego::engine::Engine::getGuiController()
{
    EGO_ASSERT(m_guiController);
    return *m_guiController;
}

ego::gui::GuiControllerPointer ego::engine::Engine::getGuiControllerPointer() const
{
    return m_guiController;
}

ego::JobGraphReference ego::engine::Engine::getMainLoopJobGraph()
{
    return m_mainLoop.createJobGraph();
}

const ego::engine::MainLoop& ego::engine::Engine::getMainLoop() const
{
    return m_mainLoop;
}

ego::engine::MainLoop& ego::engine::Engine::getMainLoop()
{
    return m_mainLoop;
}

bool ego::engine::Engine::initGraphicPresenter(const InitData& _initData)
{
    m_graphicPresenter = _initData.m_graphicPresenter;
    return true;
}

bool ego::engine::Engine::loadDefaultGuiFont(gui::GuiFontAtlasDesc& _fontAtlasDesc) const
{
    const PlatformPointer platform = context::GetPlatformPointer();
    const FileSystemPointer fileSystem = platform ? platform->getFileSystem() : nullptr;
    EGO_CHECK_RETURN_FALSE(fileSystem && fileSystem->readFile(EngineGuiFontPath, _fontAtlasDesc.m_fontData));

    _fontAtlasDesc.m_pixelHeight = 16.0f;
    _fontAtlasDesc.m_width = 512;
    _fontAtlasDesc.m_height = 512;
    _fontAtlasDesc.m_firstCharacter = ' ';
    _fontAtlasDesc.m_characterCount = 95;
    _fontAtlasDesc.m_textureId = gui::GuiDefaultFontTextureID;
    return true;
}

bool ego::engine::Engine::initGuiController(const InitData& _initData)
{
    m_guiController = new gui::GuiController();
    EGO_CHECK_RETURN_FALSE(m_guiController);

    gui::GuiController::InitData guiInitData;
    guiInitData.m_viewportDesc = _initData.m_guiViewportDesc;
    EGO_CHECK_RETURN_FALSE(loadDefaultGuiFont(guiInitData.m_fontAtlasDesc));
    EGO_CHECK_RETURN_FALSE(m_guiController->init(guiInitData));

    return true;
}

bool ego::engine::Engine::initRender(const InitData& _initData)
{
    m_renderPlugin = _initData.m_renderPlugin;
    if (!m_renderPlugin)
    {
        const PluginControllerPointer pluginController = GetCurrentPluginController();
        EGO_CHECK_RETURN_FALSE(pluginController);

        m_renderPlugin = pluginController->loadPlugin<ego::render::RenderPlugin>();
    }

    EGO_CHECK_RETURN_FALSE(m_renderPlugin);

    m_render = m_renderPlugin->createRender();
    EGO_CHECK_RETURN_FALSE(m_render && m_render->init());

    return true;
}

bool ego::engine::Engine::initMainLoop()
{
    return m_mainLoop.init(
        CreateJobDescriptor(
            [this]()
            {
                renderFrame();
            },
            "Render frame"),
        CreateJobDescriptor(
            [this]()
            {
                presentFrame();
            },
            "Present frame"),
        CreateJobDescriptor(
            [this]()
            {
                prepareRenderFrame();
            },
            "Prepare render frame"));
}

bool ego::engine::Engine::initJobController(const InitData& _initData)
{
    m_jobController = new JobController();
    EGO_CHECK_RETURN_FALSE(m_jobController);

    const context::ContextScopePointer contextScope = context::ContextStackCore::GetInstance().getCurrentScope();
    EGO_CHECK_RETURN_FALSE(contextScope);

    uint32_t threadCount = _initData.m_jobThreadCount;
    if (threadCount == 0)
    {
        threadCount = JobController::GetHardwareThreadCount();
    }

    if (threadCount == 0)
    {
        threadCount = 1;
    }

    EGO_CHECK_RETURN_FALSE(m_jobController->init(threadCount, contextScope, _initData.m_jobThreadName));

    return true;
}

void ego::engine::Engine::syncPresenterTargetResolution()
{
    gpu::Texture2DSize targetResolution(0);
    bool hasTargetResolution = false;
    if (m_graphicPresenter)
    {
        const gpu::Texture2DReference targetTexture = m_graphicPresenter->getTargetTexture();
        if (targetTexture)
        {
            targetResolution = targetTexture->getDesc().m_size;
            hasTargetResolution = targetResolution.m_x != 0 && targetResolution.m_y != 0;
        }
    }

    if (hasTargetResolution && m_render)
    {
        const gpu::Texture2DSize& renderResolution = m_render->getResolution();
        if (renderResolution.m_x != targetResolution.m_x || renderResolution.m_y != targetResolution.m_y)
        {
            m_render->setResolution(targetResolution);
        }
    }

    if (hasTargetResolution && m_guiController && m_guiController->isInitialized())
    {
        const gui::GuiViewportPointer guiViewport = m_guiController->getViewport();
        if (guiViewport)
        {
            const gui::GuiSize viewportSize(
                static_cast<float>(targetResolution.m_x),
                static_cast<float>(targetResolution.m_y));
            const gui::GuiSize& currentViewportSize = guiViewport->getSize();
            if (currentViewportSize.m_x != viewportSize.m_x || currentViewportSize.m_y != viewportSize.m_y)
            {
                guiViewport->setSize(viewportSize);
            }
        }
    }
}

void ego::engine::Engine::releaseJobController()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_jobController);
}

void ego::engine::Engine::beginFrame()
{
    m_currentFrameTime = Clock::GetCurrentTimePoint();
    m_deltaTime = Clock::CalcTimePointDelta<float>(m_currentFrameTime, m_prevFrameStartTime);
    if (m_guiController)
    {
        m_guiController->beginFrame();
    }
}

void ego::engine::Engine::endFrame()
{
    if (m_guiController)
    {
        m_guiController->endFrame();
    }

    m_prevFrameStartTime = m_currentFrameTime;
}

void ego::engine::Engine::cleanResources()
{
    if (m_render)
    {
        m_render->clearResources();
    }
}

void ego::engine::Engine::renderFrame()
{
    if (m_render)
    {
        m_render->render();
    }
}

void ego::engine::Engine::presentFrame()
{
    if (m_render && m_graphicPresenter)
    {
        m_render->present(*m_graphicPresenter);
    }
}

void ego::engine::Engine::prepareRenderFrame()
{
    syncPresenterTargetResolution();

    if (!m_render || !m_levelController)
    {
        return;
    }

    const LevelPointer activeLevel = m_levelController->getActiveLevel();
    if (activeLevel && m_renderCameraEntity)
    {
        m_render->prepare(*activeLevel, m_renderCameraEntity);
    }
}

ego::engine::Engine& ego::engine::GetEngine()
{
    const EngineContextPointer engineContext = context::FindCurrentContext<EngineContext>();
    EGO_ASSERT(engineContext);

    return engineContext->getEngine();
}
