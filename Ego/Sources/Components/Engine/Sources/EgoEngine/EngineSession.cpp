#include "EngineSession.h"

#include <algorithm>
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

#include "EgoGuiRender/GuiRender.h"
#include "EgoGuiRender/GuiRenderPlugin.h"

#include "Graphic/Presentation/EngineViewportPresentation.h"
#include "Graphic/Render/RenderPlugin.h"
#include "Level/LevelController.h"
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
    EGO_CHECK_RETURN_FALSE(!_initData.m_enableGui || _initData.m_enablePresentation);
    EGO_CHECK_RETURN_FALSE(!m_jobController);
    EGO_CHECK_RETURN_FALSE(!m_engineLogic);
    EGO_CHECK_RETURN_FALSE(m_id == InvalidEngineSessionID);

    m_jobController = _jobController;
    m_id = _id;
    m_isGuiEnabled = _initData.m_enableGui;
    m_currentFrameTime = Clock::GetCurrentTimePoint();
    m_prevFrameStartTime = m_currentFrameTime;

    EGO_CHECK_INITIALIZATION(initProject(_initData.m_project));

    m_levelController = new LevelController();
    EGO_CHECK_INITIALIZATION(m_levelController && m_levelController->init());

    EGO_CHECK_INITIALIZATION(initInputController());
    if (_initData.m_enablePresentation)
    {
        EGO_CHECK_INITIALIZATION(initGuiController(_initData.m_presentation, _initData.m_gui, _initData.m_enableGui));
    }

    EGO_CHECK_INITIALIZATION(initRender(_initData));
    if (_initData.m_enablePresentation)
    {
        EGO_CHECK_INITIALIZATION(initPresentation(_initData.m_presentation, _initData.m_gui, _initData.m_enableGui));
    }

    EGO_CHECK_INITIALIZATION(initFrameLogic());
    EGO_CHECK_INITIALIZATION(initEngineLogic());

    return true;
}

void ego::engine::EngineSession::release()
{
    releaseEngineLogic();
    cleanResources();

    m_frameLogic.release();

    releasePresentation();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_render);
    m_renderPlugin = nullptr;
    if (m_guiController)
    {
        m_guiController->release();
        m_guiController = nullptr;
    }
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_inputController);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_levelController);

    releaseProject();

    m_jobController = nullptr;
    m_id = InvalidEngineSessionID;
    m_renderCameraEntity = ecs::Entity();
    m_currentFrameTime = ClockTimePoint();
    m_prevFrameStartTime = ClockTimePoint();
    m_deltaTime = 0.0f;
    m_isRenderFramePrepared = false;
    m_hasRenderedScene = false;
    m_isGuiEnabled = false;
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

    endFrame();

    return true;
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

ego::gui::GuiControllerPointer ego::engine::EngineSession::getGuiControllerPointer() const
{
    return m_isGuiEnabled ? m_guiController : nullptr;
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

bool ego::engine::EngineSession::initGuiController(const PresentationOptions& _presentationOptions, const GuiOptions& _guiOptions, bool _enableGui)
{
    m_guiController = new gui::GuiController();
    EGO_CHECK_RETURN_FALSE(m_guiController);

    gui::GuiController::InitData guiInitData;
    guiInitData.m_primaryViewportDesc = _presentationOptions.m_primaryViewportDesc;
    guiInitData.m_viewportBackend = _presentationOptions.m_viewportBackend;
    if (_enableGui)
    {
        guiInitData.m_fontAtlasDesc = _guiOptions.m_fontAtlasDesc;
        guiInitData.m_theme = _guiOptions.m_theme;
    }
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

bool ego::engine::EngineSession::initPresentation(const PresentationOptions& _presentationOptions, const GuiOptions& _guiOptions, bool _enableGui)
{
    EGO_CHECK_RETURN_FALSE(!m_viewportBackend);
    EGO_CHECK_RETURN_FALSE(!m_guiRender);
    EGO_CHECK_RETURN_FALSE(m_viewportPresentations.empty());

    m_viewportBackend = _presentationOptions.m_viewportBackend;
    EGO_CHECK_RETURN_FALSE(m_viewportBackend);

    if (_enableGui)
    {
        const PluginControllerPointer pluginController = getPluginControllerPointer();
        EGO_CHECK_RETURN_FALSE(pluginController);

        const FileName moduleName = _guiOptions.m_renderPluginModuleName ? _guiOptions.m_renderPluginModuleName : resolvePluginModuleName(gui::GuiRenderPlugin::GetPluginType());
        EGO_CHECK_RETURN_FALSE(moduleName);

        m_guiRenderPlugin = pluginController->loadPlugin<gui::GuiRenderPlugin>(moduleName);
        EGO_CHECK_RETURN_FALSE(m_guiRenderPlugin);

        m_guiRender = m_guiRenderPlugin->createGuiRender(gpu::GetGraphicDevice());
        EGO_CHECK_RETURN_FALSE(m_guiRender);
    }

    return true;
}

void ego::engine::EngineSession::releasePresentation()
{
    m_preparedViewportIDs.clear();
    m_viewportPresentations.clear();
    if (m_guiRender)
    {
        m_guiRender->release();
        m_guiRender = nullptr;
    }

    m_guiRenderPlugin = nullptr;
    m_viewportBackend = nullptr;
    m_primaryViewportID = gui::InvalidViewportID;
}

ego::engine::EngineViewportPrepareResult ego::engine::EngineSession::prepareViewportPresentation(
    gui::ViewportFrame&& _viewportFrame,
    const gui::Frame::ResourceCollection& _resources)
{
    const gui::ViewportID viewportID = _viewportFrame.m_viewportID;
    const EngineViewportHostPointer host = m_viewportBackend ? m_viewportBackend->findViewportHost(viewportID) : nullptr;
    if (!host)
    {
        return EngineViewportPrepareResult::Unavailable;
    }

    ViewportPresentationMap::iterator presentationIt = m_viewportPresentations.find(viewportID);
    if (presentationIt != m_viewportPresentations.end() && !presentationIt->second->matchesHost(host))
    {
        presentationIt->second->wait();
        if (m_guiRender)
        {
            m_guiRender->removeViewport(viewportID);
        }

        presentationIt = m_viewportPresentations.erase(presentationIt);
    }

    if (presentationIt == m_viewportPresentations.end())
    {
        std::unique_ptr<EngineViewportPresentation> presentation = std::make_unique<EngineViewportPresentation>();
        if (!presentation || !presentation->init(host))
        {
            return EngineViewportPrepareResult::Failed;
        }

        presentationIt = m_viewportPresentations.emplace(viewportID, std::move(presentation)).first;
    }

    EngineViewportPresentation& presentation = *presentationIt->second;
    EngineViewportPrepareResult prepareResult = presentation.prepare();
    if (prepareResult == EngineViewportPrepareResult::TargetResizeRequired)
    {
        prepareResult = presentation.resizeTarget() ? EngineViewportPrepareResult::Ready : EngineViewportPrepareResult::Failed;
    }

    if (prepareResult != EngineViewportPrepareResult::Ready || !m_guiRender)
    {
        return prepareResult;
    }

    gui::GuiRenderPacket packet;
    packet.m_viewportID = viewportID;
    packet.m_frameIndex = presentation.getFrameIndex();
    packet.m_drawData = std::move(_viewportFrame.m_drawData);
    packet.m_imageBindings = _resources;
    return m_guiRender->prepare(gpu::GetGraphicDevice(), std::move(packet)) ? EngineViewportPrepareResult::Ready : EngineViewportPrepareResult::Failed;
}

void ego::engine::EngineSession::removeUnusedViewportPresentations(const gui::ViewportIDCollection& _viewportIDs)
{
    for (ViewportPresentationMap::iterator presentationIt = m_viewportPresentations.begin(); presentationIt != m_viewportPresentations.end();)
    {
        const gui::ViewportID viewportID = presentationIt->first;
        if (std::find(_viewportIDs.begin(), _viewportIDs.end(), viewportID) != _viewportIDs.end())
        {
            ++presentationIt;
            continue;
        }

        if (m_guiRender)
        {
            if (presentationIt->second)
            {
                presentationIt->second->wait();
            }

            m_guiRender->removeViewport(viewportID);
        }

        presentationIt = m_viewportPresentations.erase(presentationIt);
    }
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
    for (const ViewportPresentationMap::value_type& presentationEntry : m_viewportPresentations)
    {
        if (presentationEntry.second)
        {
            presentationEntry.second->wait();
        }
    }

    if (m_guiRender)
    {
        m_guiRender->clearResources();
    }

    if (m_render)
    {
        m_render->clearResources();
    }

    m_isRenderFramePrepared = false;
    m_hasRenderedScene = false;
}

void ego::engine::EngineSession::renderFrame()
{
    m_hasRenderedScene = m_isRenderFramePrepared;
    if (m_render && m_isRenderFramePrepared)
    {
        m_render->render();
    }

    m_isRenderFramePrepared = false;
}

void ego::engine::EngineSession::presentFrame()
{
    for (const gui::ViewportID viewportID : m_preparedViewportIDs)
    {
        const ViewportPresentationMap::iterator presentationIt = m_viewportPresentations.find(viewportID);
        if (presentationIt == m_viewportPresentations.end() || !presentationIt->second)
        {
            continue;
        }

        EngineViewportPresentation& presentation = *presentationIt->second;
        const gpu::Texture2DReference sceneTexture = viewportID == m_primaryViewportID && m_hasRenderedScene && m_render ? m_render->getResultTexture() : nullptr;
        const bool presentResult = presentation.present(m_guiRender, viewportID, sceneTexture);
        EGO_ASSERT(presentResult);
    }

    m_hasRenderedScene = false;
}

void ego::engine::EngineSession::prepareRenderFrame()
{
    m_preparedViewportIDs.clear();
    m_primaryViewportID = gui::InvalidViewportID;

    if (m_guiController)
    {
        gui::Frame guiFrame = m_guiController->buildFrame();
        m_primaryViewportID = guiFrame.m_primaryViewportID;

        gui::ViewportIDCollection viewportIDs;
        viewportIDs.reserve(guiFrame.m_viewports.size());
        for (const gui::ViewportFrame& viewportFrame : guiFrame.m_viewports)
        {
            viewportIDs.push_back(viewportFrame.m_viewportID);
        }
        removeUnusedViewportPresentations(viewportIDs);

        for (gui::ViewportFrame& viewportFrame : guiFrame.m_viewports)
        {
            const gui::ViewportID viewportID = viewportFrame.m_viewportID;
            const EngineViewportPrepareResult prepareResult = prepareViewportPresentation(std::move(viewportFrame), guiFrame.m_resources);
            if (prepareResult == EngineViewportPrepareResult::Ready)
            {
                m_preparedViewportIDs.push_back(viewportID);
            }
            else if (prepareResult == EngineViewportPrepareResult::Failed && viewportID == m_primaryViewportID)
            {
                EGO_ASSERT_FAIL_MESSAGE("Failed to prepare the primary viewport for presentation.");
            }
        }
    }

    if (m_render && m_levelController)
    {
        const LevelPointer activeLevel = m_levelController->getActiveLevel();
        const ViewportPresentationMap::const_iterator primaryPresentationIt = m_viewportPresentations.find(m_primaryViewportID);
        if (primaryPresentationIt != m_viewportPresentations.end() && primaryPresentationIt->second)
        {
            const gpu::Texture2DReference renderTarget = primaryPresentationIt->second->getTargetTexture();
            if (renderTarget)
            {
                m_render->setResolution(renderTarget->getDesc().m_size);
            }
        }

        m_isRenderFramePrepared = false;
        if (activeLevel && m_renderCameraEntity)
        {
            const render::RenderPrepareContext prepareContext{*activeLevel, m_renderCameraEntity, getDeltaTime()};
            m_isRenderFramePrepared = m_render->prepare(prepareContext);
            EGO_ASSERT(m_isRenderFramePrepared);
        }
    }
}
