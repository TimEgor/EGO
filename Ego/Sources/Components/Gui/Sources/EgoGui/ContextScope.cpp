#include "ContextScope.h"

#include <imgui.h>

ego::gui::ContextScope::ContextScope(ImGuiContext& _context)
    : m_previousContext(ImGui::GetCurrentContext())
{
    ImGui::SetCurrentContext(&_context);
}

ego::gui::ContextScope::~ContextScope()
{
    ImGui::SetCurrentContext(m_previousContext);
}
