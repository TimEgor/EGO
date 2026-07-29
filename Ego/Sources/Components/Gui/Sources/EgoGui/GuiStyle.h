#pragma once

#include <cstdint>

#include "EgoCore/Math/Color.h"

namespace ego::gui
{
    enum class GuiDirection
    {
        None,
        Left,
        Right,
        Up,
        Down
    };

    enum class GuiTreeLines
    {
        None,
        Full,
        ToNodes
    };

    enum class GuiHoverFlags : uint32_t
    {
        None = 0,
        AllowWhenDisabled = 1 << 0,
        Stationary = 1 << 1,
        DelayShort = 1 << 2,
        DelayNormal = 1 << 3,
        NoSharedDelay = 1 << 4
    };

    constexpr GuiHoverFlags operator|(GuiHoverFlags _left, GuiHoverFlags _right)
    {
        return static_cast<GuiHoverFlags>(static_cast<uint32_t>(_left) | static_cast<uint32_t>(_right));
    }

    struct GuiStyle final
    {
        float m_fontSizeBase = 0.0f;
        float m_fontScaleMain = 1.0f;
        float m_fontScaleDpi = 1.0f;

        float m_alpha = 1.0f;
        float m_disabledAlpha = 0.60f;
        FloatVector2 m_windowPadding = FloatVector2(8.0f, 8.0f);
        float m_windowRounding = 0.0f;
        float m_windowBorderSize = 1.0f;
        float m_windowBorderHoverPadding = 4.0f;
        FloatVector2 m_windowMinSize = FloatVector2(32.0f, 32.0f);
        FloatVector2 m_windowTitleAlign = FloatVector2(0.0f, 0.5f);
        GuiDirection m_windowMenuButtonPosition = GuiDirection::Left;

        float m_childRounding = 0.0f;
        float m_childBorderSize = 1.0f;
        float m_popupRounding = 0.0f;
        float m_popupBorderSize = 1.0f;
        FloatVector2 m_framePadding = FloatVector2(4.0f, 3.0f);
        float m_frameRounding = 0.0f;
        float m_frameBorderSize = 0.0f;

        FloatVector2 m_itemSpacing = FloatVector2(8.0f, 4.0f);
        FloatVector2 m_itemInnerSpacing = FloatVector2(4.0f, 4.0f);
        FloatVector2 m_cellPadding = FloatVector2(4.0f, 2.0f);
        FloatVector2 m_touchExtraPadding = FloatVector2Zero;
        float m_indentSpacing = 21.0f;
        float m_columnsMinSpacing = 6.0f;

        float m_scrollbarSize = 14.0f;
        float m_scrollbarRounding = 9.0f;
        float m_scrollbarPadding = 2.0f;
        float m_grabMinSize = 12.0f;
        float m_grabRounding = 0.0f;
        float m_logSliderDeadzone = 4.0f;

        float m_imageRounding = 0.0f;
        float m_imageBorderSize = 0.0f;

        float m_tabRounding = 5.0f;
        float m_tabBorderSize = 0.0f;
        float m_tabMinWidthBase = 1.0f;
        float m_tabMinWidthShrink = 80.0f;
        float m_tabCloseButtonMinWidthSelected = -1.0f;
        float m_tabCloseButtonMinWidthUnselected = 0.0f;
        float m_tabBarBorderSize = 1.0f;
        float m_tabBarOverlineSize = 1.0f;

        float m_tableAngledHeadersAngle = 0.610865238f;
        FloatVector2 m_tableAngledHeadersTextAlign = FloatVector2(0.5f, 0.0f);

        GuiTreeLines m_treeLines = GuiTreeLines::None;
        float m_treeLinesSize = 1.0f;
        float m_treeLinesRounding = 0.0f;

        float m_dragDropTargetRounding = 0.0f;
        float m_dragDropTargetBorderSize = 2.0f;
        float m_dragDropTargetPadding = 3.0f;

        float m_colorMarkerSize = 3.0f;
        GuiDirection m_colorButtonPosition = GuiDirection::Right;
        FloatVector2 m_buttonTextAlign = FloatVector2(0.5f, 0.5f);
        FloatVector2 m_selectableTextAlign = FloatVector2Zero;

        float m_separatorSize = 1.0f;
        float m_separatorTextBorderSize = 3.0f;
        FloatVector2 m_separatorTextAlign = FloatVector2(0.0f, 0.5f);
        FloatVector2 m_separatorTextPadding = FloatVector2(20.0f, 3.0f);

        FloatVector2 m_displayWindowPadding = FloatVector2(19.0f, 19.0f);
        FloatVector2 m_displaySafeAreaPadding = FloatVector2(3.0f, 3.0f);
        bool m_dockingNodeHasCloseButton = true;
        float m_dockingSeparatorSize = 2.0f;
        float m_mouseCursorScale = 1.0f;

        bool m_antiAliasedLines = true;
        bool m_antiAliasedLinesUseTexture = true;
        bool m_antiAliasedFill = true;
        float m_curveTessellationTolerance = 1.25f;
        float m_circleTessellationMaxError = 0.30f;

        NormalizedColorRGBA m_textColor = NormalizedColorRGBA(1.00f, 1.00f, 1.00f, 1.00f);
        NormalizedColorRGBA m_textDisabledColor = NormalizedColorRGBA(0.50f, 0.50f, 0.50f, 1.00f);
        NormalizedColorRGBA m_windowBackgroundColor = NormalizedColorRGBA(0.06f, 0.06f, 0.06f, 0.94f);
        NormalizedColorRGBA m_childBackgroundColor = NormalizedColorRGBA(0.00f, 0.00f, 0.00f, 0.00f);
        NormalizedColorRGBA m_popupBackgroundColor = NormalizedColorRGBA(0.08f, 0.08f, 0.08f, 0.94f);
        NormalizedColorRGBA m_borderColor = NormalizedColorRGBA(0.43f, 0.43f, 0.50f, 0.50f);
        NormalizedColorRGBA m_borderShadowColor = NormalizedColorRGBA(0.00f, 0.00f, 0.00f, 0.00f);

        NormalizedColorRGBA m_frameBackgroundColor = NormalizedColorRGBA(0.16f, 0.29f, 0.48f, 0.54f);
        NormalizedColorRGBA m_frameBackgroundHoveredColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 0.40f);
        NormalizedColorRGBA m_frameBackgroundActiveColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 0.67f);
        NormalizedColorRGBA m_titleBackgroundColor = NormalizedColorRGBA(0.04f, 0.04f, 0.04f, 1.00f);
        NormalizedColorRGBA m_titleBackgroundActiveColor = NormalizedColorRGBA(0.16f, 0.29f, 0.48f, 1.00f);
        NormalizedColorRGBA m_titleBackgroundCollapsedColor = NormalizedColorRGBA(0.00f, 0.00f, 0.00f, 0.51f);
        NormalizedColorRGBA m_menuBarBackgroundColor = NormalizedColorRGBA(0.14f, 0.14f, 0.14f, 1.00f);

        NormalizedColorRGBA m_scrollbarBackgroundColor = NormalizedColorRGBA(0.02f, 0.02f, 0.02f, 0.53f);
        NormalizedColorRGBA m_scrollbarGrabColor = NormalizedColorRGBA(0.31f, 0.31f, 0.31f, 1.00f);
        NormalizedColorRGBA m_scrollbarGrabHoveredColor = NormalizedColorRGBA(0.41f, 0.41f, 0.41f, 1.00f);
        NormalizedColorRGBA m_scrollbarGrabActiveColor = NormalizedColorRGBA(0.51f, 0.51f, 0.51f, 1.00f);

        NormalizedColorRGBA m_checkMarkColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 1.00f);
        NormalizedColorRGBA m_checkboxSelectedBackgroundColor = NormalizedColorRGBA(0.225f, 0.485f, 0.805f, 0.449f);
        NormalizedColorRGBA m_sliderGrabColor = NormalizedColorRGBA(0.24f, 0.52f, 0.88f, 1.00f);
        NormalizedColorRGBA m_sliderGrabActiveColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 1.00f);
        NormalizedColorRGBA m_buttonColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 0.40f);
        NormalizedColorRGBA m_buttonHoveredColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 1.00f);
        NormalizedColorRGBA m_buttonActiveColor = NormalizedColorRGBA(0.06f, 0.53f, 0.98f, 1.00f);
        NormalizedColorRGBA m_headerColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 0.31f);
        NormalizedColorRGBA m_headerHoveredColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 0.80f);
        NormalizedColorRGBA m_headerActiveColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 1.00f);

        NormalizedColorRGBA m_separatorColor = NormalizedColorRGBA(0.43f, 0.43f, 0.50f, 0.50f);
        NormalizedColorRGBA m_separatorHoveredColor = NormalizedColorRGBA(0.10f, 0.40f, 0.75f, 0.78f);
        NormalizedColorRGBA m_separatorActiveColor = NormalizedColorRGBA(0.10f, 0.40f, 0.75f, 1.00f);
        NormalizedColorRGBA m_resizeGripColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 0.20f);
        NormalizedColorRGBA m_resizeGripHoveredColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 0.67f);
        NormalizedColorRGBA m_resizeGripActiveColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 0.95f);
        NormalizedColorRGBA m_inputTextCursorColor = NormalizedColorRGBA(1.00f, 1.00f, 1.00f, 1.00f);

        NormalizedColorRGBA m_tabColor = NormalizedColorRGBA(0.18f, 0.35f, 0.58f, 0.862f);
        NormalizedColorRGBA m_tabHoveredColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 0.80f);
        NormalizedColorRGBA m_tabSelectedColor = NormalizedColorRGBA(0.20f, 0.41f, 0.68f, 1.00f);
        NormalizedColorRGBA m_tabSelectedOverlineColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 1.00f);
        NormalizedColorRGBA m_tabDimmedColor = NormalizedColorRGBA(0.068f, 0.102f, 0.148f, 0.9724f);
        NormalizedColorRGBA m_tabDimmedSelectedColor = NormalizedColorRGBA(0.136f, 0.262f, 0.424f, 1.00f);
        NormalizedColorRGBA m_tabDimmedSelectedOverlineColor = NormalizedColorRGBA(0.50f, 0.50f, 0.50f, 0.00f);
        NormalizedColorRGBA m_dockingPreviewColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 0.70f);
        NormalizedColorRGBA m_dockingEmptyBackgroundColor = NormalizedColorRGBA(0.20f, 0.20f, 0.20f, 1.00f);

        NormalizedColorRGBA m_plotLinesColor = NormalizedColorRGBA(0.61f, 0.61f, 0.61f, 1.00f);
        NormalizedColorRGBA m_plotLinesHoveredColor = NormalizedColorRGBA(1.00f, 0.43f, 0.35f, 1.00f);
        NormalizedColorRGBA m_plotHistogramColor = NormalizedColorRGBA(0.90f, 0.70f, 0.00f, 1.00f);
        NormalizedColorRGBA m_plotHistogramHoveredColor = NormalizedColorRGBA(1.00f, 0.60f, 0.00f, 1.00f);
        NormalizedColorRGBA m_tableHeaderBackgroundColor = NormalizedColorRGBA(0.19f, 0.19f, 0.20f, 1.00f);
        NormalizedColorRGBA m_tableBorderStrongColor = NormalizedColorRGBA(0.31f, 0.31f, 0.35f, 1.00f);
        NormalizedColorRGBA m_tableBorderLightColor = NormalizedColorRGBA(0.23f, 0.23f, 0.25f, 1.00f);
        NormalizedColorRGBA m_tableRowBackgroundColor = NormalizedColorRGBA(0.00f, 0.00f, 0.00f, 0.00f);
        NormalizedColorRGBA m_tableRowBackgroundAlternateColor = NormalizedColorRGBA(1.00f, 1.00f, 1.00f, 0.06f);

        NormalizedColorRGBA m_textLinkColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 1.00f);
        NormalizedColorRGBA m_textSelectedBackgroundColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 0.35f);
        NormalizedColorRGBA m_treeLinesColor = NormalizedColorRGBA(0.43f, 0.43f, 0.50f, 0.50f);
        NormalizedColorRGBA m_dragDropTargetColor = NormalizedColorRGBA(1.00f, 1.00f, 0.00f, 0.90f);
        NormalizedColorRGBA m_dragDropTargetBackgroundColor = NormalizedColorRGBA(0.00f, 0.00f, 0.00f, 0.00f);
        NormalizedColorRGBA m_unsavedMarkerColor = NormalizedColorRGBA(1.00f, 1.00f, 1.00f, 1.00f);
        NormalizedColorRGBA m_navCursorColor = NormalizedColorRGBA(0.26f, 0.59f, 0.98f, 1.00f);
        NormalizedColorRGBA m_navWindowingHighlightColor = NormalizedColorRGBA(1.00f, 1.00f, 1.00f, 0.70f);
        NormalizedColorRGBA m_navWindowingDimBackgroundColor = NormalizedColorRGBA(0.80f, 0.80f, 0.80f, 0.20f);
        NormalizedColorRGBA m_modalWindowDimBackgroundColor = NormalizedColorRGBA(0.80f, 0.80f, 0.80f, 0.35f);

        float m_hoverStationaryDelay = 0.15f;
        float m_hoverDelayShort = 0.15f;
        float m_hoverDelayNormal = 0.40f;
        GuiHoverFlags m_hoverFlagsForTooltipMouse = GuiHoverFlags::Stationary | GuiHoverFlags::DelayShort | GuiHoverFlags::AllowWhenDisabled;
        GuiHoverFlags m_hoverFlagsForTooltipNavigation = GuiHoverFlags::NoSharedDelay | GuiHoverFlags::DelayNormal | GuiHoverFlags::AllowWhenDisabled;
    };
} // namespace ego::gui
