#include "ImGuiContextScope.h"

#include <imgui.h>

ego::gui::ImGuiContextScope::ImGuiContextScope(ImGuiContext& _context)
    : m_previousContext(ImGui::GetCurrentContext())
{
    ImGui::SetCurrentContext(&_context);
}

ego::gui::ImGuiContextScope::~ImGuiContextScope()
{
    ImGui::SetCurrentContext(m_previousContext);
}
