#include "ErrorWindow.h"

#include <utility>

#include <imgui.h>

namespace
{
    constexpr const char* ErrorWindowName = "Error";
} // namespace

ego::editor::ErrorWindow::ErrorWindow(std::string _message)
    : m_message(std::move(_message))
{
}

std::string_view ego::editor::ErrorWindow::getTitle() const
{
    return ErrorWindowName;
}

bool ego::editor::ErrorWindow::draw()
{
    ImGui::TextWrapped("%s", m_message.c_str());

    return !ImGui::Button("OK");
}
