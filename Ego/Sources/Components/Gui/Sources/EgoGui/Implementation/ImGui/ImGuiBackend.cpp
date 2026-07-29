#include "ImGuiBackend.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGui/Implementation/ImGui/ImGuiContextScope.h"

#include <imgui.h>

namespace
{
    ImVec2 ToImGuiVector(const ego::FloatVector2& _vector)
    {
        return ImVec2(_vector.m_x, _vector.m_y);
    }

    ImVec4 ToImGuiColor(const ego::NormalizedColorRGBA& _color)
    {
        return ImVec4(_color.m_r, _color.m_g, _color.m_b, _color.m_a);
    }

    ImGuiDir ToImGuiDirection(ego::gui::GuiDirection _direction)
    {
        switch (_direction)
        {
        case ego::gui::GuiDirection::None:
            return ImGuiDir_None;
        case ego::gui::GuiDirection::Left:
            return ImGuiDir_Left;
        case ego::gui::GuiDirection::Right:
            return ImGuiDir_Right;
        case ego::gui::GuiDirection::Up:
            return ImGuiDir_Up;
        case ego::gui::GuiDirection::Down:
            return ImGuiDir_Down;
        }

        return ImGuiDir_None;
    }

    ImGuiTreeNodeFlags ToImGuiTreeLines(ego::gui::GuiTreeLines _treeLines)
    {
        switch (_treeLines)
        {
        case ego::gui::GuiTreeLines::None:
            return ImGuiTreeNodeFlags_DrawLinesNone;
        case ego::gui::GuiTreeLines::Full:
            return ImGuiTreeNodeFlags_DrawLinesFull;
        case ego::gui::GuiTreeLines::ToNodes:
            return ImGuiTreeNodeFlags_DrawLinesToNodes;
        }

        return ImGuiTreeNodeFlags_DrawLinesNone;
    }

    bool HasGuiHoverFlag(ego::gui::GuiHoverFlags _flags, ego::gui::GuiHoverFlags _flag)
    {
        return (static_cast<uint32_t>(_flags) & static_cast<uint32_t>(_flag)) != 0;
    }

    ImGuiHoveredFlags ToImGuiHoverFlags(ego::gui::GuiHoverFlags _flags)
    {
        ImGuiHoveredFlags result = ImGuiHoveredFlags_None;
        if (HasGuiHoverFlag(_flags, ego::gui::GuiHoverFlags::AllowWhenDisabled))
        {
            result |= ImGuiHoveredFlags_AllowWhenDisabled;
        }

        if (HasGuiHoverFlag(_flags, ego::gui::GuiHoverFlags::Stationary))
        {
            result |= ImGuiHoveredFlags_Stationary;
        }

        if (HasGuiHoverFlag(_flags, ego::gui::GuiHoverFlags::DelayShort))
        {
            result |= ImGuiHoveredFlags_DelayShort;
        }

        if (HasGuiHoverFlag(_flags, ego::gui::GuiHoverFlags::DelayNormal))
        {
            result |= ImGuiHoveredFlags_DelayNormal;
        }

        if (HasGuiHoverFlag(_flags, ego::gui::GuiHoverFlags::NoSharedDelay))
        {
            result |= ImGuiHoveredFlags_NoSharedDelay;
        }

        return result;
    }
} // namespace

ego::gui::ImGuiBackend::ImGuiBackend() = default;

ego::gui::ImGuiBackend::~ImGuiBackend()
{
    release();
}

bool ego::gui::ImGuiBackend::init(const ViewportProviderPointer& _viewportProvider, bool _enableMultiViewport)
{
    EGO_CHECK_RETURN_FALSE(!m_isInitialized && !m_context);
    EGO_CHECK_RETURN_FALSE(_viewportProvider);

    EGO_CHECK_INITIALIZATION(initializeContext(_enableMultiViewport));

    bool adaptersInitialized = false;
    {
        const ImGuiContextScope contextScope(*m_context);
        adaptersInitialized = m_rendererAdapter.init() && m_platformAdapter.init(_viewportProvider);
    }
    EGO_CHECK_INITIALIZATION(adaptersInitialized);

    m_isInitialized = true;

    return true;
}

bool ego::gui::ImGuiBackend::release()
{
    EGO_ASSERT(!m_isFrameActive);
    if (m_isFrameActive || !releaseContext())
    {
        return false;
    }

    resetState();

    return true;
}

bool ego::gui::ImGuiBackend::setFont(const FileContent& _content, float _size)
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized && m_context && !m_isFrameActive);
    EGO_CHECK_RETURN_FALSE(!_content.empty());
    EGO_CHECK_RETURN_FALSE(std::isfinite(_size) && _size > 0.0f);
    EGO_CHECK_RETURN_FALSE(_content.size() <= static_cast<size_t>(std::numeric_limits<int>::max()));

    void* fontData = ImGui::MemAlloc(_content.size());
    EGO_CHECK_RETURN_FALSE(fontData);
    std::memcpy(fontData, _content.data(), _content.size());

    const ImGuiContextScope contextScope(*m_context);

    ImGuiIO& io = ImGui::GetIO();
    ImFont* previousFont = io.FontDefault;
    ImFont* font = io.Fonts->AddFontFromMemoryTTF(fontData, static_cast<int>(_content.size()), _size);
    EGO_CHECK_RETURN_FALSE(font);

    io.FontDefault = font;
    if (previousFont)
    {
        io.Fonts->RemoveFont(previousFont);
    }

    return true;
}

bool ego::gui::ImGuiBackend::setStyle(const GuiStyle& _style)
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized && m_context && !m_isFrameActive);

    const ImGuiContextScope contextScope(*m_context);
    ::ImGuiStyle& style = ImGui::GetStyle();

    style.FontSizeBase = _style.m_fontSizeBase;
    style.FontScaleMain = _style.m_fontScaleMain;
    style.FontScaleDpi = _style.m_fontScaleDpi;

    style.Alpha = _style.m_alpha;
    style.DisabledAlpha = _style.m_disabledAlpha;
    style.WindowPadding = ToImGuiVector(_style.m_windowPadding);
    style.WindowRounding = _style.m_windowRounding;
    style.WindowBorderSize = _style.m_windowBorderSize;
    style.WindowBorderHoverPadding = _style.m_windowBorderHoverPadding;
    style.WindowMinSize = ToImGuiVector(_style.m_windowMinSize);
    style.WindowTitleAlign = ToImGuiVector(_style.m_windowTitleAlign);
    style.WindowMenuButtonPosition = ToImGuiDirection(_style.m_windowMenuButtonPosition);
    style.ChildRounding = _style.m_childRounding;
    style.ChildBorderSize = _style.m_childBorderSize;
    style.PopupRounding = _style.m_popupRounding;
    style.PopupBorderSize = _style.m_popupBorderSize;
    style.FramePadding = ToImGuiVector(_style.m_framePadding);
    style.FrameRounding = _style.m_frameRounding;
    style.FrameBorderSize = _style.m_frameBorderSize;
    style.ItemSpacing = ToImGuiVector(_style.m_itemSpacing);
    style.ItemInnerSpacing = ToImGuiVector(_style.m_itemInnerSpacing);
    style.CellPadding = ToImGuiVector(_style.m_cellPadding);
    style.TouchExtraPadding = ToImGuiVector(_style.m_touchExtraPadding);
    style.IndentSpacing = _style.m_indentSpacing;
    style.ColumnsMinSpacing = _style.m_columnsMinSpacing;
    style.ScrollbarSize = _style.m_scrollbarSize;
    style.ScrollbarRounding = _style.m_scrollbarRounding;
    style.ScrollbarPadding = _style.m_scrollbarPadding;
    style.GrabMinSize = _style.m_grabMinSize;
    style.GrabRounding = _style.m_grabRounding;
    style.LogSliderDeadzone = _style.m_logSliderDeadzone;
    style.ImageRounding = _style.m_imageRounding;
    style.ImageBorderSize = _style.m_imageBorderSize;
    style.TabRounding = _style.m_tabRounding;
    style.TabBorderSize = _style.m_tabBorderSize;
    style.TabMinWidthBase = _style.m_tabMinWidthBase;
    style.TabMinWidthShrink = _style.m_tabMinWidthShrink;
    style.TabCloseButtonMinWidthSelected = _style.m_tabCloseButtonMinWidthSelected;
    style.TabCloseButtonMinWidthUnselected = _style.m_tabCloseButtonMinWidthUnselected;
    style.TabBarBorderSize = _style.m_tabBarBorderSize;
    style.TabBarOverlineSize = _style.m_tabBarOverlineSize;
    style.TableAngledHeadersAngle = _style.m_tableAngledHeadersAngle;
    style.TableAngledHeadersTextAlign = ToImGuiVector(_style.m_tableAngledHeadersTextAlign);
    style.TreeLinesFlags = ToImGuiTreeLines(_style.m_treeLines);
    style.TreeLinesSize = _style.m_treeLinesSize;
    style.TreeLinesRounding = _style.m_treeLinesRounding;
    style.DragDropTargetRounding = _style.m_dragDropTargetRounding;
    style.DragDropTargetBorderSize = _style.m_dragDropTargetBorderSize;
    style.DragDropTargetPadding = _style.m_dragDropTargetPadding;
    style.ColorMarkerSize = _style.m_colorMarkerSize;
    style.ColorButtonPosition = ToImGuiDirection(_style.m_colorButtonPosition);
    style.ButtonTextAlign = ToImGuiVector(_style.m_buttonTextAlign);
    style.SelectableTextAlign = ToImGuiVector(_style.m_selectableTextAlign);
    style.SeparatorSize = _style.m_separatorSize;
    style.SeparatorTextBorderSize = _style.m_separatorTextBorderSize;
    style.SeparatorTextAlign = ToImGuiVector(_style.m_separatorTextAlign);
    style.SeparatorTextPadding = ToImGuiVector(_style.m_separatorTextPadding);
    style.DisplayWindowPadding = ToImGuiVector(_style.m_displayWindowPadding);
    style.DisplaySafeAreaPadding = ToImGuiVector(_style.m_displaySafeAreaPadding);
    style.DockingNodeHasCloseButton = _style.m_dockingNodeHasCloseButton;
    style.DockingSeparatorSize = _style.m_dockingSeparatorSize;
    style.MouseCursorScale = _style.m_mouseCursorScale;
    style.AntiAliasedLines = _style.m_antiAliasedLines;
    style.AntiAliasedLinesUseTex = _style.m_antiAliasedLinesUseTexture;
    style.AntiAliasedFill = _style.m_antiAliasedFill;
    style.CurveTessellationTol = _style.m_curveTessellationTolerance;
    style.CircleTessellationMaxError = _style.m_circleTessellationMaxError;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ToImGuiColor(_style.m_textColor);
    colors[ImGuiCol_TextDisabled] = ToImGuiColor(_style.m_textDisabledColor);
    colors[ImGuiCol_WindowBg] = ToImGuiColor(_style.m_windowBackgroundColor);
    colors[ImGuiCol_ChildBg] = ToImGuiColor(_style.m_childBackgroundColor);
    colors[ImGuiCol_PopupBg] = ToImGuiColor(_style.m_popupBackgroundColor);
    colors[ImGuiCol_Border] = ToImGuiColor(_style.m_borderColor);
    colors[ImGuiCol_BorderShadow] = ToImGuiColor(_style.m_borderShadowColor);

    colors[ImGuiCol_FrameBg] = ToImGuiColor(_style.m_frameBackgroundColor);
    colors[ImGuiCol_FrameBgHovered] = ToImGuiColor(_style.m_frameBackgroundHoveredColor);
    colors[ImGuiCol_FrameBgActive] = ToImGuiColor(_style.m_frameBackgroundActiveColor);
    colors[ImGuiCol_TitleBg] = ToImGuiColor(_style.m_titleBackgroundColor);
    colors[ImGuiCol_TitleBgActive] = ToImGuiColor(_style.m_titleBackgroundActiveColor);
    colors[ImGuiCol_TitleBgCollapsed] = ToImGuiColor(_style.m_titleBackgroundCollapsedColor);
    colors[ImGuiCol_MenuBarBg] = ToImGuiColor(_style.m_menuBarBackgroundColor);

    colors[ImGuiCol_ScrollbarBg] = ToImGuiColor(_style.m_scrollbarBackgroundColor);
    colors[ImGuiCol_ScrollbarGrab] = ToImGuiColor(_style.m_scrollbarGrabColor);
    colors[ImGuiCol_ScrollbarGrabHovered] = ToImGuiColor(_style.m_scrollbarGrabHoveredColor);
    colors[ImGuiCol_ScrollbarGrabActive] = ToImGuiColor(_style.m_scrollbarGrabActiveColor);

    colors[ImGuiCol_CheckMark] = ToImGuiColor(_style.m_checkMarkColor);
    colors[ImGuiCol_CheckboxSelectedBg] = ToImGuiColor(_style.m_checkboxSelectedBackgroundColor);
    colors[ImGuiCol_SliderGrab] = ToImGuiColor(_style.m_sliderGrabColor);
    colors[ImGuiCol_SliderGrabActive] = ToImGuiColor(_style.m_sliderGrabActiveColor);
    colors[ImGuiCol_Button] = ToImGuiColor(_style.m_buttonColor);
    colors[ImGuiCol_ButtonHovered] = ToImGuiColor(_style.m_buttonHoveredColor);
    colors[ImGuiCol_ButtonActive] = ToImGuiColor(_style.m_buttonActiveColor);
    colors[ImGuiCol_Header] = ToImGuiColor(_style.m_headerColor);
    colors[ImGuiCol_HeaderHovered] = ToImGuiColor(_style.m_headerHoveredColor);
    colors[ImGuiCol_HeaderActive] = ToImGuiColor(_style.m_headerActiveColor);

    colors[ImGuiCol_Separator] = ToImGuiColor(_style.m_separatorColor);
    colors[ImGuiCol_SeparatorHovered] = ToImGuiColor(_style.m_separatorHoveredColor);
    colors[ImGuiCol_SeparatorActive] = ToImGuiColor(_style.m_separatorActiveColor);
    colors[ImGuiCol_ResizeGrip] = ToImGuiColor(_style.m_resizeGripColor);
    colors[ImGuiCol_ResizeGripHovered] = ToImGuiColor(_style.m_resizeGripHoveredColor);
    colors[ImGuiCol_ResizeGripActive] = ToImGuiColor(_style.m_resizeGripActiveColor);
    colors[ImGuiCol_InputTextCursor] = ToImGuiColor(_style.m_inputTextCursorColor);

    colors[ImGuiCol_Tab] = ToImGuiColor(_style.m_tabColor);
    colors[ImGuiCol_TabHovered] = ToImGuiColor(_style.m_tabHoveredColor);
    colors[ImGuiCol_TabSelected] = ToImGuiColor(_style.m_tabSelectedColor);
    colors[ImGuiCol_TabSelectedOverline] = ToImGuiColor(_style.m_tabSelectedOverlineColor);
    colors[ImGuiCol_TabDimmed] = ToImGuiColor(_style.m_tabDimmedColor);
    colors[ImGuiCol_TabDimmedSelected] = ToImGuiColor(_style.m_tabDimmedSelectedColor);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ToImGuiColor(_style.m_tabDimmedSelectedOverlineColor);
    colors[ImGuiCol_DockingPreview] = ToImGuiColor(_style.m_dockingPreviewColor);
    colors[ImGuiCol_DockingEmptyBg] = ToImGuiColor(_style.m_dockingEmptyBackgroundColor);

    colors[ImGuiCol_PlotLines] = ToImGuiColor(_style.m_plotLinesColor);
    colors[ImGuiCol_PlotLinesHovered] = ToImGuiColor(_style.m_plotLinesHoveredColor);
    colors[ImGuiCol_PlotHistogram] = ToImGuiColor(_style.m_plotHistogramColor);
    colors[ImGuiCol_PlotHistogramHovered] = ToImGuiColor(_style.m_plotHistogramHoveredColor);
    colors[ImGuiCol_TableHeaderBg] = ToImGuiColor(_style.m_tableHeaderBackgroundColor);
    colors[ImGuiCol_TableBorderStrong] = ToImGuiColor(_style.m_tableBorderStrongColor);
    colors[ImGuiCol_TableBorderLight] = ToImGuiColor(_style.m_tableBorderLightColor);
    colors[ImGuiCol_TableRowBg] = ToImGuiColor(_style.m_tableRowBackgroundColor);
    colors[ImGuiCol_TableRowBgAlt] = ToImGuiColor(_style.m_tableRowBackgroundAlternateColor);

    colors[ImGuiCol_TextLink] = ToImGuiColor(_style.m_textLinkColor);
    colors[ImGuiCol_TextSelectedBg] = ToImGuiColor(_style.m_textSelectedBackgroundColor);
    colors[ImGuiCol_TreeLines] = ToImGuiColor(_style.m_treeLinesColor);
    colors[ImGuiCol_DragDropTarget] = ToImGuiColor(_style.m_dragDropTargetColor);
    colors[ImGuiCol_DragDropTargetBg] = ToImGuiColor(_style.m_dragDropTargetBackgroundColor);
    colors[ImGuiCol_UnsavedMarker] = ToImGuiColor(_style.m_unsavedMarkerColor);
    colors[ImGuiCol_NavCursor] = ToImGuiColor(_style.m_navCursorColor);
    colors[ImGuiCol_NavWindowingHighlight] = ToImGuiColor(_style.m_navWindowingHighlightColor);
    colors[ImGuiCol_NavWindowingDimBg] = ToImGuiColor(_style.m_navWindowingDimBackgroundColor);
    colors[ImGuiCol_ModalWindowDimBg] = ToImGuiColor(_style.m_modalWindowDimBackgroundColor);

    style.HoverStationaryDelay = _style.m_hoverStationaryDelay;
    style.HoverDelayShort = _style.m_hoverDelayShort;
    style.HoverDelayNormal = _style.m_hoverDelayNormal;
    style.HoverFlagsForTooltipMouse = ToImGuiHoverFlags(_style.m_hoverFlagsForTooltipMouse);
    style.HoverFlagsForTooltipNav = ToImGuiHoverFlags(_style.m_hoverFlagsForTooltipNavigation);

    return true;
}

bool ego::gui::ImGuiBackend::update(float _deltaTime, const DrawFunction& _drawFunction, GuiRenderData& _renderData)
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized && m_context && !m_isFrameActive && _drawFunction);

    const ImGuiContextScope contextScope(*m_context);

    if (!beginFrame(_deltaTime))
    {
        return false;
    }

    if (!_drawFunction())
    {
        cancelFrame();

        return false;
    }

    return endFrame(_renderData);
}

ego::gui::GuiFrameTextureID ego::gui::ImGuiBackend::bindTexture(const gpu::TextureViewPointer& _textureView, TextureSamplingMode _samplingMode)
{
    EGO_CHECK_RETURN_VALUE(m_isFrameActive, InvalidGuiFrameTextureID);

    return m_rendererAdapter.bindTexture(_textureView, _samplingMode);
}

bool ego::gui::ImGuiBackend::isInitialized() const
{
    return m_isInitialized;
}

bool ego::gui::ImGuiBackend::initializeContext(bool _enableMultiViewport)
{
    ImGuiContext* previousContext = ImGui::GetCurrentContext();
    m_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(previousContext);
    EGO_CHECK_RETURN_FALSE(m_context);

    const ImGuiContextScope contextScope(*m_context);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    if (_enableMultiViewport)
    {
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    }
    io.IniFilename = nullptr;

    return true;
}

bool ego::gui::ImGuiBackend::releaseContext()
{
    if (!m_context)
    {
        return true;
    }

    {
        const ImGuiContextScope contextScope(*m_context);
        m_platformAdapter.release();
        m_rendererAdapter.release();
    }

    ImGui::DestroyContext(m_context);
    m_context = nullptr;

    return true;
}

void ego::gui::ImGuiBackend::resetState()
{
    EGO_ASSERT(!m_context && !m_isFrameActive);

    m_isInitialized = false;
}

bool ego::gui::ImGuiBackend::beginFrame(float _deltaTime)
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized && !m_isFrameActive);

    EGO_ASSERT(ImGui::GetCurrentContext() == m_context);
    if (ImGui::GetCurrentContext() != m_context || !m_platformAdapter.beginFrame())
    {
        return false;
    }

    static constexpr float DefaultDeltaTime = 1.0f / 60.0f;

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = std::isfinite(_deltaTime) && _deltaTime > 0.0f ? _deltaTime : DefaultDeltaTime;

    m_rendererAdapter.beginFrame();
    ImGui::NewFrame();
    m_isFrameActive = true;

    return true;
}

bool ego::gui::ImGuiBackend::endFrame(GuiRenderData& _renderData)
{
    EGO_CHECK_RETURN_FALSE(m_isFrameActive);

    EGO_ASSERT(ImGui::GetCurrentContext() == m_context);
    if (ImGui::GetCurrentContext() != m_context)
    {
        m_isFrameActive = false;

        return false;
    }

    ImGui::Render();

    GuiRenderData renderData;
    const bool frameBuilt = m_platformAdapter.endFrame() && m_rendererAdapter.buildRenderData(m_platformAdapter, renderData);
    if (frameBuilt)
    {
        _renderData = std::move(renderData);
    }

    m_isFrameActive = false;

    return frameBuilt;
}

void ego::gui::ImGuiBackend::cancelFrame()
{
    EGO_CHECK_RETURN(m_isFrameActive);

    EGO_ASSERT(ImGui::GetCurrentContext() == m_context);
    if (ImGui::GetCurrentContext() == m_context)
    {
        ImGui::EndFrame();
    }

    m_isFrameActive = false;
}
