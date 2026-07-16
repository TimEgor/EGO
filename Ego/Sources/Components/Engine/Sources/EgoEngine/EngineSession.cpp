#include "EngineSession.h"

#include <utility>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/Platform/FileSystem/RootedFileSystem.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/Profile/Profile.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoPlugin/Catalog/PluginCatalog.h"
#include "EgoPlugin/Catalog/PluginCatalogBuilder.h"
#include "EgoPlugin/Plugin.h"
#include "EgoPlugin/PluginController.h"
#include "EgoPlugin/PluginSubsystem.h"

#include "EgoResource/ResourceController.h"
#include "EgoResource/ResourceSubsystem.h"

#include "EgoJob/JobController.h"
#include "EgoJob/JobDescriptor.h"

#include "EgoInput/InputController.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "Graphic/Render/RenderPlugin.h"
#include "Level/LevelController.h"
#include "Project/EngineLogic.h"
#include "Project/EngineLogicPlugin.h"

namespace
{
    constexpr const char* EngineGuiFontPath = "C:/Windows/Fonts/segoeui.ttf";
} // namespace

ego::engine::EngineSession::~EngineSession()
{
    release();
}

bool ego::engine::EngineSession::init(const JobControllerPointer& _jobController, EngineSessionID _id, const InitData& _initData)
{
    EGO_CHECK_RETURN_FALSE(_jobController);
    EGO_CHECK_RETURN_FALSE(_id != InvalidEngineSessionID);
    EGO_CHECK_RETURN_FALSE(_initData.m_guiViewportBackend);
    EGO_CHECK_RETURN_FALSE(!m_jobController);
    EGO_CHECK_RETURN_FALSE(!m_engineLogic);
    EGO_CHECK_RETURN_FALSE(m_id == InvalidEngineSessionID);

    m_jobController = _jobController;
    m_id = _id;
    m_currentFrameTime = Clock::GetCurrentTimePoint();
    m_prevFrameStartTime = m_currentFrameTime;

    EGO_CHECK_INITIALIZATION(initProject(_initData.m_project));

    m_levelController = new LevelController();
    EGO_CHECK_INITIALIZATION(m_levelController && m_levelController->init());

    EGO_CHECK_INITIALIZATION(initInputController());
    EGO_CHECK_INITIALIZATION(initGuiController(_initData));

    EGO_CHECK_INITIALIZATION(initRender(_initData));
    EGO_CHECK_INITIALIZATION(initGuiRenderCoordinator(_initData));

    EGO_CHECK_INITIALIZATION(initFrameLogic());
    EGO_CHECK_INITIALIZATION(initEngineLogic());

    return true;
}

void ego::engine::EngineSession::release()
{
    releaseEngineLogic();
    cleanResources();

    m_frameLogic.release();

    releaseGuiRenderCoordinator();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_render);
    m_renderPlugin = nullptr;
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_guiController);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_inputController);
    m_guiViewportBackend = nullptr;
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_levelController);

    releaseProject();

    m_jobController = nullptr;
    m_id = InvalidEngineSessionID;
    m_renderCameraEntity = ecs::Entity();
    m_currentFrameTime = ClockTimePoint();
    m_prevFrameStartTime = ClockTimePoint();
    m_deltaTime = 0.0f;
    m_frameSucceeded.store(true);
}

bool ego::engine::EngineSession::initProject(const ProjectPointer& _project)
{
    const PluginSubsystemPointer pluginSubsystem = subsystem::FindSubsystem<PluginSubsystem>();
    EGO_CHECK_RETURN_FALSE(pluginSubsystem);

    if (!_project)
    {
        return true;
    }

    EGO_CHECK_RETURN_FALSE(buildProjectPluginCatalog(*_project));
    EGO_CHECK_RETURN_FALSE(registerProjectAssetFileSystems(*_project));
    EGO_CHECK_RETURN_FALSE(loadProjectPlugins(*_project));
    EGO_CHECK_RETURN_FALSE(loadProjectEngineLogicPlugin(*_project));

    return true;
}

void ego::engine::EngineSession::releaseProject()
{
    if (m_engineLogicPlugin || !m_projectPlugins.empty() || !m_projectAssetFileSystems.empty())
    {
        const ResourceControllerPointer resourceController = getResourceControllerPointer();
        if (resourceController)
        {
            resourceController->waitAllLoading();
        }
    }

    m_engineLogicPlugin = nullptr;

    while (!m_projectPlugins.empty())
    {
        m_projectPlugins.pop_back();
    }

    releaseProjectAssetFileSystems();

    m_projectPluginCatalog.clear();
}

ego::FileSystemPointer ego::engine::EngineSession::getFileSystemPointer() const
{
    const PlatformSubsystemPointer platformSubsystem = subsystem::FindSubsystem<PlatformSubsystem>();
    const PlatformPointer platform = platformSubsystem ? platformSubsystem->getPlatformPointer() : nullptr;
    return platform ? platform->getFileSystem() : nullptr;
}

ego::PluginControllerPointer ego::engine::EngineSession::getPluginControllerPointer() const
{
    const PluginSubsystemPointer pluginSubsystem = subsystem::FindSubsystem<PluginSubsystem>();
    return pluginSubsystem ? pluginSubsystem->getPluginControllerPointer() : nullptr;
}

ego::ResourceControllerPointer ego::engine::EngineSession::getResourceControllerPointer() const
{
    const ResourceSubsystemPointer resourceSubsystem = subsystem::FindSubsystem<ResourceSubsystem>();
    return resourceSubsystem ? resourceSubsystem->getResourceControllerPointer() : nullptr;
}

bool ego::engine::EngineSession::buildProjectPluginCatalog(const Project& _project)
{
    const FileSystemPointer fileSystem = getFileSystemPointer();
    EGO_CHECK_RETURN_FALSE(fileSystem);

    for (const FileName& pluginDirectory : _project.getPluginDirectories())
    {
        PluginCatalogBuilder::Options options;
        options.m_mode = PluginCatalogBuilder::Mode::BestEffort;
        const PluginCatalog::RegistrationID registrationID = PluginCatalogBuilder::AddPluginsFromPath(m_projectPluginCatalog, *fileSystem, pluginDirectory, options);

        if (registrationID == PluginCatalog::InvalidRegistrationID)
        {
            return false;
        }
    }

    return true;
}

bool ego::engine::EngineSession::registerProjectAssetFileSystems(const Project& _project)
{
    const ResourceControllerPointer resourceController = getResourceControllerPointer();
    const FileSystemPointer sourceFileSystem = getFileSystemPointer();
    EGO_CHECK_RETURN_FALSE(resourceController && sourceFileSystem);

    for (const FileName& assetDirectory : _project.getAssetDirectories())
    {
        FileSystemPointer assetFileSystem = createProjectAssetFileSystem(sourceFileSystem, assetDirectory);
        EGO_CHECK_RETURN_FALSE(assetFileSystem);

        resourceController->addFileSystem(assetFileSystem);
        m_projectAssetFileSystems.push_back(assetFileSystem);
    }

    return true;
}

void ego::engine::EngineSession::releaseProjectAssetFileSystems()
{
    const ResourceControllerPointer resourceController = getResourceControllerPointer();

    while (!m_projectAssetFileSystems.empty())
    {
        const FileSystemPointer assetFileSystem = m_projectAssetFileSystems.back();
        m_projectAssetFileSystems.pop_back();
        if (!assetFileSystem)
        {
            continue;
        }

        if (resourceController)
        {
            resourceController->removeFileSystem(assetFileSystem);
        }

        assetFileSystem->release();
    }
}

ego::FileSystemPointer ego::engine::EngineSession::createProjectAssetFileSystem(const FileSystemPointer& _sourceFileSystem, const FileName& _rootPath) const
{
    RootedFileSystemPointer fileSystem = new RootedFileSystem(_sourceFileSystem, _rootPath);
    return fileSystem && fileSystem->init() ? fileSystem : nullptr;
}

bool ego::engine::EngineSession::loadProjectPlugins(const Project& _project)
{
    for (const Project::PluginDesc& pluginDesc : _project.getPlugins())
    {
        EGO_CHECK_RETURN_FALSE(loadProjectPlugin(pluginDesc));
    }

    return true;
}

bool ego::engine::EngineSession::loadProjectPlugin(const Project::PluginDesc& _pluginDesc)
{
    const FileName moduleName = resolveProjectPluginModuleName(_pluginDesc);
    EGO_CHECK_RETURN_FALSE(moduleName);

    const PluginControllerPointer pluginController = getPluginControllerPointer();
    EGO_CHECK_RETURN_FALSE(pluginController);

    const PluginPointer plugin = pluginController->loadPlugin(moduleName, _pluginDesc.m_typeName.c_str());
    EGO_CHECK_RETURN_FALSE(plugin);

    m_projectPlugins.push_back(plugin);
    return true;
}

bool ego::engine::EngineSession::loadProjectEngineLogicPlugin(const Project& _project)
{
    if (!_project.getEngineLogicPlugin().has_value())
    {
        return true;
    }

    const Project::PluginDesc& pluginDesc = _project.getEngineLogicPlugin().value();
    const FileName moduleName = resolveProjectPluginModuleName(pluginDesc);
    EGO_CHECK_RETURN_FALSE(moduleName);

    const PluginControllerPointer pluginController = getPluginControllerPointer();
    EGO_CHECK_RETURN_FALSE(pluginController);

    m_engineLogicPlugin = pluginController->loadPlugin<EngineLogicPlugin>(moduleName);
    return static_cast<bool>(m_engineLogicPlugin);
}

ego::FileName ego::engine::EngineSession::resolveProjectPluginModuleName(const Project::PluginDesc& _pluginDesc) const
{
    if (_pluginDesc.m_moduleName)
    {
        return _pluginDesc.m_moduleName;
    }

    if (_pluginDesc.m_typeName.empty() || _pluginDesc.m_name.empty())
    {
        return FileName();
    }

    const PluginType pluginType = GetPluginType(_pluginDesc.m_typeName.c_str());
    return resolvePluginModuleName(pluginType, _pluginDesc.m_name);
}

ego::FileName ego::engine::EngineSession::resolvePluginModuleName(PluginType _pluginType) const
{
    const FileName projectModuleName = m_projectPluginCatalog.resolve(_pluginType);
    if (projectModuleName)
    {
        return projectModuleName;
    }

    const PluginSubsystemPointer pluginSubsystem = subsystem::FindSubsystem<PluginSubsystem>();
    return pluginSubsystem ? pluginSubsystem->getPluginCatalog().resolve(_pluginType) : FileName();
}

ego::FileName ego::engine::EngineSession::resolvePluginModuleName(PluginType _pluginType, std::string_view _pluginName) const
{
    const FileName projectModuleName = m_projectPluginCatalog.resolve(_pluginType, _pluginName);
    if (projectModuleName)
    {
        return projectModuleName;
    }

    const PluginSubsystemPointer pluginSubsystem = subsystem::FindSubsystem<PluginSubsystem>();
    return pluginSubsystem ? pluginSubsystem->getPluginCatalog().resolve(_pluginType, _pluginName) : FileName();
}

bool ego::engine::EngineSession::tick()
{
    EGO_CHECK_RETURN_FALSE(m_id != InvalidEngineSessionID);
    EGO_CHECK_RETURN_FALSE(m_inputController && m_jobController);

    EGO_PROFILE_BLOCK_EVENT("Frame");

    beginFrame();

    m_inputController->update();
    if (m_guiController)
    {
        m_guiController->update();
    }

    JobGraphReference frameLogicJobGraph = getFrameLogicJobGraph();
    if (!frameLogicJobGraph)
    {
        EGO_ASSERT_FAIL_MESSAGE("Frame logic job graph is invalid.");
        endFrame();
        return false;
    }

    EGO_ASSERT(m_jobController);
    m_jobController->addJobGraph(frameLogicJobGraph);
    m_jobController->waitAndExecute(frameLogicJobGraph);

    const bool frameSucceeded = m_frameSucceeded.load();
    endFrame();

    return frameSucceeded;
}

ego::engine::EngineSessionID ego::engine::EngineSession::getID() const
{
    return m_id;
}

ego::LevelController& ego::engine::EngineSession::getLevelController()
{
    EGO_ASSERT(m_levelController);
    return *m_levelController;
}

ego::render::Render& ego::engine::EngineSession::getRender()
{
    EGO_ASSERT(m_render);
    return *m_render;
}

void ego::engine::EngineSession::setRenderCameraEntity(ecs::Entity _cameraEntity)
{
    m_renderCameraEntity = _cameraEntity;
}

void ego::engine::EngineSession::clearRenderCameraEntity()
{
    m_renderCameraEntity = ecs::Entity();
}

ego::gui::GuiController& ego::engine::EngineSession::getGuiController()
{
    EGO_ASSERT(m_guiController);
    return *m_guiController;
}

ego::gui::GuiControllerPointer ego::engine::EngineSession::getGuiControllerPointer() const
{
    return m_guiController;
}

ego::InputControllerPointer ego::engine::EngineSession::getInputControllerPointer() const
{
    return m_inputController;
}

ego::JobGraphReference ego::engine::EngineSession::getFrameLogicJobGraph()
{
    return m_frameLogic.createJobGraph();
}

bool ego::engine::EngineSession::initInputController()
{
    m_inputController = new InputController();
    EGO_CHECK_RETURN_FALSE(m_inputController && m_inputController->init());

    return true;
}

bool ego::engine::EngineSession::loadDefaultGuiFont(gui::GuiFontAtlasDesc& _fontAtlasDesc) const
{
    const PlatformPointer platform = GetPlatformPointer();
    const FileSystemPointer fileSystem = platform ? platform->getFileSystem() : nullptr;
    EGO_CHECK_RETURN_FALSE(fileSystem && fileSystem->readFile(EngineGuiFontPath, _fontAtlasDesc.m_fontData));

    _fontAtlasDesc.m_pixelHeight = 16.0f;
    _fontAtlasDesc.m_width = 512;
    _fontAtlasDesc.m_height = 512;
    _fontAtlasDesc.m_firstCharacter = ' ';
    _fontAtlasDesc.m_characterCount = 95;
    return true;
}

bool ego::engine::EngineSession::initGuiController(const InitData& _initData)
{
    m_guiController = new gui::GuiController();
    EGO_CHECK_RETURN_FALSE(m_guiController);

    gui::GuiController::InitData guiInitData;
    guiInitData.m_primaryViewportDesc = _initData.m_primaryGuiViewportDesc;
    guiInitData.m_viewportBackend = _initData.m_guiViewportBackend;
    EGO_CHECK_RETURN_FALSE(loadDefaultGuiFont(guiInitData.m_fontAtlasDesc));
    EGO_CHECK_RETURN_FALSE(m_guiController->init(guiInitData));

    return true;
}

bool ego::engine::EngineSession::initRender(const InitData& _initData)
{
    EGO_CHECK_RETURN_FALSE(!m_renderPlugin);
    EGO_CHECK_RETURN_FALSE(!m_render);

    const PluginControllerPointer pluginController = getPluginControllerPointer();
    EGO_CHECK_RETURN_FALSE(pluginController);

    const FileName moduleName = _initData.m_renderPluginModuleName ? _initData.m_renderPluginModuleName : resolvePluginModuleName(render::RenderPlugin::GetPluginType());
    EGO_CHECK_RETURN_FALSE(moduleName);

    m_renderPlugin = pluginController->loadPlugin<render::RenderPlugin>(moduleName);

    EGO_CHECK_RETURN_FALSE(m_renderPlugin);

    m_render = m_renderPlugin->createRender();
    EGO_CHECK_RETURN_FALSE(m_render && m_render->init());

    return true;
}

bool ego::engine::EngineSession::initGuiRenderCoordinator(const InitData& _initData)
{
    m_guiViewportBackend = _initData.m_guiViewportBackend;
    EGO_CHECK_RETURN_FALSE(m_guiViewportBackend);

    const PluginControllerPointer pluginController = getPluginControllerPointer();
    EGO_CHECK_RETURN_FALSE(pluginController);

    const FileName moduleName = _initData.m_guiRenderPluginModuleName ? _initData.m_guiRenderPluginModuleName : resolvePluginModuleName(gui::GuiRenderPlugin::GetPluginType());
    EGO_CHECK_RETURN_FALSE(moduleName);

    m_guiRenderPlugin = pluginController->loadPlugin<gui::GuiRenderPlugin>(moduleName);
    EGO_CHECK_RETURN_FALSE(m_guiRenderPlugin);

    const gui::GuiRenderPointer guiRender = m_guiRenderPlugin->createGuiRender(gpu::GetGraphicDevice());
    EGO_CHECK_RETURN_FALSE(guiRender);
    return m_guiRenderCoordinator.init(m_guiViewportBackend, guiRender);
}

void ego::engine::EngineSession::releaseGuiRenderCoordinator()
{
    m_guiRenderCoordinator.release();
    m_guiRenderPlugin = nullptr;
}

bool ego::engine::EngineSession::initFrameLogic()
{
    return m_frameLogic.init(
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

bool ego::engine::EngineSession::initEngineLogic()
{
    EGO_CHECK_RETURN_FALSE(!m_engineLogic);

    if (!m_engineLogicPlugin)
    {
        return true;
    }

    EngineLogicPointer engineLogic = m_engineLogicPlugin->createEngineLogic();
    EGO_CHECK_RETURN_FALSE(engineLogic);

    EngineLogic::InitData initData;
    initData.m_engineSession = weakFromThis();
    initData.m_resourceController = getResourceControllerPointer();
    if (!engineLogic->init(initData))
    {
        engineLogic->release();
        return false;
    }

    m_engineLogic = engineLogic;
    return registerEngineLogicFrameLogicJob();
}

void ego::engine::EngineSession::releaseEngineLogic()
{
    unregisterEngineLogicFrameLogicJob();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_engineLogic);
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
    m_frameSucceeded.store(true);
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

void ego::engine::EngineSession::cleanResources()
{
    m_guiRenderCoordinator.clearResources();

    if (m_render)
    {
        m_render->clearResources();
    }
}

void ego::engine::EngineSession::renderFrame()
{
    if (m_render)
    {
        m_render->render();
    }
}

void ego::engine::EngineSession::presentFrame()
{
    if (!m_render)
    {
        return;
    }

    if (m_guiRenderCoordinator.isInitialized())
    {
        const bool presentResult = m_guiRenderCoordinator.renderAndPresent(*m_render);
        EGO_ASSERT(presentResult);
        if (!presentResult)
        {
            m_frameSucceeded.store(false);
        }
        return;
    }
}

void ego::engine::EngineSession::prepareRenderFrame()
{
    if (!m_guiController)
    {
        return;
    }

    gui::GuiFrame guiFrame = m_guiController->buildFrame();
    const gui::GuiViewportID primaryViewportID = guiFrame.m_primaryViewportID;

    if (m_guiRenderCoordinator.isInitialized())
    {
        const bool prepareResult = m_guiRenderCoordinator.prepare(std::move(guiFrame));
        EGO_ASSERT(prepareResult);
        if (!prepareResult)
        {
            m_frameSucceeded.store(false);
        }
    }

    if (m_render && m_levelController)
    {
        const LevelPointer activeLevel = m_levelController->getActiveLevel();
        const EngineViewportHostPointer primaryHost = m_guiViewportBackend->findViewportHost(primaryViewportID);
        const GraphicPresenterPointer graphicPresenter = primaryHost ? primaryHost->getGraphicPresenterPointer() : nullptr;
        if (graphicPresenter)
        {
            const gpu::Texture2DReference targetTexture = graphicPresenter->getTargetTexture();
            if (targetTexture)
            {
                m_render->setResolution(targetTexture->getDesc().m_size);
            }
        }

        if (activeLevel && m_renderCameraEntity)
        {
            const render::RenderPrepareContext prepareContext{*activeLevel, m_renderCameraEntity, getDeltaTime()};
            const bool prepareResult = m_render->prepare(prepareContext);
            EGO_ASSERT(prepareResult);
            if (!prepareResult)
            {
                m_frameSucceeded.store(false);
            }
        }
    }
}
