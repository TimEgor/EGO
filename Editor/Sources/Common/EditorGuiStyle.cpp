#include "EditorGuiStyle.h"

#include <cstdint>

#include <imgui.h>

namespace
{
    constexpr float ColorChannelScale = 1.0f / 255.0f;

    constexpr ImVec4 MakeColor(uint8_t _red, uint8_t _green, uint8_t _blue, uint8_t _alpha = 255)
    {
        return ImVec4(_red * ColorChannelScale, _green * ColorChannelScale, _blue * ColorChannelScale, _alpha * ColorChannelScale);
    }
} // namespace

void ego::editor::EditorGuiStyle::apply()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark(&style);

    style.WindowPadding = ImVec2(8.0f, 6.0f);
    style.FramePadding = ImVec2(6.0f, 3.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(5.0f, 4.0f);
    style.CellPadding = ImVec2(6.0f, 3.0f);
    style.IndentSpacing = 16.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 11.0f;
    style.ScrollbarPadding = 1.0f;
    style.GrabMinSize = 8.0f;

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.PopupRounding = 2.0f;
    style.FrameRounding = 1.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 1.0f;
    style.ImageRounding = 0.0f;
    style.TabRounding = 2.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.ImageBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    style.TabBarBorderSize = 1.0f;
    style.TabBarOverlineSize = 0.0f;
    style.SeparatorSize = 1.0f;
    style.DockingSeparatorSize = 2.0f;

    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.5f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = MakeColor(210, 210, 210);
    colors[ImGuiCol_TextDisabled] = MakeColor(112, 114, 118);
    colors[ImGuiCol_WindowBg] = MakeColor(39, 40, 43);
    colors[ImGuiCol_ChildBg] = MakeColor(39, 40, 43);
    colors[ImGuiCol_PopupBg] = MakeColor(45, 46, 50, 250);
    colors[ImGuiCol_Border] = MakeColor(17, 18, 20);
    colors[ImGuiCol_BorderShadow] = MakeColor(0, 0, 0, 0);

    colors[ImGuiCol_FrameBg] = MakeColor(50, 51, 55);
    colors[ImGuiCol_FrameBgHovered] = MakeColor(61, 63, 68);
    colors[ImGuiCol_FrameBgActive] = MakeColor(70, 73, 79);
    colors[ImGuiCol_TitleBg] = MakeColor(27, 28, 30);
    colors[ImGuiCol_TitleBgActive] = MakeColor(31, 32, 35);
    colors[ImGuiCol_TitleBgCollapsed] = MakeColor(24, 25, 27);
    colors[ImGuiCol_MenuBarBg] = MakeColor(13, 14, 15);

    colors[ImGuiCol_ScrollbarBg] = MakeColor(31, 32, 35);
    colors[ImGuiCol_ScrollbarGrab] = MakeColor(65, 67, 72);
    colors[ImGuiCol_ScrollbarGrabHovered] = MakeColor(83, 86, 92);
    colors[ImGuiCol_ScrollbarGrabActive] = MakeColor(99, 103, 110);

    colors[ImGuiCol_CheckMark] = MakeColor(75, 159, 224);
    colors[ImGuiCol_CheckboxSelectedBg] = MakeColor(49, 91, 130);
    colors[ImGuiCol_SliderGrab] = MakeColor(142, 145, 151);
    colors[ImGuiCol_SliderGrabActive] = MakeColor(75, 159, 224);
    colors[ImGuiCol_Button] = MakeColor(58, 59, 63);
    colors[ImGuiCol_ButtonHovered] = MakeColor(72, 74, 79);
    colors[ImGuiCol_ButtonActive] = MakeColor(82, 85, 91);
    colors[ImGuiCol_Header] = MakeColor(51, 52, 56);
    colors[ImGuiCol_HeaderHovered] = MakeColor(64, 66, 71);
    colors[ImGuiCol_HeaderActive] = MakeColor(55, 96, 136);

    colors[ImGuiCol_Separator] = MakeColor(18, 19, 21);
    colors[ImGuiCol_SeparatorHovered] = MakeColor(64, 114, 158);
    colors[ImGuiCol_SeparatorActive] = MakeColor(70, 134, 189);
    colors[ImGuiCol_ResizeGrip] = MakeColor(0, 0, 0, 0);
    colors[ImGuiCol_ResizeGripHovered] = MakeColor(70, 134, 189, 150);
    colors[ImGuiCol_ResizeGripActive] = MakeColor(70, 134, 189, 230);
    colors[ImGuiCol_InputTextCursor] = MakeColor(220, 220, 220);

    colors[ImGuiCol_Tab] = MakeColor(29, 30, 33);
    colors[ImGuiCol_TabHovered] = MakeColor(48, 50, 54);
    colors[ImGuiCol_TabSelected] = MakeColor(41, 42, 46);
    colors[ImGuiCol_TabSelectedOverline] = MakeColor(41, 42, 46);
    colors[ImGuiCol_TabDimmed] = MakeColor(25, 26, 28);
    colors[ImGuiCol_TabDimmedSelected] = MakeColor(36, 37, 40);
    colors[ImGuiCol_TabDimmedSelectedOverline] = MakeColor(36, 37, 40);
    colors[ImGuiCol_DockingPreview] = MakeColor(70, 134, 189, 110);
    colors[ImGuiCol_DockingEmptyBg] = MakeColor(22, 23, 25);

    colors[ImGuiCol_PlotLines] = MakeColor(142, 145, 151);
    colors[ImGuiCol_PlotLinesHovered] = MakeColor(75, 159, 224);
    colors[ImGuiCol_PlotHistogram] = MakeColor(210, 150, 70);
    colors[ImGuiCol_PlotHistogramHovered] = MakeColor(230, 170, 90);
    colors[ImGuiCol_TableHeaderBg] = MakeColor(45, 46, 50);
    colors[ImGuiCol_TableBorderStrong] = MakeColor(18, 19, 21);
    colors[ImGuiCol_TableBorderLight] = MakeColor(52, 54, 58);
    colors[ImGuiCol_TableRowBg] = MakeColor(0, 0, 0, 0);
    colors[ImGuiCol_TableRowBgAlt] = MakeColor(255, 255, 255, 8);

    colors[ImGuiCol_TextLink] = MakeColor(89, 168, 230);
    colors[ImGuiCol_TextSelectedBg] = MakeColor(55, 96, 136, 180);
    colors[ImGuiCol_TreeLines] = MakeColor(68, 70, 74);
    colors[ImGuiCol_DragDropTarget] = MakeColor(75, 159, 224);
    colors[ImGuiCol_DragDropTargetBg] = MakeColor(75, 159, 224, 35);
    colors[ImGuiCol_UnsavedMarker] = MakeColor(230, 170, 90);
    colors[ImGuiCol_NavCursor] = MakeColor(75, 159, 224);
    colors[ImGuiCol_NavWindowingHighlight] = MakeColor(255, 255, 255, 180);
    colors[ImGuiCol_NavWindowingDimBg] = MakeColor(20, 21, 23, 180);
    colors[ImGuiCol_ModalWindowDimBg] = MakeColor(0, 0, 0, 150);
}
