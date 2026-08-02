#include "EntityInspectorWindow.h"

#include <imgui.h>

namespace
{
    constexpr std::string_view WindowTitle = "Entity Inspector";
} // namespace

std::string_view ego::editor::EntityInspectorWindow::getTitle() const
{
    return WindowTitle;
}

void ego::editor::EntityInspectorWindow::drawWindow(bool& _isVisible)
{
    if (ImGui::Begin(WindowTitle.data(), &_isVisible))
    {
        ImGui::TextDisabled("Selection");
        ImGui::SameLine();
        ImGui::TextUnformatted("Camera");
        ImGui::SeparatorText("Camera Component");
    }

    ImGui::End();
}
