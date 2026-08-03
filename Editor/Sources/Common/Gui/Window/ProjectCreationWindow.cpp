#include "ProjectCreationWindow.h"

#include <cstddef>
#include <string_view>

#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EditorController.h"
#include "EditorSubsystem.h"

#include <imgui.h>

namespace
{
    constexpr const char* ProjectCreationWindowName = "Create Project";
    constexpr float ProjectNameInputWidth = 360.0f;
    constexpr ImVec4 ErrorMessageColor = {1.0f, 0.35f, 0.35f, 1.0f};
    constexpr std::string_view TrimTextCharacters = " \t\r\n";
} // namespace

std::string_view ego::editor::ProjectCreationWindow::getTitle() const
{
    return ProjectCreationWindowName;
}

bool ego::editor::ProjectCreationWindow::draw()
{
    ImGui::TextUnformatted("Enter a name for the new project.");

    const bool isSubmitted = drawProjectNameInput();
    drawProjectPathInput();

    const FileName projectDirectory = getProjectDirectory();
    if (projectDirectory)
    {
        ImGui::TextDisabled("Project directory: %s", projectDirectory.c_str());
    }

    drawErrorMessage();

    return drawActions(projectDirectory, isSubmitted);
}

bool ego::editor::ProjectCreationWindow::drawProjectNameInput()
{
    if (ImGui::IsWindowAppearing())
    {
        ImGui::SetKeyboardFocusHere();
    }

    ImGui::SetNextItemWidth(ProjectNameInputWidth);
    const bool isSubmitted =
        ImGui::InputText("Project Name", m_projectName.data(), m_projectName.size(), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::IsItemEdited())
    {
        m_errorMessage.clear();
    }

    return isSubmitted;
}

void ego::editor::ProjectCreationWindow::drawProjectPathInput()
{
    ImGui::TextDisabled("Location: %s", m_projectPath ? m_projectPath.c_str() : "Not selected");
    ImGui::SameLine();
    EGO_CHECK_RETURN(ImGui::Button("Browse..."));

    const FileName selectedPath = selectProjectPath();
    EGO_CHECK_RETURN(selectedPath);

    m_projectPath = selectedPath;
    m_errorMessage.clear();
}

void ego::editor::ProjectCreationWindow::drawErrorMessage() const
{
    if (m_errorMessage.empty())
    {
        return;
    }

    ImGui::TextColored(ErrorMessageColor, "%s", m_errorMessage.c_str());
}

bool ego::editor::ProjectCreationWindow::drawActions(const FileName& _directory, bool _isSubmitted)
{
    ImGui::BeginDisabled(!_directory);
    const bool isCreateRequested = ImGui::Button("Create") || _isSubmitted;
    ImGui::EndDisabled();

    if (isCreateRequested && _directory && tryCreateProject(_directory))
    {
        return false;
    }

    ImGui::SameLine();

    return !ImGui::Button("Cancel");
}

ego::FileName ego::editor::ProjectCreationWindow::selectProjectPath() const
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN_VALUE(platform, FileName());

    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    EGO_CHECK_RETURN_VALUE(editorSubsystem, FileName());

    Platform::SelectDirectoryDialogParams params;
    params.m_title = "Select project location";

    const PlatformSurfacePointer mainSurface = editorSubsystem->getEditorController().getMainSurfacePointer();
    params.m_ownerWindowHandle = mainSurface ? mainSurface->getNativeHandle() : nullptr;

    return platform->selectDirectory(params);
}

std::string ego::editor::ProjectCreationWindow::TrimText(const char* _text)
{
    const std::string text(_text);
    const std::size_t firstCharacter = text.find_first_not_of(TrimTextCharacters);
    if (firstCharacter == std::string::npos)
    {
        return std::string();
    }

    const std::size_t lastCharacter = text.find_last_not_of(TrimTextCharacters);

    return text.substr(firstCharacter, lastCharacter - firstCharacter + 1);
}

std::string ego::editor::ProjectCreationWindow::getProjectName() const
{
    return TrimText(m_projectName.data());
}

ego::FileName ego::editor::ProjectCreationWindow::getProjectDirectory() const
{
    const FileName projectName(getProjectName());
    EGO_CHECK_RETURN_VALUE(m_projectPath && projectName, FileName());

    return file_name_utils::CombinePath(m_projectPath, projectName);
}

bool ego::editor::ProjectCreationWindow::tryCreateProject(const FileName& _directory)
{
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    if (!editorSubsystem)
    {
        m_errorMessage = "Editor subsystem is unavailable.";

        return false;
    }

    const std::string projectName = getProjectName();
    if (!editorSubsystem->getEditorController().getProjectController().createProject(projectName, _directory))
    {
        m_errorMessage = "Failed to create the project.";

        return false;
    }

    return true;
}
