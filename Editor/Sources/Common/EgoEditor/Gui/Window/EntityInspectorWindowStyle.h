#pragma once

#include <string_view>

#include <imgui.h>

namespace ego::editor::entity_inspector_style
{
    inline constexpr std::string_view WindowTitle = "Entity Inspector";

    inline constexpr ImVec4 EntityHeaderColor = ImVec4(0.125f, 0.13f, 0.14f, 1.0f);
    inline constexpr float EntityHeaderRounding = 2.0f;
    inline constexpr ImVec2 EntityHeaderPadding = ImVec2(8.0f, 8.0f);
    inline constexpr float MetadataLabelWidth = 72.0f;

    inline constexpr ImVec4 ComponentHeaderColor = ImVec4(0.19f, 0.195f, 0.21f, 1.0f);
    inline constexpr ImVec4 ComponentHeaderHoveredColor = ImVec4(0.24f, 0.25f, 0.27f, 1.0f);
    inline constexpr ImVec4 ComponentHeaderActiveColor = ImVec4(0.27f, 0.28f, 0.3f, 1.0f);
    inline constexpr ImVec2 ComponentHeaderPadding = ImVec2(7.0f, 5.0f);
    inline constexpr ImVec2 PropertyCellPadding = ImVec2(7.0f, 3.0f);
    inline constexpr float EmptyPropertyIndent = 7.0f;
    inline constexpr float PropertyNameColumnWeight = 0.42f;
    inline constexpr float PropertyValueColumnWeight = 0.58f;
} // namespace ego::editor::entity_inspector_style
