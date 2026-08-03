#include "EngineSession.h"

#include <utility>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Profile/Profile.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoPlugin/PluginController.h"
#include "EgoPlugin/PluginSubsystem.h"

#include "EgoJob/JobController.h"
#include "EgoJob/JobDescriptor.h"

#include "EgoGui/Rendering/GuiRenderPlugin.h"

#include "EgoApplication/Engine/Gui/ApplicationGuiViewportProvider.h"

#include "Graphic/SceneRender/RenderPlugin.h"
#include "Project/EngineLogic.h"
#include "Project/EngineLogicPlugin.h"

ego::engine::EngineSession::EngineSession() = default;

ego::engine::EngineSession::~EngineSession()
{
    release();
}

bool ego::engine::EngineSession::init(const JobControllerPointer& _jobController, EngineSessionID _id, const InitData& _initData)
{
    EGO_CHECK_RETURN_FALSE(_jobController);
    EGO_CHECK_RETURN_FALSE(_id != InvalidEngineSessionID);

    const application::Presentation& mainPresentation = _initData.m_mainPresentation;
    const bool hasMainSurface = static_cast<bool>(mainPresentation.m_surface);
    const bool hasMainGraphicPresenter = static_cast<bool>(mainPresentation.m_graphicPresenter);
    EGO_CHECK_RETURN_FALSE(!hasMainSurface || hasMainGraphicPresenter);
    EGO_CHECK_RETURN_FALSE(!_initData.m_gui.m_isEnabled || hasMainSurface);
    EGO_CHECK_RETURN_FALSE(!_initData.m_sceneRender.m_isEnabled || hasMainGraphicPresenter);

    EGO_CHECK_RETURN_FALSE(!m_jobController);
    EGO_CHECK_RETURN_FALSE(!m_engineLogic);
    EGO_CHECK_RETURN_FALSE(!m_activeLevel);
    EGO_CHECK_RETURN_FALSE(!m_scenePresenter);
    EGO_CHECK_RETURN_FALSE(!m_guiViewportProvider);
    EGO_CHECK_RETURN_FALSE(m_id == InvalidEngineSessionID);

    m_jobController = _jobController;
    m_scenePresenter = mainPresentation.m_graphicPresenter;
    m_id = _id;
    m_isGuiEnabled = _initData.m_gui.m_isEnabled;
    m_currentFrameTime = Clock::GetCurrentTimePoint();
    m_prevFrameStartTime = m_currentFrameTime;

    EGO_CHECK_INITIALIZATION(m_projectRuntime.init(_initData.m_project));

    if (_initData.m_gui.m_isEnabled)
    {
        EGO_CHECK_INITIALIZATION(initGuiController(mainPresentation));
    }
    EGO_CHECK_INITIALIZATION(initGraphicFrameController(_initData));

    EGO_CHECK_INITIALIZATION(initFrameLogic());
    EGO_CHECK_INITIALIZATION(initEngineLogic());

    return true;
}

void ego::engine::EngineSession::release()
{
    releaseEngineLogic();

    m_frameLogic.release();

    m_graphicFrameController.release();
    m_guiController = nullptr;
    m_guiViewportProvider = nullptr;
    m_activeLevel = nullptr;

    m_projectRuntime.release();

    m_scenePresenter = nullptr;
    m_jobController = nullptr;
    m_id = InvalidEngineSessionID;
    m_renderCameraEntity = ecs::Entity();
    m_currentFrameTime = ClockTimePoint();
    m_prevFrameStartTime = ClockTimePoint();
    m_deltaTime = 0.0f;
    m_isGuiEnabled = false;
}

ego::PluginControllerPointer ego::engine::EngineSession::getPluginControllerPointer() const
{
    const PluginSubsystemPointer pluginSubsystem = subsystem::FindSubsystem<PluginSubsystem>();

    return pluginSubsystem ? pluginSubsystem->getPluginControllerPointer() : nullptr;
}

bool ego::engine::EngineSession::tick()
{
    EGO_CHECK_RETURN_FALSE(m_id != InvalidEngineSessionID);
    EGO_CHECK_RETURN_FALSE(m_jobController);

    EGO_PROFILE_BLOCK_EVENT("Frame");

    beginFrame();

    if (m_guiController)
    {
        m_guiController->update(getDeltaTime());
    }

    JobGraphPointer frameLogicJobGraph = getFrameLogicJobGraph();
    if (!frameLogicJobGraph)
    {
        EGO_ASSERT_FAIL_MESSAGE("Frame logic job graph is invalid.");
        endFrame();

        return false;
    }

    EGO_ASSERT(m_jobController);
    m_jobController->addJobGraph(frameLogicJobGraph);
    m_jobController->waitAndExecute(frameLogicJobGraph);

    endFrame();

    return true;
}

ego::engine::EngineSessionID ego::engine::EngineSession::getID() const
{
    return m_id;
}

ego::LevelPointer ego::engine::EngineSession::getActiveLevel() const
{
    return m_activeLevel;
}

bool ego::engine::EngineSession::setActiveLevel(const LevelPointer& _level)
{
    if (!_level || !_level->isValid())
    {
        return false;
    }

    m_activeLevel = _level;

    return true;
}

void ego::engine::EngineSession::clearActiveLevel()
{
    m_activeLevel = nullptr;
}

ego::render::Render& ego::engine::EngineSession::getRender()
{
    return m_graphicFrameController.getRender();
}

void ego::engine::EngineSession::setRenderCameraEntity(ecs::Entity _cameraEntity)
{
    m_renderCameraEntity = _cameraEntity;
}

void ego::engine::EngineSession::clearRenderCameraEntity()
{
    m_renderCameraEntity = ecs::Entity();
}

ego::gui::GuiControllerPointer ego::engine::EngineSession::getGuiControllerPointer() const
{
    return m_isGuiEnabled ? m_guiController : nullptr;
}

ego::JobGraphPointer ego::engine::EngineSession::getFrameLogicJobGraph()
{
    return m_frameLogic.createJobGraph();
}

bool ego::engine::EngineSession::initGuiController(const application::Presentation& _mainPresentation)
{
    EGO_CHECK_RETURN_FALSE(!m_guiViewportProvider && !m_guiController);

    m_guiViewportProvider = MakePointer<application::ApplicationGuiViewportProvider>();
    EGO_CHECK_RETURN_FALSE(m_guiViewportProvider && m_guiViewportProvider->init(_mainPresentation));

    m_guiController = MakePointer<gui::GuiController>();
    EGO_CHECK_RETURN_FALSE(m_guiController);

    gui::GuiController::InitData guiInitData;
    guiInitData.m_viewportProvider = m_guiViewportProvider;
    EGO_CHECK_RETURN_FALSE(m_guiController->init(guiInitData));

    return true;
}

bool ego::engine::EngineSession::initGraphicFrameController(const InitData& _initData)
{
    GraphicFrameController::InitData graphicInitData;
    graphicInitData.m_enablePresentation = static_cast<bool>(_initData.m_mainPresentation.m_graphicPresenter);

    if (_initData.m_sceneRender.m_isEnabled || _initData.m_gui.m_isEnabled)
    {
        const PluginControllerPointer pluginController = getPluginControllerPointer();
        EGO_CHECK_RETURN_FALSE(pluginController);

        if (_initData.m_sceneRender.m_isEnabled)
        {
            const FileName moduleName = _initData.m_sceneRender.m_pluginModuleName ?
                                            _initData.m_sceneRender.m_pluginModuleName :
                                            m_projectRuntime.resolvePluginModuleName(render::RenderPlugin::GetPluginType());
            EGO_CHECK_RETURN_FALSE(moduleName);

            graphicInitData.m_sceneRenderPlugin = pluginController->loadPlugin<render::RenderPlugin>(moduleName);
            EGO_CHECK_RETURN_FALSE(graphicInitData.m_sceneRenderPlugin);
        }
        if (_initData.m_gui.m_isEnabled)
        {
            const FileName moduleName = _initData.m_gui.m_pluginModuleName ? _initData.m_gui.m_pluginModuleName :
                                                                             m_projectRuntime.resolvePluginModuleName(gui::GuiRenderPlugin::GetPluginType());
            EGO_CHECK_RETURN_FALSE(moduleName);

            graphicInitData.m_guiRenderPlugin = pluginController->loadPlugin<gui::GuiRenderPlugin>(moduleName);
            EGO_CHECK_RETURN_FALSE(graphicInitData.m_guiRenderPlugin);
        }
    }

    return m_graphicFrameController.init(graphicInitData);
}

bool ego::engine::EngineSession::initFrameLogic()
{
    return m_frameLogic.init(
        CreateJobDescriptor(
            [this]()
            {
                m_graphicFrameController.renderFrame();
            },
            "Render scene frame"),
        CreateJobDescriptor(
            [this]()
            {
                m_graphicFrameController.presentFrame();
            },
            "Render GUI and present frame"),
        CreateJobDescriptor(
            [this]()
            {
                prepareGraphicFrame();
            },
            "Prepare render frame"));
}

bool ego::engine::EngineSession::initEngineLogic()
{
    EGO_CHECK_RETURN_FALSE(!m_engineLogic);

    const EngineLogicPluginPointer engineLogicPlugin = m_projectRuntime.getEngineLogicPluginPointer();
    if (!engineLogicPlugin)
    {
        return true;
    }

    EngineLogicPointer engineLogic = engineLogicPlugin->createEngineLogic();
    EGO_CHECK_RETURN_FALSE(engineLogic);

    if (!engineLogic->init(weakFromThis()))
    {
        return false;
    }

    m_engineLogic = engineLogic;

    return registerEngineLogicFrameLogicJob();
}

void ego::engine::EngineSession::releaseEngineLogic()
{
    unregisterEngineLogicFrameLogicJob();
    m_engineLogic = nullptr;
}

bool ego::engine::EngineSession::registerEngineLogicFrameLogicJob()
{
    EGO_CHECK_RETURN_FALSE(m_engineLogic);

    const JobDescriptorID frameLogicBeginJobID = m_frameLogic.getFrameLogicBeginJobID();
    const JobDescriptorID frameLogicEndJobID = m_frameLogic.getFrameLogicEndJobID();
    EGO_CHECK_RETURN_FALSE(frameLogicBeginJobID.isValid());
    EGO_CHECK_RETURN_FALSE(frameLogicEndJobID.isValid());

    m_updateEngineLogicJobID = m_frameLogic.addJobBetween(
        CreateJobDescriptor(
            [this]()
            {
                updateEngineLogic();
            },
            "Engine logic update"),
        frameLogicBeginJobID,
        frameLogicEndJobID);

    return m_updateEngineLogicJobID.isValid();
}

void ego::engine::EngineSession::unregisterEngineLogicFrameLogicJob()
{
    if (m_updateEngineLogicJobID.isValid())
    {
        m_frameLogic.removeJob(m_updateEngineLogicJobID);
    }

    m_updateEngineLogicJobID = JobDescriptorID();
}

void ego::engine::EngineSession::updateEngineLogic()
{
    if (m_engineLogic)
    {
        m_engineLogic->update(getDeltaTime());
    }
}

void ego::engine::EngineSession::beginFrame()
{
    m_currentFrameTime = Clock::GetCurrentTimePoint();
    m_deltaTime = Clock::CalcTimePointDelta<float>(m_currentFrameTime, m_prevFrameStartTime);
}

void ego::engine::EngineSession::endFrame()
{
    m_prevFrameStartTime = m_currentFrameTime;
}

float ego::engine::EngineSession::getDeltaTime() const
{
    return m_deltaTime;
}

void ego::engine::EngineSession::prepareGraphicFrame()
{
    gui::GuiRenderData guiRenderData;
    GraphicFrameController::SceneRenderData sceneRenderData;
    sceneRenderData.m_graphicPresenter = m_scenePresenter;
    if (m_guiController)
    {
        guiRenderData = m_guiController->takeRenderData();
    }

    sceneRenderData.m_activeLevel = m_activeLevel;
    sceneRenderData.m_cameraEntity = m_renderCameraEntity;
    sceneRenderData.m_deltaTime = getDeltaTime();
    m_graphicFrameController.prepareFrame(std::move(guiRenderData), sceneRenderData);
}
