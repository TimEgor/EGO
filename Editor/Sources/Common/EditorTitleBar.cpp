#include "EditorTitleBar.h"

#include <algorithm>
#include <cstdint>
#include <limits>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Platform/Surface/PlatformSurface.h"

#include "EgoApplication/ApplicationSubsystem.h"

#include <imgui.h>

namespace
{
    constexpr char EditorTitle[] = "Ego Editor";
    constexpr float TitleBarHorizontalPadding = 8.0f;
    constexpr float TitleBarVerticalPadding = 5.0f;
    constexpr float SystemButtonWidth = 46.0f;
    constexpr float SystemButtonIconHalfSize = 5.0f;
    constexpr float SystemButtonIconLineThickness = 1.0f;
    constexpr float RestoredWindowIconOffset = 2.0f;

    constexpr ImU32 CloseButtonHighlightColor = IM_COL32(196, 43, 28, 255);
} // namespace

struct ego::editor::EditorTitleBar::TitleBarLayout final
{
    ImVec2 m_position;
    ImVec2 m_size;
    ImVec2 m_viewportPosition;
    ImVec2 m_windowMenuMin;
    ImVec2 m_windowMenuMax;
    float m_dpiScale = 1.0f;
    float m_systemButtonWidth = 0.0f;
    float m_systemButtonsMinX = 0.0f;
};

struct ego::editor::EditorTitleBar::SystemButtonLayout final
{
    ImVec2 m_position;
    ImVec2 m_size;
    ImVec2 m_center;
    float m_iconHalfSize = 0.0f;
    float m_lineThickness = 0.0f;
    float m_restoredWindowIconOffset = 0.0f;
};

void ego::editor::EditorTitleBar::draw(PlatformSurface& _surface, bool& _showViewport, bool& _showSceneInspector, bool& _showEntityInspector)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(TitleBarHorizontalPadding, TitleBarVerticalPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(TitleBarHorizontalPadding, 0.0f));

    if (ImGui::BeginMainMenuBar())
    {
        const ImGuiViewport& mainViewport = *ImGui::GetMainViewport();

        TitleBarLayout layout;
        layout.m_position = ImGui::GetWindowPos();
        layout.m_size = ImGui::GetWindowSize();
        layout.m_viewportPosition = mainViewport.Pos;
        layout.m_dpiScale = mainViewport.DpiScale;

        drawIcon(layout.m_size.y);
        drawWindowMenu(_showViewport, _showSceneInspector, _showEntityInspector, layout);
        drawSystemButtons(_surface, layout);
        drawTitle(layout);
        updateCaptionArea(_surface, layout);

        ImGui::EndMainMenuBar();
    }

    ImGui::PopStyleVar(2);
}

void ego::editor::EditorTitleBar::drawWindowMenu(bool& _showViewport, bool& _showSceneInspector, bool& _showEntityInspector, TitleBarLayout& _layout) const
{
    const bool isWindowMenuOpen = ImGui::BeginMenu("Window");
    _layout.m_windowMenuMin = ImGui::GetItemRectMin();
    _layout.m_windowMenuMax = ImGui::GetItemRectMax();
    if (isWindowMenuOpen)
    {
        ImGui::MenuItem("Viewport", nullptr, &_showViewport);
        ImGui::MenuItem("Scene Inspector", nullptr, &_showSceneInspector);
        ImGui::MenuItem("Entity Inspector", nullptr, &_showEntityInspector);
        ImGui::EndMenu();
    }
}

void ego::editor::EditorTitleBar::drawSystemButtons(PlatformSurface& _surface, TitleBarLayout& _layout) const
{
    _layout.m_systemButtonWidth = SystemButtonWidth * _layout.m_dpiScale;
    _layout.m_systemButtonsMinX = _layout.m_position.x + _layout.m_size.x - _layout.m_systemButtonWidth * 3.0f;

    const bool isWindowMaximized = _surface.getWindowState() == PlatformSurfaceWindowState::Maximized;

    if (drawSystemButton(SystemButton::Minimize, _layout.m_systemButtonsMinX, _layout, isWindowMaximized))
    {
        _surface.minimize();
    }

    if (drawSystemButton(SystemButton::Maximize, _layout.m_systemButtonsMinX + _layout.m_systemButtonWidth, _layout, isWindowMaximized))
    {
        if (isWindowMaximized)
        {
            _surface.restore();
        }
        else
        {
            _surface.maximize();
        }
    }

    if (drawSystemButton(SystemButton::Close, _layout.m_systemButtonsMinX + _layout.m_systemButtonWidth * 2.0f, _layout, isWindowMaximized))
    {
        application::GetApplication().requestExit();
    }
}

void ego::editor::EditorTitleBar::drawTitle(const TitleBarLayout& _layout) const
{
    const ImVec2 titleSize = ImGui::CalcTextSize(EditorTitle);
    const float minimumTitleX = _layout.m_windowMenuMax.x + TitleBarHorizontalPadding * _layout.m_dpiScale;
    const float maximumTitleX = _layout.m_systemButtonsMinX - titleSize.x - TitleBarHorizontalPadding * _layout.m_dpiScale;
    const float centeredTitleX = _layout.m_position.x + (_layout.m_size.x - titleSize.x) * 0.5f;
    if (minimumTitleX <= maximumTitleX)
    {
        const float titlePositionX = std::clamp(centeredTitleX, minimumTitleX, maximumTitleX);
        const float titlePositionY = _layout.m_position.y + (_layout.m_size.y - titleSize.y) * 0.5f;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddText(ImVec2(titlePositionX, titlePositionY), ImGui::GetColorU32(ImGuiCol_Text), EditorTitle);
    }
}

void ego::editor::EditorTitleBar::updateCaptionArea(PlatformSurface& _surface, const TitleBarLayout& _layout) const
{
    constexpr float maximumExtent = static_cast<float>((std::numeric_limits<uint16_t>::max)());

    const SurfacePoint position(
        static_cast<int32_t>(_layout.m_windowMenuMax.x - _layout.m_viewportPosition.x),
        static_cast<int32_t>(_layout.m_position.y - _layout.m_viewportPosition.y));
    const SurfaceSize size(
        static_cast<uint16_t>(std::clamp(_layout.m_systemButtonsMinX - _layout.m_windowMenuMax.x, 0.0f, maximumExtent)),
        static_cast<uint16_t>(std::clamp(_layout.m_size.y, 0.0f, maximumExtent)));
    EGO_ASSERT(_surface.setCaptionArea(position, size));
}

void ego::editor::EditorTitleBar::drawIcon(float _titleBarHeight) const
{
    const float logoHeight = _titleBarHeight * 0.55f;
    const float stroke = logoHeight / 5.0f;
    const float glyphWidth = stroke * 5.0f;
    const float glyphSpacing = stroke;
    const float logoWidth = glyphWidth * 3.0f + glyphSpacing * 2.0f;

    ImGui::Dummy(ImVec2(logoWidth, ImGui::GetTextLineHeight()));

    const ImVec2 itemMin = ImGui::GetItemRectMin();
    const float logoMinY = ImGui::GetWindowPos().y + (_titleBarHeight - logoHeight) * 0.5f;
    const ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const auto drawHorizontalLine = [drawList, color, stroke](float _positionX, float _positionY, float _length)
    {
        drawList->AddRectFilled(ImVec2(_positionX, _positionY), ImVec2(_positionX + _length, _positionY + stroke), color);
    };
    const auto drawVerticalLine = [drawList, color, stroke](float _positionX, float _positionY, float _length)
    {
        drawList->AddRectFilled(ImVec2(_positionX, _positionY), ImVec2(_positionX + stroke, _positionY + _length), color);
    };

    const float eMinX = itemMin.x;
    drawHorizontalLine(eMinX, logoMinY, glyphWidth);
    drawHorizontalLine(eMinX, logoMinY + stroke * 2.0f, glyphWidth);
    drawHorizontalLine(eMinX, logoMinY + stroke * 4.0f, glyphWidth);

    const float gMinX = eMinX + glyphWidth + glyphSpacing;
    drawHorizontalLine(gMinX, logoMinY, glyphWidth);
    drawVerticalLine(gMinX, logoMinY, logoHeight);
    drawHorizontalLine(gMinX, logoMinY + stroke * 4.0f, glyphWidth);
    drawHorizontalLine(gMinX + stroke * 2.0f, logoMinY + stroke * 2.0f, stroke * 3.0f);
    drawVerticalLine(gMinX + glyphWidth - stroke, logoMinY + stroke * 2.0f, stroke * 3.0f);

    const float oMinX = gMinX + glyphWidth + glyphSpacing;
    drawHorizontalLine(oMinX, logoMinY, glyphWidth);
    drawHorizontalLine(oMinX, logoMinY + stroke * 4.0f, glyphWidth);
    drawVerticalLine(oMinX, logoMinY, logoHeight);
    drawVerticalLine(oMinX + glyphWidth - stroke, logoMinY, logoHeight);
}

bool ego::editor::EditorTitleBar::drawSystemButton(SystemButton _button, float _positionX, const TitleBarLayout& _layout, bool _isWindowMaximized) const
{
    const SystemButtonLayout buttonLayout = CreateSystemButtonLayout(_positionX, _layout);
    ImGui::SetCursorScreenPos(buttonLayout.m_position);
    ImGui::InvisibleButton(GetSystemButtonID(_button), buttonLayout.m_size);

    const bool isHovered = ImGui::IsItemHovered();
    const bool isActive = ImGui::IsItemActive();
    const bool isClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);

    drawSystemButtonBackground(_button, buttonLayout, isHovered, isActive);
    drawSystemButtonIcon(_button, buttonLayout, _isWindowMaximized);

    return isClicked;
}

const char* ego::editor::EditorTitleBar::GetSystemButtonID(SystemButton _button)
{
    switch (_button)
    {
    case SystemButton::Minimize:
        return "##EditorMinimize";
    case SystemButton::Maximize:
        return "##EditorMaximize";
    case SystemButton::Close:
        return "##EditorClose";
    }

    EGO_ASSERT_FAIL_MESSAGE("Unsupported editor system button.");

    return "";
}

ego::editor::EditorTitleBar::SystemButtonLayout ego::editor::EditorTitleBar::CreateSystemButtonLayout(float _positionX, const TitleBarLayout& _titleBarLayout)
{
    SystemButtonLayout layout;
    layout.m_position = ImVec2(_positionX, _titleBarLayout.m_position.y);
    layout.m_size = ImVec2(_titleBarLayout.m_systemButtonWidth, _titleBarLayout.m_size.y);
    layout.m_center = ImVec2(layout.m_position.x + layout.m_size.x * 0.5f, layout.m_position.y + layout.m_size.y * 0.5f);
    layout.m_iconHalfSize = SystemButtonIconHalfSize * _titleBarLayout.m_dpiScale;
    layout.m_lineThickness = SystemButtonIconLineThickness * _titleBarLayout.m_dpiScale;
    layout.m_restoredWindowIconOffset = RestoredWindowIconOffset * _titleBarLayout.m_dpiScale;

    return layout;
}

void ego::editor::EditorTitleBar::drawSystemButtonBackground(SystemButton _button, const SystemButtonLayout& _layout, bool _isHovered, bool _isActive) const
{
    if (!_isHovered && !_isActive)
    {
        return;
    }

    const ImGuiCol backgroundColor = _isActive ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered;
    const ImU32 color = _button == SystemButton::Close ? CloseButtonHighlightColor : ImGui::GetColorU32(backgroundColor);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(_layout.m_position, ImVec2(_layout.m_position.x + _layout.m_size.x, _layout.m_position.y + _layout.m_size.y), color);
}

void ego::editor::EditorTitleBar::drawSystemButtonIcon(SystemButton _button, const SystemButtonLayout& _layout, bool _isWindowMaximized) const
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImU32 iconColor = ImGui::GetColorU32(ImGuiCol_Text);
    switch (_button)
    {
    case SystemButton::Minimize:
        drawList->AddLine(
            ImVec2(_layout.m_center.x - _layout.m_iconHalfSize, _layout.m_center.y + _layout.m_iconHalfSize * 0.5f),
            ImVec2(_layout.m_center.x + _layout.m_iconHalfSize, _layout.m_center.y + _layout.m_iconHalfSize * 0.5f),
            iconColor,
            _layout.m_lineThickness);
        break;

    case SystemButton::Maximize:
        if (_isWindowMaximized)
        {
            drawList->AddRect(
                ImVec2(
                    _layout.m_center.x - _layout.m_iconHalfSize + _layout.m_restoredWindowIconOffset,
                    _layout.m_center.y - _layout.m_iconHalfSize - _layout.m_restoredWindowIconOffset),
                ImVec2(
                    _layout.m_center.x + _layout.m_iconHalfSize + _layout.m_restoredWindowIconOffset,
                    _layout.m_center.y + _layout.m_iconHalfSize - _layout.m_restoredWindowIconOffset),
                iconColor,
                0.0f,
                _layout.m_lineThickness);
            drawList->AddRect(
                ImVec2(
                    _layout.m_center.x - _layout.m_iconHalfSize - _layout.m_restoredWindowIconOffset,
                    _layout.m_center.y - _layout.m_iconHalfSize + _layout.m_restoredWindowIconOffset),
                ImVec2(
                    _layout.m_center.x + _layout.m_iconHalfSize - _layout.m_restoredWindowIconOffset,
                    _layout.m_center.y + _layout.m_iconHalfSize + _layout.m_restoredWindowIconOffset),
                iconColor,
                0.0f,
                _layout.m_lineThickness);
        }
        else
        {
            drawList->AddRect(
                ImVec2(_layout.m_center.x - _layout.m_iconHalfSize, _layout.m_center.y - _layout.m_iconHalfSize),
                ImVec2(_layout.m_center.x + _layout.m_iconHalfSize, _layout.m_center.y + _layout.m_iconHalfSize),
                iconColor,
                0.0f,
                _layout.m_lineThickness);
        }
        break;

    case SystemButton::Close:
        drawList->AddLine(
            ImVec2(_layout.m_center.x - _layout.m_iconHalfSize, _layout.m_center.y - _layout.m_iconHalfSize),
            ImVec2(_layout.m_center.x + _layout.m_iconHalfSize, _layout.m_center.y + _layout.m_iconHalfSize),
            iconColor,
            _layout.m_lineThickness);
        drawList->AddLine(
            ImVec2(_layout.m_center.x + _layout.m_iconHalfSize, _layout.m_center.y - _layout.m_iconHalfSize),
            ImVec2(_layout.m_center.x - _layout.m_iconHalfSize, _layout.m_center.y + _layout.m_iconHalfSize),
            iconColor,
            _layout.m_lineThickness);
        break;
    }
}
