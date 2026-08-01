#include "EditorProjectController.h"

#include <string>

#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoECS/Entity.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoEngine/EngineSubsystem.h"
#include "EgoEngine/Graphic/SceneRender/Component/CameraComponent.h"
#include "EgoEngine/Project/ProjectReader.h"

#include "EditorApplication/EditorController.h"
#include "EditorApplication/EditorSubsystem.h"

namespace
{
    constexpr ego::gpu::Texture2DSize SimulationTextureSize(900, 600);
} // namespace

ego::editor::EditorProjectController::~EditorProjectController()
{
    release();
}

bool ego::editor::EditorProjectController::init(const XmlDocument& _config)
{
    release();

    EGO_CHECK_INITIALIZATION(readConfig(_config));

    m_menuLayer = MakePointer<ProjectMenuLayer>(*this);
    EGO_CHECK_INITIALIZATION(m_menuLayer);

    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    EGO_CHECK_INITIALIZATION(editorSubsystem);

    EditorGuiController& editorGuiController = editorSubsystem->getEditorController().getEditorGuiController();
    EGO_CHECK_INITIALIZATION(editorGuiController.registerMenuLayer(m_menuLayer, GuiMenuOrder::Project));

    return true;
}

void ego::editor::EditorProjectController::release()
{
    releaseProjectContext();

    if (m_menuLayer)
    {
        const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
        if (editorSubsystem)
        {
            editorSubsystem->getEditorController().getEditorGuiController().unregisterMenuLayer(m_menuLayer);
        }
    }

    m_menuLayer = nullptr;

    m_simulationRenderPluginModuleName.clear();
}

bool ego::editor::EditorProjectController::isProjectLoaded() const
{
    return m_projectContext.m_simulationSession && m_projectContext.m_project;
}

ego::engine::EngineSessionPointer ego::editor::EditorProjectController::getSimulationSessionPointer() const
{
    return m_projectContext.m_simulationSession;
}

ego::LevelPointer ego::editor::EditorProjectController::getCurrentLevelPointer() const
{
    return m_projectContext.m_simulationLevel;
}

void ego::editor::EditorProjectController::loadProject()
{
    const FileName projectFileName = selectProjectFile();
    EGO_CHECK_RETURN(projectFileName);

    if (isProjectLoaded())
    {
        unloadProject();
    }

    initProjectContext(projectFileName);
}

void ego::editor::EditorProjectController::unloadProject()
{
    releaseProjectContext();
}

bool ego::editor::EditorProjectController::readConfig(const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(!m_simulationRenderPluginModuleName);

    const XmlNode rootNode = _config.getRootNode();
    EGO_CHECK_RETURN_FALSE(rootNode && rootNode.getNameView() == "Editor");

    const XmlNode engineNode = rootNode.getChild("Engine");
    EGO_CHECK_RETURN_FALSE(engineNode);

    m_simulationRenderPluginModuleName = FileName(engineNode.getChildValueOr<std::string>("SimulationRenderPlugin", std::string()));

    return static_cast<bool>(m_simulationRenderPluginModuleName);
}

ego::FileName ego::editor::EditorProjectController::selectProjectFile() const
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN_VALUE(platform, FileName());

    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    EGO_CHECK_RETURN_VALUE(editorSubsystem, FileName());

    const Platform::OpenFileDialogFilter filters[] = {{"EGO Project (*.xml)", "*.xml"}, {"All Files (*.*)", "*.*"}};

    Platform::OpenFileDialogParams params;
    params.m_title = "Select EGO project";
    params.m_defaultExtension = "xml";
    params.m_filters = filters;
    params.m_filterCount = sizeof(filters) / sizeof(filters[0]);

    const PlatformSurfacePointer mainSurface = editorSubsystem->getEditorController().getMainSurfacePointer();
    params.m_ownerWindowHandle = mainSurface ? mainSurface->getNativeHandle() : nullptr;

    return platform->selectOpenFile(params);
}

bool ego::editor::EditorProjectController::initProjectContext(const FileName& _projectFileName)
{
    EGO_CHECK_RETURN_FALSE(m_simulationRenderPluginModuleName && _projectFileName);

    engine::ProjectPointer project = MakePointer<engine::Project>();
    EGO_CHECK_RETURN_FALSE(project && engine::ProjectReader::ReadFromFile(_projectFileName, *project));

    m_projectContext.m_project = project;

    EGO_CHECK_RETURN_CALL_FALSE(initSimulationGraphicPresenter(), releaseProjectContext());
    EGO_CHECK_RETURN_CALL_FALSE(initSimulationSession(), releaseProjectContext());

    return true;
}

bool ego::editor::EditorProjectController::initSimulationGraphicPresenter()
{
    TextureGraphicPresenterPointer graphicPresenter = MakePointer<TextureGraphicPresenter>();
    if (!graphicPresenter || !graphicPresenter->init(gpu::GetGraphicDevice(), SimulationTextureSize, gpu::GraphicResourceFormat::R8G8B8A8UNorm))
    {
        EGO_SAFE_RESET_POINTER_WITH_RELEASING(graphicPresenter);

        return false;
    }

    const gpu::TextureViewPointer sceneTexture = graphicPresenter->getTextureView();
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    if (!sceneTexture || !editorSubsystem)
    {
        EGO_SAFE_RESET_POINTER_WITH_RELEASING(graphicPresenter);

        return false;
    }

    m_projectContext.m_simulationGraphicPresenter = graphicPresenter;
    editorSubsystem->getEditorController().getEditorGuiController().setSceneTexture(sceneTexture);

    return true;
}

bool ego::editor::EditorProjectController::initSimulationSession()
{
    const engine::EngineSubsystemPointer engineSubsystem = engine::GetEngineSubsystemPointer();
    const engine::EnginePointer engine = engineSubsystem ? engineSubsystem->getEnginePointer() : nullptr;
    EGO_CHECK_RETURN_FALSE(engine && m_projectContext.m_project && m_projectContext.m_simulationGraphicPresenter);

    engine::EngineSession::InitData sessionInitData;
    sessionInitData.m_project = m_projectContext.m_project;
    sessionInitData.m_mainPresentation.m_graphicPresenter = m_projectContext.m_simulationGraphicPresenter;
    sessionInitData.m_sceneRender.m_pluginModuleName = m_simulationRenderPluginModuleName;

    m_projectContext.m_simulationSession = engine->createSession(sessionInitData);

    return static_cast<bool>(m_projectContext.m_simulationSession);
}

void ego::editor::EditorProjectController::releaseProjectContext()
{
    releaseSimulationLevel();
    releaseSimulationSession();
    releaseSimulationGraphicPresenter();

    m_projectContext.m_project = nullptr;
}

void ego::editor::EditorProjectController::releaseSimulationLevel()
{
    if (m_projectContext.m_simulationSession && m_projectContext.m_simulationLevel)
    {
        LevelController& levelController = m_projectContext.m_simulationSession->getLevelController();
        const LevelPointer activeLevel = levelController.getActiveLevel();
        if (activeLevel && activeLevel->getID() == m_projectContext.m_simulationLevel->getID())
        {
            levelController.clearActiveLevel();
        }

        m_projectContext.m_simulationSession->clearRenderCameraEntity();
    }

    m_projectContext.m_simulationLevel = nullptr;
}

void ego::editor::EditorProjectController::releaseSimulationSession()
{
    const engine::EngineSubsystemPointer engineSubsystem = engine::GetEngineSubsystemPointer();
    const engine::EnginePointer engine = engineSubsystem ? engineSubsystem->getEnginePointer() : nullptr;
    if (engine && m_projectContext.m_simulationSession)
    {
        engine->destroySession(m_projectContext.m_simulationSession->getID());
    }

    m_projectContext.m_simulationSession = nullptr;
}

void ego::editor::EditorProjectController::releaseSimulationGraphicPresenter()
{
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    if (editorSubsystem)
    {
        editorSubsystem->getEditorController().getEditorGuiController().setSceneTexture(nullptr);
    }

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_projectContext.m_simulationGraphicPresenter);
}
