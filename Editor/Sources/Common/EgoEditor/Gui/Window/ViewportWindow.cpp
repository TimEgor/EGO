#include "EgoEditor/Gui/Window/ViewportWindow.h"

#include "EgoGui/Rendering/GuiTexture.h"

#include "EgoEditor/EditorController.h"
#include "EgoEditor/EditorSubsystem.h"

#include <imgui.h>

namespace
{
    constexpr std::string_view WindowTitle = "Viewport";
} // namespace

std::string_view ego::editor::ViewportWindow::getTitle() const
{
    return WindowTitle;
}

void ego::editor::ViewportWindow::drawWindow(bool& _isVisible)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin(WindowTitle.data(), &_isVisible))
    {
        const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
        const gui::GuiFrameTextureID sceneTextureID =
            editorSubsystem ? editorSubsystem->getEditorController().getGuiController().getSceneTextureID() : gui::InvalidGuiFrameTextureID;
        const ImVec2 availableSize = ImGui::GetContentRegionAvail();
        if (sceneTextureID != gui::InvalidGuiFrameTextureID && availableSize.x > 0.0f && availableSize.y > 0.0f)
        {
            ImGui::Image(ImTextureRef(static_cast<ImTextureID>(sceneTextureID)), availableSize);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}
