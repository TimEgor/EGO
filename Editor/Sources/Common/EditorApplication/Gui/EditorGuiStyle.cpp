#include "EditorGuiStyle.h"

#include <cstdint>

namespace
{
    constexpr float ColorChannelScale = 1.0f / 255.0f;

    constexpr ego::NormalizedColorRGBA MakeColor(uint8_t _red, uint8_t _green, uint8_t _blue, uint8_t _alpha = 255)
    {
        return ego::NormalizedColorRGBA(_red * ColorChannelScale, _green * ColorChannelScale, _blue * ColorChannelScale, _alpha * ColorChannelScale);
    }
} // namespace

ego::gui::GuiStyle ego::editor::CreateEditorGuiStyle()
{
    gui::GuiStyle style;

    style.m_windowPadding = FloatVector2(8.0f, 6.0f);
    style.m_framePadding = FloatVector2(6.0f, 3.0f);
    style.m_itemSpacing = FloatVector2(6.0f, 4.0f);
    style.m_itemInnerSpacing = FloatVector2(5.0f, 4.0f);
    style.m_cellPadding = FloatVector2(6.0f, 3.0f);
    style.m_indentSpacing = 16.0f;
    style.m_columnsMinSpacing = 6.0f;
    style.m_scrollbarSize = 11.0f;
    style.m_scrollbarPadding = 1.0f;
    style.m_grabMinSize = 8.0f;

    style.m_windowRounding = 0.0f;
    style.m_childRounding = 0.0f;
    style.m_popupRounding = 2.0f;
    style.m_frameRounding = 1.0f;
    style.m_scrollbarRounding = 2.0f;
    style.m_grabRounding = 1.0f;
    style.m_imageRounding = 0.0f;
    style.m_tabRounding = 2.0f;

    style.m_windowBorderSize = 1.0f;
    style.m_childBorderSize = 1.0f;
    style.m_popupBorderSize = 1.0f;
    style.m_frameBorderSize = 0.0f;
    style.m_imageBorderSize = 0.0f;
    style.m_tabBorderSize = 0.0f;
    style.m_tabBarBorderSize = 1.0f;
    style.m_tabBarOverlineSize = 0.0f;
    style.m_separatorSize = 1.0f;
    style.m_dockingSeparatorSize = 2.0f;

    style.m_windowTitleAlign = FloatVector2(0.0f, 0.5f);
    style.m_windowMenuButtonPosition = gui::GuiDirection::Left;
    style.m_buttonTextAlign = FloatVector2(0.5f, 0.5f);
    style.m_selectableTextAlign = FloatVector2(0.0f, 0.5f);

    style.m_textColor = MakeColor(210, 210, 210);
    style.m_textDisabledColor = MakeColor(112, 114, 118);
    style.m_windowBackgroundColor = MakeColor(39, 40, 43);
    style.m_childBackgroundColor = MakeColor(39, 40, 43);
    style.m_popupBackgroundColor = MakeColor(45, 46, 50, 250);
    style.m_borderColor = MakeColor(17, 18, 20);
    style.m_borderShadowColor = MakeColor(0, 0, 0, 0);

    style.m_frameBackgroundColor = MakeColor(50, 51, 55);
    style.m_frameBackgroundHoveredColor = MakeColor(61, 63, 68);
    style.m_frameBackgroundActiveColor = MakeColor(70, 73, 79);
    style.m_titleBackgroundColor = MakeColor(27, 28, 30);
    style.m_titleBackgroundActiveColor = MakeColor(31, 32, 35);
    style.m_titleBackgroundCollapsedColor = MakeColor(24, 25, 27);
    style.m_menuBarBackgroundColor = MakeColor(13, 14, 15);

    style.m_scrollbarBackgroundColor = MakeColor(31, 32, 35);
    style.m_scrollbarGrabColor = MakeColor(65, 67, 72);
    style.m_scrollbarGrabHoveredColor = MakeColor(83, 86, 92);
    style.m_scrollbarGrabActiveColor = MakeColor(99, 103, 110);

    style.m_checkMarkColor = MakeColor(75, 159, 224);
    style.m_checkboxSelectedBackgroundColor = MakeColor(49, 91, 130);
    style.m_sliderGrabColor = MakeColor(142, 145, 151);
    style.m_sliderGrabActiveColor = MakeColor(75, 159, 224);
    style.m_buttonColor = MakeColor(58, 59, 63);
    style.m_buttonHoveredColor = MakeColor(72, 74, 79);
    style.m_buttonActiveColor = MakeColor(82, 85, 91);
    style.m_headerColor = MakeColor(51, 52, 56);
    style.m_headerHoveredColor = MakeColor(64, 66, 71);
    style.m_headerActiveColor = MakeColor(55, 96, 136);

    style.m_separatorColor = MakeColor(18, 19, 21);
    style.m_separatorHoveredColor = MakeColor(64, 114, 158);
    style.m_separatorActiveColor = MakeColor(70, 134, 189);
    style.m_resizeGripColor = MakeColor(0, 0, 0, 0);
    style.m_resizeGripHoveredColor = MakeColor(70, 134, 189, 150);
    style.m_resizeGripActiveColor = MakeColor(70, 134, 189, 230);
    style.m_inputTextCursorColor = MakeColor(220, 220, 220);

    style.m_tabColor = MakeColor(29, 30, 33);
    style.m_tabHoveredColor = MakeColor(48, 50, 54);
    style.m_tabSelectedColor = MakeColor(41, 42, 46);
    style.m_tabSelectedOverlineColor = MakeColor(41, 42, 46);
    style.m_tabDimmedColor = MakeColor(25, 26, 28);
    style.m_tabDimmedSelectedColor = MakeColor(36, 37, 40);
    style.m_tabDimmedSelectedOverlineColor = MakeColor(36, 37, 40);
    style.m_dockingPreviewColor = MakeColor(70, 134, 189, 110);
    style.m_dockingEmptyBackgroundColor = MakeColor(22, 23, 25);

    style.m_plotLinesColor = MakeColor(142, 145, 151);
    style.m_plotLinesHoveredColor = MakeColor(75, 159, 224);
    style.m_plotHistogramColor = MakeColor(210, 150, 70);
    style.m_plotHistogramHoveredColor = MakeColor(230, 170, 90);
    style.m_tableHeaderBackgroundColor = MakeColor(45, 46, 50);
    style.m_tableBorderStrongColor = MakeColor(18, 19, 21);
    style.m_tableBorderLightColor = MakeColor(52, 54, 58);
    style.m_tableRowBackgroundColor = MakeColor(0, 0, 0, 0);
    style.m_tableRowBackgroundAlternateColor = MakeColor(255, 255, 255, 8);

    style.m_textLinkColor = MakeColor(89, 168, 230);
    style.m_textSelectedBackgroundColor = MakeColor(55, 96, 136, 180);
    style.m_treeLinesColor = MakeColor(68, 70, 74);
    style.m_dragDropTargetColor = MakeColor(75, 159, 224);
    style.m_dragDropTargetBackgroundColor = MakeColor(75, 159, 224, 35);
    style.m_unsavedMarkerColor = MakeColor(230, 170, 90);
    style.m_navCursorColor = MakeColor(75, 159, 224);
    style.m_navWindowingHighlightColor = MakeColor(255, 255, 255, 180);
    style.m_navWindowingDimBackgroundColor = MakeColor(20, 21, 23, 180);
    style.m_modalWindowDimBackgroundColor = MakeColor(0, 0, 0, 150);

    return style;
}
