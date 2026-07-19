#pragma once

#include "EgoCore/Math/Color.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Core/Geometry.h"
#include "EgoGui/Layout/Layout.h"

namespace ego::gui
{
    struct Theme;
    using ThemePointer = ego::SharedPointer<const Theme>;

    struct InteractionColors final
    {
        NormalizedColorRGBA m_normal = NormalizedColorTransparent;
        NormalizedColorRGBA m_hovered = NormalizedColorTransparent;
        NormalizedColorRGBA m_pressed = NormalizedColorTransparent;

        const NormalizedColorRGBA& resolve(bool _isHovered, bool _isPressed) const;
    };

    struct TypographyStyle final
    {
        NormalizedColorRGBA m_primary = NormalizedColorRGBA(NormalizedColorWhite);
        NormalizedColorRGBA m_muted = NormalizedColorRGBA(0.55f, 0.58f, 0.62f, 1.0f);
    };

    struct LayoutStyle final
    {
        Margin m_contentPadding = Margin(12.0f, 8.0f);
        float m_spacing = 8.0f;
    };

    struct ButtonStyle final
    {
        InteractionColors m_background = {
            NormalizedColorRGBA(0.16f, 0.18f, 0.20f, 1.0f),
            NormalizedColorRGBA(0.20f, 0.23f, 0.26f, 1.0f),
            NormalizedColorRGBA(0.10f, 0.12f, 0.14f, 1.0f)};
        Margin m_padding = Margin(10.0f, 6.0f);
    };

    struct FieldStyle final
    {
        NormalizedColorRGBA m_surface = NormalizedColorRGBA(0.04f, 0.05f, 0.06f, 1.0f);
        NormalizedColorRGBA m_borderNormal = NormalizedColorRGBA(0.12f, 0.13f, 0.15f, 1.0f);
        NormalizedColorRGBA m_borderHovered = NormalizedColorRGBA(0.20f, 0.23f, 0.26f, 1.0f);
        NormalizedColorRGBA m_borderFocused = NormalizedColorRGBA(0.20f, 0.55f, 0.95f, 1.0f);
        NormalizedColorRGBA m_selection = NormalizedColorRGBA(0.12f, 0.34f, 0.62f, 1.0f);
        Margin m_textPadding = Margin(6.0f, 5.0f);
        float m_height = 26.0f;
        float m_minimumWidth = 160.0f;
        float m_borderThickness = 1.0f;
        float m_caretThickness = 1.0f;
    };

    struct SelectionStyle final
    {
        NormalizedColorRGBA m_accent = NormalizedColorRGBA(0.20f, 0.55f, 0.95f, 1.0f);
        InteractionColors m_indicator = {
            NormalizedColorRGBA(0.12f, 0.13f, 0.15f, 1.0f),
            NormalizedColorRGBA(0.18f, 0.21f, 0.24f, 1.0f),
            NormalizedColorRGBA(0.08f, 0.10f, 0.12f, 1.0f)};
        float m_indicatorSize = 14.0f;
        float m_minimumHeight = 16.0f;
        float m_labelSpacing = 10.0f;
        float m_checkMarkInset = 3.0f;
        float m_radioInnerRadius = 4.0f;
        float m_radioMarkRadius = 3.0f;
        float m_optionSpacing = 4.0f;
        float m_groupTitleSpacing = 6.0f;
    };

    struct ScrollStyle final
    {
        NormalizedColorRGBA m_track = NormalizedColorRGBA(0.08f, 0.09f, 0.10f, 1.0f);
        InteractionColors m_thumb = {
            NormalizedColorRGBA(0.28f, 0.31f, 0.35f, 1.0f),
            NormalizedColorRGBA(0.38f, 0.42f, 0.48f, 1.0f),
            NormalizedColorRGBA(0.48f, 0.54f, 0.62f, 1.0f)};
        float m_thickness = 12.0f;
        float m_minimumThumbSize = 18.0f;
        float m_wheelStep = 40.0f;
    };

    struct WindowStyle final
    {
        NormalizedColorRGBA m_surface = NormalizedColorRGBA(0.075f, 0.080f, 0.090f, 1.0f);
        InteractionColors m_title = {
            NormalizedColorRGBA(0.13f, 0.14f, 0.16f, 1.0f),
            NormalizedColorRGBA(0.17f, 0.19f, 0.22f, 1.0f),
            NormalizedColorRGBA(0.20f, 0.23f, 0.27f, 1.0f)};
        InteractionColors m_resizeGrip = {
            NormalizedColorRGBA(0.32f, 0.35f, 0.40f, 1.0f),
            NormalizedColorRGBA(0.45f, 0.50f, 0.58f, 1.0f),
            NormalizedColorRGBA(0.58f, 0.65f, 0.76f, 1.0f)};
        NormalizedColorRGBA m_border = NormalizedColorRGBA(0.30f, 0.32f, 0.36f, 1.0f);
        float m_titleHeight = 28.0f;
        Margin m_titlePadding = Margin(8.0f, 6.0f, 8.0f, 0.0f);
        float m_resizeGripSize = 16.0f;
        float m_borderThickness = 1.0f;
        Size m_minimumSize = Size(120.0f, 80.0f);
    };

    struct Theme final
    {
        TypographyStyle m_typography;
        LayoutStyle m_layout;
        ButtonStyle m_button;
        FieldStyle m_field;
        SelectionStyle m_selection;
        ScrollStyle m_scroll;
        WindowStyle m_window;

        static const Theme& GetDefault();
    };
} // namespace ego::gui
