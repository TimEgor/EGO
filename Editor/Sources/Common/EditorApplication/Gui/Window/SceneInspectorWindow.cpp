#include "SceneInspectorWindow.h"

#include <imgui.h>

namespace
{
    constexpr std::string_view WindowTitle = "Scene Inspector";
} // namespace

std::string_view ego::editor::SceneInspectorWindow::getTitle() const
{
    return WindowTitle;
}

void ego::editor::SceneInspectorWindow::drawWindow(bool& _isVisible)
{
    if (ImGui::Begin(WindowTitle.data(), &_isVisible))
    {
        const ImGuiTreeNodeFlags sceneFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (ImGui::TreeNodeEx("Scene", sceneFlags))
        {
            const ImGuiTreeNodeFlags cameraFlags =
                ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAvailWidth;
            ImGui::TreeNodeEx("Camera", cameraFlags);
            ImGui::TreePop();
        }
    }

    ImGui::End();
}
