#include "EgoEditor/ProjectController.h"

#include <string>

#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/Parsers/XmlParser/XmlNode.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoEngine/EngineSubsystem.h"
#include "EgoEngine/Project/ProjectReader.h"
#include "EgoEngine/Project/ProjectWriter.h"

#include "EgoEditor/EditorController.h"
#include "EgoEditor/EditorSubsystem.h"
#include "EgoEditor/Gui/Window/ErrorWindow.h"
#include "EgoEditor/Gui/Window/ProjectCreationWindow.h"

namespace
{
    constexpr const char* ProjectAlreadyExistsErrorMessage = "A project with this name already exists in the selected location.";
    constexpr const char* ProjectBackupFileSuffix = ".backup";
    constexpr const char* ProjectFileExtension = ".egoproj";
    constexpr const char* ProjectSavingErrorMessage = "Failed to save the project.";
    constexpr ego::gpu::Texture2DSize SimulationTextureSize(900, 600);
} // namespace

ego::editor::ProjectController::~ProjectController()
{
    release();
}

bool ego::editor::ProjectController::init(const XmlDocument& _config)
{
    release();

    return readConfig(_config);
}

void ego::editor::ProjectController::release()
{
    releaseProjectContext();

    m_simulationRenderPluginModuleName.clear();
}

bool ego::editor::ProjectController::isProjectLoaded() const
{
    return m_projectContext.m_simulationSession && m_projectContext.m_project;
}

const ego::FileName& ego::editor::ProjectController::getProjectDirectory() const
{
    return m_projectContext.m_directory;
}

ego::engine::EngineSessionPointer ego::editor::ProjectController::getSimulationSessionPointer() const
{
    return m_projectContext.m_simulationSession;
}

ego::LevelPointer ego::editor::ProjectController::getCurrentLevelPointer() const
{
    return m_projectContext.m_simulationSession ? m_projectContext.m_simulationSession->getActiveLevel() : nullptr;
}

bool ego::editor::ProjectController::selectEntity(ecs::Entity _entity)
{
    const LevelPointer currentLevel = getCurrentLevelPointer();
    if (!currentLevel || !currentLevel->isNode(_entity))
    {
        return false;
    }

    m_projectContext.m_selectedLevel = currentLevel;
    m_projectContext.m_selectedEntity = _entity;

    return true;
}

ego::ecs::Entity ego::editor::ProjectController::getSelectedEntity() const
{
    const LevelPointer selectedLevel = m_projectContext.m_selectedLevel.lock();
    const LevelPointer currentLevel = getCurrentLevelPointer();
    if (!selectedLevel || selectedLevel != currentLevel || !selectedLevel->isNode(m_projectContext.m_selectedEntity))
    {
        return ecs::Entity();
    }

    return m_projectContext.m_selectedEntity;
}

void ego::editor::ProjectController::clearSelectedEntity()
{
    m_projectContext.m_selectedLevel.reset();
    m_projectContext.m_selectedEntity = ecs::Entity();
}

void ego::editor::ProjectController::createProject()
{
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    EGO_CHECK_RETURN(editorSubsystem);

    const ProjectCreationWindowPointer projectCreationWindow = MakePointer<ProjectCreationWindow>();
    EGO_CHECK_RETURN(projectCreationWindow);

    GuiController& editorGuiController = editorSubsystem->getEditorController().getGuiController();
    EGO_CHECK_RETURN(editorGuiController.pushModalWindow(projectCreationWindow));
}

bool ego::editor::ProjectController::createProject(const std::string& _name, const FileName& _directory)
{
    EGO_CHECK_RETURN_FALSE(!_name.empty() && _directory);

    const PlatformPointer platform = GetPlatformPointer();
    const FileSystemPointer fileSystem = platform ? platform->getFileSystem() : nullptr;
    EGO_CHECK_RETURN_FALSE(fileSystem);

    const FileName projectDirectory = fileSystem->getAbsolutePath(_directory);
    EGO_CHECK_RETURN_FALSE(projectDirectory);

    const FileName projectFileName = file_name_utils::CombinePath(projectDirectory, FileName(_name + ProjectFileExtension));
    EGO_CHECK_RETURN_FALSE(projectFileName);
    if (fileSystem->exists(projectFileName))
    {
        showError(ProjectAlreadyExistsErrorMessage);

        return false;
    }

    const engine::ProjectPointer project = MakePointer<engine::Project>();
    EGO_CHECK_RETURN_FALSE(project && project->setName(_name));

    if (isProjectLoaded())
    {
        unloadProject();
    }

    EGO_CHECK_RETURN_FALSE(initProjectContext(project, projectDirectory));

    return saveProject();
}

void ego::editor::ProjectController::loadProject()
{
    const FileName projectFileName = selectProjectFile();
    EGO_CHECK_RETURN(projectFileName);

    if (isProjectLoaded())
    {
        unloadProject();
    }

    const engine::ProjectPointer project = MakePointer<engine::Project>();
    EGO_CHECK_RETURN(project && engine::ProjectReader::ReadFromFile(projectFileName, *project));

    const FileName projectDirectory = file_name_utils::GetFileDirPath(projectFileName);
    EGO_CHECK_RETURN(projectDirectory);

    initProjectContext(project, projectDirectory);
}

bool ego::editor::ProjectController::saveProject() const
{
    if (saveProjectContext())
    {
        return true;
    }

    showError(ProjectSavingErrorMessage);

    return false;
}

void ego::editor::ProjectController::unloadProject()
{
    releaseProjectContext();
}

bool ego::editor::ProjectController::readConfig(const XmlDocument& _config)
{
    EGO_CHECK_RETURN_FALSE(!m_simulationRenderPluginModuleName);

    const XmlNode rootNode = _config.getRootNode();
    EGO_CHECK_RETURN_FALSE(rootNode && rootNode.getNameView() == "Editor");

    const XmlNode engineNode = rootNode.getChild("Engine");
    EGO_CHECK_RETURN_FALSE(engineNode);

    m_simulationRenderPluginModuleName = FileName(engineNode.getChildValueOr<std::string>("SimulationRenderPlugin", std::string()));

    return static_cast<bool>(m_simulationRenderPluginModuleName);
}

ego::FileName ego::editor::ProjectController::selectProjectFile() const
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN_VALUE(platform, FileName());

    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    EGO_CHECK_RETURN_VALUE(editorSubsystem, FileName());

    const Platform::OpenFileDialogFilter filters[] = {{"EGO Project (*.egoproj)", "*.egoproj"}, {"All Files (*.*)", "*.*"}};

    Platform::SelectFileDialogParams params;
    params.m_title = "Select EGO project";
    params.m_defaultExtension = "egoproj";
    params.m_filters = filters;
    params.m_filterCount = sizeof(filters) / sizeof(filters[0]);

    const PlatformSurfacePointer mainSurface = editorSubsystem->getEditorController().getMainSurfacePointer();
    params.m_ownerWindowHandle = mainSurface ? mainSurface->getNativeHandle() : nullptr;

    return platform->selectOpenFile(params);
}

void ego::editor::ProjectController::showError(const std::string& _message) const
{
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    EGO_CHECK_RETURN(editorSubsystem);

    const ErrorWindowPointer errorWindow = MakePointer<ErrorWindow>(_message);
    EGO_CHECK_RETURN(errorWindow);

    GuiController& editorGuiController = editorSubsystem->getEditorController().getGuiController();
    EGO_CHECK_RETURN(editorGuiController.pushModalWindow(errorWindow));
}

bool ego::editor::ProjectController::initProjectContext(const engine::ProjectPointer& _project, const FileName& _directory)
{
    EGO_CHECK_RETURN_FALSE(m_simulationRenderPluginModuleName && _project && _directory);

    m_projectContext.m_project = _project;
    m_projectContext.m_directory = _directory;

    EGO_CHECK_RETURN_CALL_FALSE(initSimulationGraphicPresenter(), releaseProjectContext());
    EGO_CHECK_RETURN_CALL_FALSE(initSimulationSession(), releaseProjectContext());

    return true;
}

bool ego::editor::ProjectController::saveProjectContext() const
{
    EGO_CHECK_RETURN_FALSE(m_projectContext.m_project && m_projectContext.m_directory);

    const PlatformPointer platform = GetPlatformPointer();
    const FileSystemPointer fileSystem = platform ? platform->getFileSystem() : nullptr;
    EGO_CHECK_RETURN_FALSE(fileSystem);
    EGO_CHECK_RETURN_FALSE(fileSystem->isDirectory(m_projectContext.m_directory) || fileSystem->createDirectory(m_projectContext.m_directory));

    const std::string& projectName = m_projectContext.m_project->getName();
    EGO_CHECK_RETURN_FALSE(!projectName.empty());

    const FileName projectFileName = file_name_utils::CombinePath(m_projectContext.m_directory, FileName(projectName + ProjectFileExtension));
    EGO_CHECK_RETURN_FALSE(projectFileName);

    const bool hasProjectFile = fileSystem->isFile(projectFileName);
    EGO_CHECK_RETURN_FALSE(!fileSystem->exists(projectFileName) || hasProjectFile);

    const FileName backupFileName = projectFileName + ProjectBackupFileSuffix;
    if (hasProjectFile)
    {
        EGO_CHECK_RETURN_FALSE(!fileSystem->exists(backupFileName));
        EGO_CHECK_RETURN_FALSE(fileSystem->move(projectFileName, backupFileName, false));
    }

    if (!engine::ProjectWriter::WriteToFile(projectFileName, *m_projectContext.m_project))
    {
        if (fileSystem->isFile(projectFileName))
        {
            fileSystem->removeFile(projectFileName);
        }

        if (hasProjectFile)
        {
            fileSystem->move(backupFileName, projectFileName, false);
        }

        return false;
    }

    return !hasProjectFile || fileSystem->removeFile(backupFileName);
}

bool ego::editor::ProjectController::initSimulationGraphicPresenter()
{
    TextureGraphicPresenterPointer graphicPresenter = MakePointer<TextureGraphicPresenter>();
    if (!graphicPresenter || !graphicPresenter->init(gpu::GetGraphicDevice(), SimulationTextureSize, gpu::GraphicResourceFormat::R8G8B8A8UNorm))
    {
        graphicPresenter = nullptr;

        return false;
    }

    const gpu::TextureViewPointer sceneTexture = graphicPresenter->getTextureView();
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    if (!sceneTexture || !editorSubsystem)
    {
        graphicPresenter = nullptr;

        return false;
    }

    m_projectContext.m_simulationGraphicPresenter = graphicPresenter;
    editorSubsystem->getEditorController().getGuiController().setSceneTexture(sceneTexture);

    return true;
}

bool ego::editor::ProjectController::initSimulationSession()
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

void ego::editor::ProjectController::releaseProjectContext()
{
    releaseSimulationLevel();
    releaseSimulationSession();
    releaseSimulationGraphicPresenter();

    m_projectContext.m_project = nullptr;
    m_projectContext.m_directory.clear();
}

void ego::editor::ProjectController::releaseSimulationLevel()
{
    clearSelectedEntity();

    if (m_projectContext.m_simulationSession)
    {
        m_projectContext.m_simulationSession->clearActiveLevel();
        m_projectContext.m_simulationSession->clearRenderCameraEntity();
    }
}

void ego::editor::ProjectController::releaseSimulationSession()
{
    const engine::EngineSubsystemPointer engineSubsystem = engine::GetEngineSubsystemPointer();
    const engine::EnginePointer engine = engineSubsystem ? engineSubsystem->getEnginePointer() : nullptr;
    if (engine && m_projectContext.m_simulationSession)
    {
        engine->destroySession(m_projectContext.m_simulationSession->getID());
    }

    m_projectContext.m_simulationSession = nullptr;
}

void ego::editor::ProjectController::releaseSimulationGraphicPresenter()
{
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    if (editorSubsystem)
    {
        editorSubsystem->getEditorController().getGuiController().setSceneTexture(nullptr);
    }

    m_projectContext.m_simulationGraphicPresenter = nullptr;
}
