#include "GuiBackendFactory.h"

#include <memory>

#include "EgoGui/Implementation/ImGui/ImGuiBackend.h"

std::unique_ptr<ego::gui::GuiBackend> ego::gui::CreateGuiBackend()
{
    return std::make_unique<ImGuiBackend>();
}
